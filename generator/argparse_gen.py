import argparse

from params import FreqType, SAMPLE_FORMATS, PLAYERS, FORMATS, pyaudio


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ("yes", "true", "t", "y", "1"):
        return True
    elif v.lower() in ("no", "false", "f", "n", "0"):
        return False
    else:
        raise argparse.ArgumentTypeError("Boolean value expected.")


class ArgumentParserClass:
    def __init__(self):
        self.parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description="Play or/and save audio from the Dev Board Micro mic",
        )
        self.parser.add_argument(
            "--host", type=str, default="10.10.10.1", help="host to connect"
        )
        self.parser.add_argument(
            "--port", type=int, default=33000, help="port to connect"
        )
        self.parser.add_argument(
            "--sample_rate_hz",
            "-sr",
            type=int,
            default=48000,
            choices=(16000, 48000),
            help="audio sample rate in Hz",
        )
        self.parser.add_argument(
            "--sample_format",
            "-sf",
            type=SAMPLE_FORMATS.get,
            default="S16_LE",
            choices=SAMPLE_FORMATS.values(),  # S32_LE
            metavar="{%s}" % ",".join(SAMPLE_FORMATS.keys()),
            help="audio sample format",
        )
        self.parser.add_argument(
            "--num_dma_buffers",
            "-n",
            type=int,
            default=2,
            metavar="N",
            help="number of DMA buffers",
        )
        self.parser.add_argument(
            "--dma_buffer_size_ms",
            "-b",
            type=int,
            default=50,
            metavar="MS",
            help="size of each DMA buffer in ms",
        )
        self.parser.add_argument(
            "--drop_first_samples_ms",
            "-d",
            type=int,
            default=0,
            metavar="MS",
            help="do not send first audio samples to avoid clicks",
        )
        self.parser.add_argument(
            "--player",
            "-p",
            type=str,
            default="callback" if pyaudio else "none",
            choices=PLAYERS.keys(),
            help="audio player type",
        )
        self.parser.add_argument(
            "--format",
            "-f",
            default="wav",
            choices=FORMATS.keys(),
            help="audio file format",
        )
        self.parser.add_argument(
            "--output",
            "-o",
            type=str,
            default="output.wav",
            metavar="FILENAME",
            help="record audio to file",
        )
        self.parser.add_argument(
            "--ffplay", action="store_true", help="only print ffplay command line"
        )
        self.parser.add_argument(
            "--dac_sample_rate_hz",
            "-dr",
            type=float,
            default=200000.0,
            choices=(48000.0, 200000.0),
            help="DAC sample rate in Hz",
        )
        self.parser.add_argument(
            "--duration",
            "-du",
            type=float,
            default=0.001,
            choices=(0.0005, 1.0),
            help="signal duration in S",
        )
        self.parser.add_argument(
            "--fstart",
            "-fs",
            type=float,
            default=7000.0,
            choices=(1.0, 80000.0),
            help="start frequency in Hz",
        )
        self.parser.add_argument(
            "--fstop",
            "-fe",
            type=float,
            default=17000.0,
            choices=(1.0, 80000.0),
            help="stop frequency in Hz",
        )
        self.parser.add_argument(
            "--amp",
            "-am",
            type=float,
            default=1.0,
            choices=(0.0, 1.0),
            help="amplitude 0.0 to 1.0",
        )
        self.parser.add_argument(
            "--phi",
            "-ph",
            type=float,
            default=0.0,
            choices=(0.0, 360.0),
            help="phase 0.0 to 360.0",
        )
        self.parser.add_argument(
            "--signal_type",
            "-st",
            type=FreqType.from_string,
            default=FreqType["quad"],
            choices=list(FreqType),
            help="signal type linear, quad, log, sin, triangle",
        )
        self.parser.add_argument(
            "--auto_restart",
            "-ar",
            type=str2bool,
            nargs="?",
            const=True,
            default=True,
            help="auto restart mode. 1 or 0 (default)",
        )
        self.parser.add_argument(
            "--run_back",
            "-rb",
            type=str2bool,
            nargs="?",
            const=True,
            default=True,
            help="run back mode. 1 or 0 (default)",
        )

    def parse_args(self):
        return self.parser.parse_args()
