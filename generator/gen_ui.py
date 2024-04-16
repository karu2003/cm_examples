#!/usr/bin/python3
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import Qt

import matplotlib.pyplot as plt
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
from PyQt5.QtWidgets import QSizePolicy
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree
import sys
import time
import os
import numpy as np

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
        self.vlay_tree.addWidget(self.t, stretch=5)
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
        print(self.p.param('Phase').value())
        # self.p.param('Phase').setValue(360.0)
        for k , v in self.p.names.items():
            print(k ,self.p.param(k).value() )

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
