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

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np


def main():
    f, ax = plt.subplots(2)
    x = np.arange(10000)
    y = np.random.randn(10000)
    # Plot 0 is for raw audio data
    (li,) = ax[0].plot(x, y)
    ax[0].set_xlim(0, 1024)
    ax[0].set_ylim(-(2**14), 2**14)
    ax[0].set_title("Raw Audio Signal")
    # Plot 1 is for the FFT of the audio
    (li2,) = ax[1].plot(x, y)
    ax[1].set_xlim(0, 5000)
    ax[1].set_ylim(-200, 200)
    ax[1].set_title("Fast Fourier Transform")
    # Show the plot, but without blocking updates
    plt.pause(0.01)
    plt.tight_layout()

    from argparse_gen import ArgumentParserClass

    arg_parser = ArgumentParserClass()
    args = arg_parser.parse_args()

    if args.ffplay:
        filename = args.output if args.output else "<filename>"
        if args.format == "wav":
            print("ffplay %s" % filename)
        else:
            print(
                "ffplay -f %s -ar %d -ac 1 %s"
                % (args.sample_format.ffplay, args.sample_rate_hz, filename)
            )
        return

    sock = socket.socket()
    sock.settimeout(10)
    try:
        sock.connect((args.host, args.port))
    except (TimeoutError, ConnectionError) as e:
        msg = (
            "Cannot connect to Coral Dev Board Micro, make sure you specify"
            " the correct IP address with --host."
        )
        if sys.platform == "darwin":
            msg += " Network over USB is not supported on macOS."
        raise RuntimeError(msg) from e
    sock.settimeout(None)
    g_start = 1
    send_data = struct.pack(
        "<iiiiiffffffi??i",
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
        g_start,
    )
    send = sock.send(send_data)

    while True:
        samples = sock.recv(4096)
        if not samples:
            break
        audio_data = np.frombuffer(samples, np.int16)
        dfft = 10.0 * np.log10(abs(np.fft.rfft(audio_data)))
        li.set_xdata(np.arange(len(audio_data)))
        li.set_ydata(audio_data)
        li2.set_xdata(np.arange(len(dfft)) * 10.0)
        li2.set_ydata(dfft)
        plt.pause(0.01)


if __name__ == "__main__":
    main()
