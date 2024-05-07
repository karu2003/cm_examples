#!/usr/bin/python3
from PyQt5 import QtWidgets, QtCore

import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
from PyQt5.QtWidgets import QSizePolicy

import sys
import time
import os
import numpy as np
import collections
import contextlib
import socket
import struct

from params import GenParameterTree
from argparse_gen import ArgumentParserClass

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))

uiclass, baseclass = pg.Qt.loadUiType(__location__ + "/ui_gen.ui")

filename = __location__ + "/args.yaml"


class MainWindow(uiclass, baseclass, object):
    def __init__(self, args):
        self.app = QtWidgets.QApplication(sys.argv)
        super().__init__()
        self.setupUi(self)
        self.tree = GenParameterTree(args=args, filename=filename)

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
        self.tree.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.vlay_tree.addWidget(self.tree, stretch=7)
        # self.vlay_tree.addStretch()
        self.button = QtWidgets.QPushButton("Start")
        self.button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.button.clicked.connect(self.ButtonStart)
        self.vlay_tree.addWidget(self.button, stretch=1)
        # self.vlay_tree.addStretch()

    def ButtonStart(self):
        key = []
        print("Start")
        print(self.tree.p.param("phi").value())
        self.tree.p.param("phi").setValue(360.0)

        self.tree.save_to_yaml(filename)

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

    arg_parser = ArgumentParserClass()
    args = arg_parser.parse_args()

    g_start = 1

    main = MainWindow(args)
    main.show()
    main.animation()
    print("Exiting...")
