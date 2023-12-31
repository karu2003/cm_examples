# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import collections
import contextlib
import socket
import struct
import sys
import threading
import wave

from enum import Enum

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

"""
Saves audio data fetched from the audio_server that's running
on the connected Dev Board Micro.

First load audio_server onto the Dev Board Micro:

    python3 scripts/flashtool.py -b build -e audio_streaming

Then start this client on your computer:

    python3 examples/audio_streaming/audio_client.py

It will start recording and continue until you quit this script, and
then it saves to output.wav by default.
"""

try:
    import pyaudio  # sudo apt-get install python3-pyaudio
except ImportError:
    pyaudio = None


class Overflow(Exception):
    """Exception raised when ring buffer does not have enough space to write."""
    pass


class Underflow(Exception):
    """Exception raised when ring buffer does not have enough data to read."""
    pass


class RingBuffer:
    """Simple ring buffer implementation.
    https://en.wikipedia.org/wiki/Circular_buffer
    """

    def __init__(self, buf):
        self._buf = buf
        self._r = 0
        self._size = 0

    def __len__(self):
        return len(self._buf)

    def __str__(self):
        return str(self._buf)

    @property
    def read_size(self):
        return self._size

    @property
    def write_size(self):
        return len(self) - self.read_size

    def read_only(self, buf):
        size = len(buf)
        if size == 0:
            return
        if size > self.read_size:
            raise Underflow
        f = self._r
        l = (f + size) % len(self)
        if f < l:
            buf[:] = self._buf[f:l]
        else:
            n = len(self) - f
            buf[:n] = self._buf[f:]
            buf[n:] = self._buf[:l]

    def remove_only(self, size):
        if size < 0:
            raise ValueError("'size' must be a non-negative number")
        if size > self.read_size:
            raise Underflow
        self._r = (self._r + size) % len(self)
        self._size -= size

    def read(self, buf):
        self.read_only(buf)
        self.remove_only(len(buf))

    def write(self, buf):
        size = len(buf)
        if size == 0:
            return
        if size > self.write_size:
            raise Overflow
        f = (self._r + self._size) % len(self)
        l = (f + size) % len(self)
        if f < l:
            self._buf[f:l] = buf
        else:
            n = len(self) - f
            self._buf[f:] = buf[:n]
            self._buf[:l] = buf[n:]
        self._size += size


class ConcurrentRingBuffer:
    """Blocking ring buffer for concurrent access from multiple threads."""

    def __init__(self, buf):
        self._rb = RingBuffer(buf)
        self._lock = threading.Lock()
        self._overflow = threading.Condition(self._lock)
        self._underflow = threading.Condition(self._lock)

    def __str__(self):
        return str(self._rb)

    def write(self, buf, block=True, timeout=None):
        if len(buf) > len(self._rb):
            raise ValueError("'buf' is too big")

        with self._lock:
            if block and not self._overflow.wait_for(
                    lambda: len(buf) <= self._rb.write_size, timeout):
                raise Overflow
            self._rb.write(buf)
            self._underflow.notify()

    def read(self, buf, remove_size=None, block=True, timeout=None):
        if len(buf) > len(self._rb):
            raise ValueError("'buf' is too big")

        if remove_size is not None:
            if remove_size < 0:
                raise ValueError("'remove_size' must be non-negative")
            if remove_size > len(buf):
                raise ValueError("'remove_size' must not exceed 'len(buf)'")

        with self._lock:
            if block and not self._underflow.wait_for(
                    lambda: len(buf) <= self._rb.read_size, timeout):
                raise Underflow
            self._rb.read_only(buf)
            self._rb.remove_only(
                len(buf) if remove_size is None else remove_size)
            self._overflow.notify()


@contextlib.contextmanager
def PyAudioStream(**kwargs):
    audio = pyaudio.PyAudio()
    try:
        stream = audio.open(**kwargs)
        if not kwargs.get('start', True):
            stream.start_stream()
        try:
            yield stream
        finally:
            stream.stop_stream()
            stream.close()
    finally:
        audio.terminate()


