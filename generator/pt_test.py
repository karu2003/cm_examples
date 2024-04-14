import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets

app = pg.mkQApp("Parameter Tree Example")
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree

uiclass, baseclass = pg.Qt.loadUiType("ui_gen.ui")


## test subclassing parameters
## This parameter automatically generates two child parameters which are always reciprocals of each other
class ComplexParameter(pTypes.GroupParameter):
    def __init__(self, **opts):
        opts["type"] = "bool"
        opts["value"] = True
        pTypes.GroupParameter.__init__(self, **opts)

        self.addChild(
            {
                "name": "Sample Rate",
                "type": "float",
                "value": 200000.0,
                "suffix": "Hz",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Sample format",
                "type": "float",
                "value": 1,
                "suffix": "",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "DMA buffer size",
                "type": "float",
                "value": 50,
                "suffix": "ms",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Num DMA buffers",
                "type": "int",
                "value": 2,
                "suffix": "",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Drop first samples",
                "type": "int",
                "value": 2,
                "suffix": "mc",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "DAC sample rate",
                "type": "float",
                "value": 200000.0,
                "suffix": "Hz",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Duration",
                "type": "float",
                "value": 0.001,
                "suffix": "S",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Chirp start frequency",
                "type": "float",
                "value": 7000.0,
                "suffix": "Hz",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Chirp stop frequency",
                "type": "float",
                "value": 17000.0,
                "suffix": "Hz",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Amplitude",
                "type": "float",
                "value": 1.0,
                "suffix": "V",
                "siPrefix": True,
            }
        )
        self.addChild(
            {
                "name": "Phase",
                "type": "float",
                "value": 0.0,
                "suffix": "degrees",
                "siPrefix": True,
            }
        )

        # self.a = self.param("A = 1/B")
        # self.b = self.param("B = 1/A")
        # self.a.sigValueChanged.connect(self.aChanged)
        # self.b.sigValueChanged.connect(self.bChanged)

    def aChanged(self):
        self.b.setValue(1.0 / self.a.value(), blockSignal=self.bChanged)

    def bChanged(self):
        self.a.setValue(1.0 / self.b.value(), blockSignal=self.aChanged)

params = [
    # ComplexParameter(name="Generator parameter"),
    {
        "name": "Sample Rate",
        "type": "float",
        "value": 200000.0,
        "suffix": "Hz",
        "siPrefix": True,
    },
    {
        "name": "Sample format",
        "type": "float",
        "value": 1,
        "suffix": "",
        "siPrefix": True,
    },
    {
        "name": "DMA buffer size",
        "type": "float",
        "value": 50,
        "suffix": "ms",
        "siPrefix": True,
    },
    {
        "name": "Num DMA buffers",
        "type": "int",
        "value": 2,
        "suffix": "",
        "siPrefix": True,
    },
    {
        "name": "Drop first samples",
        "type": "int",
        "value": 2,
        "suffix": "mc",
        "siPrefix": True,
    },
    {
        "name": "DAC sample rate",
        "type": "float",
        "value": 200000.0,
        "suffix": "Hz",
        "siPrefix": True,
    },
    {
        "name": "Duration",
        "type": "float",
        "value": 0.001,
        "suffix": "S",
        "siPrefix": True,
    },
    {
        "name": "Chirp start frequency",
        "type": "float",
        "value": 7000.0,
        "suffix": "Hz",
        "siPrefix": True,
    },
    {
        "name": "Chirp stop frequency",
        "type": "float",
        "value": 17000.0,
        "suffix": "Hz",
        "siPrefix": True,
    },
    {
        "name": "Amplitude",
        "type": "float",
        "value": 1.0,
        "suffix": "V",
        "siPrefix": True,
    },
    {
        "name": "Phase",
        "type": "float",
        "limits": [0, 360],
        "value": 0.0,
        "suffix": "degrees",
        "siPrefix": True,
    },
    {
        "name": "Signal type",
        "type": "list",
        "type":"list",
        "limits": ["linear", "quad", "log", "sin", "triangle"],
        "value" : "linear",
    },
    {"name": "Auto restart", "type": "bool", "value": True},
    {"name": "Run back", "type": "bool", "value": False},
    {"name": "Start", "type": "bool", "value": False},
]

## Create tree of Parameter objects
p = Parameter.create(name="params", type="group", children=params)


## If anything changes in the tree, print a message
def change(param, changes):
    print("tree changes:")
    for param, change, data in changes:
        path = p.childPath(param)
        if path is not None:
            childName = ".".join(path)
        else:
            childName = param.name()
        print("  parameter: %s" % childName)
        print("  change:    %s" % change)
        print("  data:      %s" % str(data))
        print("  ----------")


p.sigTreeStateChanged.connect(change)


def valueChanging(param, value):
    print("Value changing (not finalized): %s %s" % (param, value))


def save():
    global state
    state = p.saveState()


def restore():
    global state
    add = p["Save/Restore functionality", "Restore State", "Add missing items"]
    rem = p["Save/Restore functionality", "Restore State", "Remove extra items"]
    p.restoreState(state, addChildren=add, removeChildren=rem)


## Create two ParameterTree widgets, both accessing the same data
t = ParameterTree()
t.setParameters(p, showTop=False)
t.setWindowTitle("pyqtgraph example: Parameter Tree")

win = QtWidgets.QWidget()
layout = QtWidgets.QGridLayout()
win.setLayout(layout)
layout.addWidget(
    QtWidgets.QLabel(
        "These are two views of the same data. They should always display the same values."
    ),
    0,
    0,
    1,
    2,
)
layout.addWidget(t, 1, 0, 1, 1)
win.show()

## test save/restore
state = p.saveState()
p.restoreState(state)
compareState = p.saveState()
assert pg.eq(compareState, state)

if __name__ == "__main__":
    pg.exec()
