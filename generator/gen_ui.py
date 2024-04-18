#!/usr/bin/python3
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import Qt

import matplotlib.pyplot as plt
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
from PyQt5.QtWidgets import QSizePolicy
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree
import pyqtgraph.parametertree as ptree
import sys
import time
import os
import numpy as np
import argparse
import collections
import contextlib
import socket
import struct
from enum import Enum

try:
    import pyaudio  # sudo apt-get install python3-pyaudio
except ImportError:
    pyaudio = None

from params import params

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))

uiclass, baseclass = pg.Qt.loadUiType(__location__ + "/ui_gen.ui")

TreeStyle = """
            QTreeView {
                background-color: rgb(46, 52, 54);
                alternate-background-color: rgb(39, 44, 45);
                color: rgb(238, 238, 238);
            }
            QLabel {
                color: rgb(238, 238, 238);
            }
            QTreeView::item:has-children {
                background-color: '#212627';
                color: rgb(233, 185, 110);
            }
            QTreeView::item:selected {
                background-color: rgb(92, 53, 102);
            }
            """


@contextlib.contextmanager
def WavFileWriter(filename, sample_format, sample_rate_hz):
    stdout = filename == "-"
    with wave.open(sys.stdout.buffer if stdout else filename, "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(sample_format.bytes)
        f.setframerate(sample_rate_hz)
        try:
            yield f.writeframes
        finally:
            if stdout:
                return
            print("File [WAV]:", filename)
            print("Sample rate (Hz):", f.getframerate())
            print("Sample size (bytes):", f.getsampwidth())
            print("Sample count:", f.getnframes())
            print("Duration (ms):", 1000.0 * f.getnframes() / f.getframerate())


@contextlib.contextmanager
def RawFileWriter(filename, sample_format, sample_rate_hz):
    if filename == "-":
        yield sys.stdout.buffer.write
    else:
        with open(filename, "wb") as f:
            try:
                yield f.write
            finally:
                num_samples = f.tell() / sample_format.bytes
                print("File [RAW]:", filename)
                print("Sample rate (Hz):", sample_rate_hz)
                print("Sample size (bytes):", sample_format.bytes)
                print("Sample count:", num_samples)
                print("Duration (ms):", 1000.0 * num_samples / sample_rate_hz)


@contextlib.contextmanager
def Null(*args, **kwargs):
    yield lambda *largs: None


SampleFormat = collections.namedtuple("SampleFormat", ["name", "id", "bytes", "ffplay"])
SAMPLE_FORMATS = {
    f.name: f
    for f in [
        SampleFormat(name="S16_LE", id=0, bytes=2, ffplay="s16le"),
        SampleFormat(name="S32_LE", id=1, bytes=4, ffplay="s32le"),
    ]
}
PLAYERS = (
    {"blocking": BlockingMonoPlayer, "callback": CallbackMonoPlayer, "none": Null}
    if pyaudio
    else {"none": Null}
)
FORMATS = {"raw": RawFileWriter, "wav": WavFileWriter}


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ("yes", "true", "t", "y", "1"):
        return True
    elif v.lower() in ("no", "false", "f", "n", "0"):
        return False
    else:
        raise argparse.ArgumentTypeError("Boolean value expected.")


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



class MainWindow(uiclass, baseclass, object):
    def __init__(self,):
        self.app = QtWidgets.QApplication(sys.argv)
        super().__init__()
        self.setupUi(self)
        self.params = params

        # self.p = ptree.Parameter.create(name="Parameters", type="group")
        # self.t = ptree.ParameterTree(showHeader=False)
        # self.t.setParameters(self.p, showTop=False)        
        # self.p.child("args").setOpts(title="Genarator Options")

        self.p = Parameter.create(name="params", type="group", children=self.params)
        self.t = ParameterTree(showHeader=False)
        self.t.setParameters(self.p, showTop=False)
        
        self.p.sigTreeStateChanged.connect(self.change)
        self.t.setStyleSheet(TreeStyle)

        # pl = self.t.palette()
        # pl.setColor(self.t.backgroundRole(), Qt.red)
        # self.t.setPalette(pl)

        # self.t.addParameters(self.p)
        # self.t.setWindowTitle("Generator Parameter")

        # self.showMaximized()
        # self.showFullScreen()

        font = QtGui.QFont("Courier")
        font.setPixelSize(30)
        font.setBold(True)
        self.text = pg.TextItem()
        self.text.setFont(font)

        self.measurements = self.win.addPlot(title="Measurements", row=2, col=1)
        self.measurements.hideAxis("left")
        self.measurements.hideAxis("bottom")

        self.vb_w = self.measurements.getViewBox()
        self.t.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.vlay_tree.addWidget(self.t, stretch=7)
        # self.vlay_tree.addStretch()
        self.button = QtWidgets.QPushButton("Start")
        self.button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.button.clicked.connect(self.ButtonStart)
        self.vlay_tree.addWidget(self.button, stretch=1)
        # self.vlay_tree.addStretch()

    def change(self, param, changes):
        print("tree changes:")
        for param, change, data in changes:
            path = self.p.childPath(param)
            if path is not None:
                childName = ".".join(path)
            else:
                childName = param.name()
            print("  parameter: %s" % childName)
            print("  change:    %s" % change)
            print("  data:      %s" % str(data))
            print("  ----------")

    def valueChanging(self, param, value):
        print("Value changing (not finalized): %s %s" % (param, value))

    def save(self):
        global state
        state = self.p.saveState()

    def restore(self):
        global state
        add = self.p["Save/Restore functionality", "Restore State", "Add missing items"]
        rem = self.p[
            "Save/Restore functionality", "Restore State", "Remove extra items"
        ]
        self.p.restoreState(state, addChildren=add, removeChildren=rem)

    def ButtonStart(self):
        print("Start")
        print(self.p.param("Phase").value())
        # self.p.param('Phase').setValue(360.0)
        for k, v in self.p.names.items():
            print(k, self.p.param(k).value())

    def update(self):
        pass

    def start(self):
        self.app.exec_()

    def animation(self):
        timer = QtCore.QTimer()
        timer.setInterval(30)
        timer.timeout.connect(self.update)
        timer.start()  # 1
        self.start()


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description="Play or/and save audio from the Dev Board Micro mic",
    )
    parser.add_argument(
        "--host", type=str, default="10.10.10.1", help="host to connect"
    )
    parser.add_argument("--port", type=int, default=33000, help="port to connect")
    parser.add_argument(
        "--sample_rate_hz",
        "-sr",
        type=int,
        default=48000,
        choices=(16000, 48000),
        help="audio sample rate in Hz",
    )
    parser.add_argument(
        "--sample_format",
        "-sf",
        type=SAMPLE_FORMATS.get,
        default="S16_LE",
        choices=SAMPLE_FORMATS.values(),  # S32_LE
        metavar="{%s}" % ",".join(SAMPLE_FORMATS.keys()),
        help="audio sample format",
    )
    parser.add_argument(
        "--num_dma_buffers",
        "-n",
        type=int,
        default=2,
        metavar="N",
        help="number of DMA buffers",
    )
    parser.add_argument(
        "--dma_buffer_size_ms",
        "-b",
        type=int,
        default=50,
        metavar="MS",
        help="size of each DMA buffer in ms",
    )
    parser.add_argument(
        "--drop_first_samples_ms",
        "-d",
        type=int,
        default=0,
        metavar="MS",
        help="do not send first audio samples to avoid clicks",
    )
    parser.add_argument(
        "--player",
        "-p",
        type=str,
        default="callback" if pyaudio else "none",
        choices=PLAYERS.keys(),
        help="audio player type",
    )
    parser.add_argument(
        "--format",
        "-f",
        default="wav",
        choices=FORMATS.keys(),
        help="audio file format",
    )
    parser.add_argument(
        "--output",
        "-o",
        type=str,
        default="output.wav",
        metavar="FILENAME",
        help="record audio to file",
    )
    parser.add_argument(
        "--ffplay", action="store_true", help="only print ffplay command line"
    )
    parser.add_argument(
        "--dac_sample_rate_hz",
        "-dr",
        type=float,
        default=200000.0,
        choices=(48000.0, 200000.0),
        help="DAC sample rate in Hz",
    )
    parser.add_argument(
        "--duration",
        "-du",
        type=float,
        default=0.001,
        choices=(0.0005, 1.0),
        help="signal duration in S",
    )
    parser.add_argument(
        "--fstart",
        "-fs",
        type=float,
        default=7000.0,
        choices=(1.0, 80000.0),
        help="start frequency in Hz",
    )
    parser.add_argument(
        "--fstop",
        "-fe",
        type=float,
        default=17000.0,
        choices=(1.0, 80000.0),
        help="stop frequency in Hz",
    )
    parser.add_argument(
        "--amp",
        "-am",
        type=float,
        default=1.0,
        choices=(0.0, 1.0),
        help="amplitude 0.0 to 1.0",
    )
    parser.add_argument(
        "--phi",
        "-ph",
        type=float,
        default=0.0,
        choices=(0.0, 360.0),
        help="phase 0.0 to 360.0",
    )
    parser.add_argument(
        "--signal_type",
        "-st",
        type=FreqType.from_string,
        default=FreqType["quad"],
        choices=list(FreqType),
        help="signal type linear, quad, log, sin, triangle",
    )
    parser.add_argument(
        "--auto_restart",
        "-ar",
        type=str2bool,
        nargs="?",
        const=True,
        default=True,
        help="auto restart mode. 1 or 0 (default)",
    )
    parser.add_argument(
        "--run_back",
        "-rb",
        type=str2bool,
        nargs="?",
        const=True,
        default=True,
        help="run back mode. 1 or 0 (default)",
    )
    args = parser.parse_args()

    g_start = 1

    main = MainWindow()
    main.show()
    main.animation()
    print("Exiting...")