@contextlib.contextmanager
def CallbackMonoPlayer(sample_format, sample_rate_hz, frames_per_callback_ms):
    frames_per_buffer = (sample_rate_hz // 1000) * frames_per_callback_ms
    rb = ConcurrentRingBuffer(bytearray(10 * frames_per_buffer))
    buf = bytearray(frames_per_buffer * sample_format.bytes)

    def callback(in_data, frame_count, time_info, status):
        assert (frame_count == frames_per_buffer)
        rb.read(buf)
        return bytes(buf), pyaudio.paContinue

    with PyAudioStream(format=pyaudio_format(sample_format),
                       frames_per_buffer=frames_per_buffer,
                       channels=1,
                       rate=sample_rate_hz,
                       stream_callback=callback,
                       start=False,
                       output=True) as stream:
        try:
            yield rb.write
        finally:
            rb.write(bytes(len(buf)))  # push zeros to unblock the callback


@contextlib.contextmanager
def BlockingMonoPlayer(sample_format, sample_rate_hz, frames_per_callback_ms):
    with PyAudioStream(format=pyaudio_format(sample_format),
                       channels=1,
                       rate=sample_rate_hz,
                       output=True) as stream:
        yield stream.write


@contextlib.contextmanager
def WavFileWriter(filename, sample_format, sample_rate_hz):
    stdout = filename == '-'
    with wave.open(sys.stdout.buffer if stdout else filename, 'wb') as f:
        f.setnchannels(1)
        f.setsampwidth(sample_format.bytes)
        f.setframerate(sample_rate_hz)
        try:
            yield f.writeframes
        finally:
            if stdout:
                return
            print('File [WAV]:', filename)
            print('Sample rate (Hz):', f.getframerate())
            print('Sample size (bytes):', f.getsampwidth())
            print('Sample count:', f.getnframes())
            print('Duration (ms):', 1000.0 * f.getnframes() / f.getframerate())


@contextlib.contextmanager
def RawFileWriter(filename, sample_format, sample_rate_hz):
    if filename == '-':
        yield sys.stdout.buffer.write
    else:
        with open(filename, 'wb') as f:
            try:
                yield f.write
            finally:
                num_samples = f.tell() / sample_format.bytes
                print('File [RAW]:', filename)
                print('Sample rate (Hz):', sample_rate_hz)
                print('Sample size (bytes):', sample_format.bytes)
                print('Sample count:', num_samples)
                print('Duration (ms):', 1000.0 * num_samples / sample_rate_hz)


@contextlib.contextmanager
def Null(*args, **kwargs):
    yield lambda *largs: None


SampleFormat = collections.namedtuple(
    'SampleFormat', ['name', 'id', 'bytes', 'ffplay'])
SAMPLE_FORMATS = {f.name: f for f in
                  [SampleFormat(name='S16_LE', id=0, bytes=2, ffplay='s16le'),
                   SampleFormat(name='S32_LE', id=1, bytes=4, ffplay='s32le')]
                  }
PLAYERS = {'blocking': BlockingMonoPlayer,
           'callback': CallbackMonoPlayer,
           'none': Null} if pyaudio else {'none': Null}
FORMATS = {'raw': RawFileWriter,
           'wav': WavFileWriter}


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


class FreqType(Enum):
    linear = 0
    quad = 1
    log = 2
    sin = 3
    triangle = 4

    def __str__(self):
        return self.name

    @staticmethod
    def from_string(s):
        try:
            return FreqType[s]
        except KeyError:
            raise ValueError()


def pyaudio_format(fmt):
    return {2: pyaudio.paInt16, 4: pyaudio.paInt32}[fmt.bytes]


def main():
    f, ax = plt.subplots(2)
    x = np.arange(10000)
    y = np.random.randn(10000)
    # Plot 0 is for raw audio data
    li, = ax[0].plot(x, y)
    ax[0].set_xlim(0, 1024)
    ax[0].set_ylim(-2**14, 2**14)
    ax[0].set_title("Raw Audio Signal")
    # Plot 1 is for the FFT of the audio
    li2, = ax[1].plot(x, y)
    ax[1].set_xlim(0, 5000)
    ax[1].set_ylim(-200, 200)
    ax[1].set_title("Fast Fourier Transform")
    # Show the plot, but without blocking updates
    plt.pause(0.01)
    plt.tight_layout()

    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Play or/and save audio from the Dev Board Micro mic')
    parser.add_argument('--host', type=str, default='10.10.10.1',
                        help='host to connect')
    parser.add_argument('--port', type=int, default=33000,
                        help='port to connect')
    parser.add_argument('--sample_rate_hz', '-sr', type=int,
                        default=48000, choices=(16000, 48000),
                        help='audio sample rate in Hz')
    parser.add_argument('--sample_format', '-sf', type=SAMPLE_FORMATS.get,
                        default='S16_LE', choices=SAMPLE_FORMATS.values(),  # S32_LE
                        metavar='{%s}' % ','.join(SAMPLE_FORMATS.keys()),
                        help='audio sample format')
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=2,
                        metavar='N', help='number of DMA buffers')
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=50,
                        metavar='MS',
                        help='size of each DMA buffer in ms')
    parser.add_argument('--drop_first_samples_ms', '-d', type=int, default=0,
                        metavar='MS',
                        help='do not send first audio samples to avoid clicks')
    parser.add_argument('--player', '-p', type=str,
                        default='callback' if pyaudio else 'none',
                        choices=PLAYERS.keys(),
                        help='audio player type')
    parser.add_argument('--format', '-f', default='wav', choices=FORMATS.keys(),
                        help='audio file format')
    parser.add_argument('--output', '-o', type=str, default='output.wav',
                        metavar='FILENAME',
                        help='record audio to file')
    parser.add_argument('--ffplay', action='store_true',
                        help='only print ffplay command line')
    parser.add_argument('--dac_sample_rate_hz', '-dr', type=float,
                        default=200000., choices=(48000., 200000.),
                        help='DAC sample rate in Hz')
    parser.add_argument('--duration', '-du', type=float,
                        default=0.001, choices=(0.0005, 1.),
                        help='signal duration in S')
    parser.add_argument('--fstart', '-fs', type=float,
                        default=7000.0, choices=(1., 80000.0),
                        help='start frequency in Hz')
    parser.add_argument('--fstop', '-fe', type=float,
                        default=17000.0, choices=(1., 80000.0),
                        help='stop frequency in Hz')
    parser.add_argument('--amp', '-am', type=float,
                        default=1.0, choices=(0., 1.0),
                        help='amplitude 0.0 to 1.0')
    parser.add_argument('--phi', '-ph', type=float,
                        default=0.0, choices=(0., 360.0),
                        help='phase 0.0 to 360.0')
    parser.add_argument('--signal_type', '-st', type=FreqType.from_string,
                        default=FreqType["quad"], choices=list(FreqType),
                        help='signal type linear, quad, log, sin, triangle')
    parser.add_argument('--auto_restart', '-ar', type=str2bool, nargs='?',
                        const=True, default=True,
                        help="auto restart mode. 1 or 0 (default)")
    parser.add_argument('--run_back', '-rb', type=str2bool, nargs='?',
                        const=True, default=True,
                        help="run back mode. 1 or 0 (default)")
    args = parser.parse_args()

    if args.ffplay:
        filename = args.output if args.output else '<filename>'
        if args.format == 'wav':
            print('ffplay %s' % filename)
        else:
            print('ffplay -f %s -ar %d -ac 1 %s' %
                  (args.sample_format.ffplay, args.sample_rate_hz, filename))
        return

    sock = socket.socket()
    sock.settimeout(10)
    try:
        sock.connect((args.host, args.port))
    except (TimeoutError, ConnectionError) as e:
        msg = 'Cannot connect to Coral Dev Board Micro, make sure you specify' \
              ' the correct IP address with --host.'
        if sys.platform == 'darwin':
            msg += ' Network over USB is not supported on macOS.'
        raise RuntimeError(msg) from e
    sock.settimeout(None)
    g_start = 1
    send_data = struct.pack('<iiiiiffffffi??i',
                          args.sample_rate_hz,
                          args.sample_format.id,
                          args.dma_buffer_size_ms,
                          args.num_dma_buffers,
                          args.drop_first_samples_ms,
                          args.dac_sample_rate_hz,
                          args.duration,
                          args.fstart,
                          args.fstop,
                          args.amp,
                          args.phi,
                          args.signal_type.value,
                          args.auto_restart,
                          args.run_back,
                          g_start
                          )
    send = sock.send(send_data)

    while True:
        samples = sock.recv(4096)
        if not samples:
            break
        audio_data = np.frombuffer(samples, np.int16)
        dfft = 10.*np.log10(abs(np.fft.rfft(audio_data)))
        li.set_xdata(np.arange(len(audio_data)))
        li.set_ydata(audio_data)
        li2.set_xdata(np.arange(len(dfft))*10.)
        li2.set_ydata(dfft)
        plt.pause(0.01)


if __name__ == '__main__':
    main()
