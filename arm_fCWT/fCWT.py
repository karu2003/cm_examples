import numpy as np
import math

PI = 3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798
sqrt2PI = 2.50662827463100050241576528
IPI4 = 0.75112554446


class Morlet:
    def __init__(self, bandwidth):
        self.four_wavelen = 0.9876
        self.fb = bandwidth
        self.fb2 = 2.0 * self.fb * self.fb
        self.ifb = 1.0 / self.fb
        self.imag_frequency = False
        self.doublesided = False
        self.mother = None

    def generate(self, size, scale=None, real=None, imag=None):
        if scale is None:
            # Frequency domain
            self.width = size
            toradians = (2 * PI) / size
            norm = np.sqrt(2 * PI) * IPI4
            self.mother = np.zeros(self.width)

            for w in range(self.width):
                tmp1 = 2.0 * (w * toradians) * self.fb - 2.0 * PI * self.fb
                tmp1 = -(tmp1 * tmp1) / 2
                self.mother[w] = norm * np.exp(tmp1)
        else:
            # Time domain
            self.width = self.getSupport(scale)
            norm = size * self.ifb / np.sqrt(2 * np.pi)

            for t in range(self.width * 2 + 1):
                tmp1 = (t - self.width) / scale
                tmp2 = np.exp(-(tmp1 * tmp1) / (self.fb2))

                real[t] = norm * tmp2 * np.cos(tmp1 * 2.0 * np.pi) / scale
                imag[t] = norm * tmp2 * np.sin(tmp1 * 2.0 * np.pi) / scale

    def getWavelet(self, scale, pwav, pn):
        w = self.getSupport(scale)
        real = np.zeros(max(w * 2 + 1, pn))
        imag = np.zeros(max(w * 2 + 1, pn))

        self.generate(pn, scale, real, imag)

        for t in range(pn):
            pwav[t] = complex(real[t], imag[t])

    def getSupport(self, scale):
        # Placeholder for getSupport implementation
        return int(scale * 10)  # Example implementation


class Scales:
    def __init__(self, wav, st, afs, af0, af1, afn):
        self.fs = afs
        self.scales = np.zeros(afn)
        self.fourwavl = wav.four_wavelen
        self.nscales = afn

        if st == "FCWT_LOGSCALES":
            self.calculate_logscale_array(2.0, wav.four_wavelen, afs, af0, af1, afn)
        elif st == "FCWT_LINSCALES":
            self.calculate_linscale_array(wav.four_wavelen, afs, af0, af1, afn)
        else:
            self.calculate_linfreq_array(wav.four_wavelen, afs, af0, af1, afn)

    def getScales(self, pfreqs, pnf):
        for i in range(pnf):
            pfreqs[i] = self.scales[i]

    def getFrequencies(self, pfreqs, pnf):
        for i in range(pnf):
            pfreqs[i] = self.fs / self.scales[i]

    def calculate_logscale_array(self, base, four_wavl, fs, f0, f1, fn):

        s0 = fs / f1
        s1 = fs / f0
        power0 = math.log(s0) / math.log(base)
        power1 = math.log(s1) / math.log(base)
        dpower = power1 - power0

        for i in range(fn):
            power = power0 + (dpower / (fn - 1)) * i
            self.scales[i] = base**power

    def calculate_linfreq_array(self, four_wavl, fs, f0, f1, fn):

        df = f1 - f0

        for i in range(fn):
            self.scales[fn - i - 1] = fs / (f0 + (df / fn) * i)

    def calculate_linscale_array(self, four_wavl, fs, f0, f1, fn):

        s0 = fs / f1
        s1 = fs / f0
        ds = s1 - s0

        for i in range(fn):
            self.scales[i] = s0 + (ds / fn) * i


if __name__ == "__main__":
    # Example usage of the Morlet and Scales classes
    bandwidth = 1  # Example bandwidth for the Morlet wavelet
    morlet_wavelet = Morlet(bandwidth)

    # Define scale type and frequency parameters
    scale_type = "FCWT_LINFREQS"
    afs = 1000  # Sampling frequency in Hz
    af0 = 0.1  # Start frequency in Hz
    af1 = 20  # End frequency in Hz
    afn = 10  # Number of scales

    # Initialize Scales object with the defined parameters
    scales = Scales(morlet_wavelet, scale_type, afs, af0, af1, afn)

    # Example: Get frequencies corresponding to the scales
    frequencies = np.zeros(afn)
    scales.getFrequencies(frequencies, afn)

    print("Frequencies corresponding to the scales:")
    print(frequencies)
    print("Scales:")
    print(scales.scales)
    scales.getScales(frequencies, afn)
    print("Scale:")
    print(frequencies)
