import collections
import contextlib

try:
    import pyaudio  # sudo apt-get install python3-pyaudio
except ImportError:
    pyaudio = None

params = [
    {
        "name": "Sample Rate",
        "type": "float",
        "value": 200000.0,
        "suffix": "Hz",
        "siPrefix": True,
        # "default": False,
        # "expanded": False,
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
        "type": "list",
        "limits": ["linear", "quad", "log", "sin", "triangle"],
        "value": "linear",
    },
    {"name": "Auto restart", "type": "bool", "value": True},
    {"name": "Run back", "type": "bool", "value": False},
    {"name": "Start", "type": "bool", "value": False},  # , "visible":False},
]


HIDE = ["host", "port", "output", "ffplay", "player", "format"]
HZ_SUFFIX = ["sample_rate_hz", "dac_sample_rate_hz", "fstart", "fstop"]

from enum import Enum


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


@contextlib.contextmanager
def Null(*args, **kwargs):
    yield lambda *largs: None


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


from pyqtgraph.parametertree import Parameter, ParameterTree
from PyQt5.QtCore import Qt


class GenParameterTree(ParameterTree):
    def __init__(self, args):
        super().__init__()
        # self.params = params
        self.p = self.init_params_from_args(args, params)
        self.p = self.create_params_from_args(args)

        self.hide_params(self.p)
        self.add_suxffix(self.p, suffix_list=HZ_SUFFIX, suffix="Hz")
        self.add_sisuffix(self.p, si_list=HZ_SUFFIX)

        self.p.sigTreeStateChanged.connect(self.change)
        self.setParameters(self.p, showTop=False)
        # self.setStyleSheet(TreeStyle)

        pl = self.palette()
        pl.setColor(self.backgroundRole(), Qt.red)
        self.setPalette(pl)

    def init_params_from_args(self, args, params):
        """Initialize parameters from command line arguments."""
        p = Parameter.create(name="params", type="group", children=params)
        args = vars(args)
        for k, v in args.items():
            if k in p.names:
                p[k] = v
        return p

    def create_params_from_args(self, args):
        """Create parameters from command line arguments use .setOpts."""
        p = Parameter.create(name="params", type="group")
        args = vars(args)
        for k, v in args.items():
            if type(v) == bool:
                p.addChild({"name": k, "type": "bool", "value": v})
            elif type(v) == int:
                p.addChild({"name": k, "type": "int", "value": v})
            elif type(v) == float:
                p.addChild({"name": k, "type": "float", "value": v})
            elif type(v) == str:
                if k == "format":
                    p.addChild(
                        {
                            "name": k,
                            "type": "list",
                            "limits": list(FORMATS.keys()),
                            "value": v,
                        }
                    )
                else:
                    p.addChild({"name": k, "type": "str", "value": v})
                # p.addChild({"name": k, "type": "str", "value": v})
            elif type(v) == FreqType:
                p.addChild(
                    {
                        "name": k,
                        "type": "list",
                        "limits": ["linear", "quad", "log", "sin", "triangle"],
                        "value": str(v),
                    }
                )
            elif type(v) == SampleFormat:
                p.addChild(
                    {
                        "name": k,
                        "type": "list",
                        "limits": list(SAMPLE_FORMATS.keys()),
                        "value": v.name,
                    }
                )
        return p

    def hide_params(self, p, hide=HIDE):
        """Hide some parameters."""
        for k in p.names:
            if k in hide:
                p.param(k).setOpts(visible=False)

    def add_suxffix(self, p, suffix_list=HZ_SUFFIX, suffix="Hz"):
        """Add suffix to list parameters."""
        for k in p.names:
            if k in suffix_list:
                k = p.param(k).setOpts(suffix=suffix)

    def add_sisuffix(self, p, si_list=HZ_SUFFIX):
        """Add SI suffix to list parameters."""
        for k in p.names:
            if k in si_list:
                k = p.param(k).setOpts(siPrefix=True)

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

    def print_all_set(self, p):
        """print all settings of paramert"""
        state = p.saveState()
        for param in p:
            # print(param.opts)
            for i in param.opts:
                print(i)
        # print(state)
        # for i in p.names:
        #     print(type(p.param(i)).opts)
