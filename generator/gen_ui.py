#!/usr/bin/python3
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import Qt

import matplotlib.pyplot as plt
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree
import sys
import time
import os
import numpy as np

from params import params




uiclass, baseclass = pg.Qt.loadUiType("ui_gen.ui")


class MainWindow(uiclass, baseclass, object):
    def __init__(self):
        self.app = QtWidgets.QApplication(sys.argv)
        super().__init__()
        self.setupUi(self)
        self.params = params

        self.p = Parameter.create(name="params", type="group", children=self.params)

        self.p.sigTreeStateChanged.connect(self.change)

        self.t = ParameterTree(showHeader=False)
        self.t.setParameters(self.p, showTop=False)
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
        # self.t.setProperty("hide_action_toolbar", True)
        self.vlay_tree.addWidget(self.t)

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
    main = MainWindow()
    main.show()
    main.animation()
    print("Exiting...")
