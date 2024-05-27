#include "cwt.h"

#include <cmath>
#include <complex>
#include <vector>

// Perform the CWT
// std::complex<double>** performCWT(double* signal, int n, double sampleRate, int numScales, double minFrequency, double maxFrequency) {
//     std::complex<double>** cwt = new std::complex<double>*[numScales];
//     double frequencyStep = (maxFrequency - minFrequency) / (numScales - 1);
//     for (int s = 0; s < numScales; s++) {
//         cwt[s] = new std::complex<double>[n];
//         double frequency = minFrequency + s * frequencyStep;
//         std::complex<double>* wavelet = generateMorletWaveletFrequency(n, sampleRate, frequency);
//         for (int i = 0; i < n; i++) {
//             for (int j = 0; j < n; j++) {
//                 cwt[s][i] += signal[j] * wavelet[(i - j + n) % n];
//             }
//         }
//         delete[] wavelet;
//     }
//     return cwt;
// }

std::complex<double>** performCWT(double* signal, int n, double sampleRate, int numScales, double minFrequency, double maxFrequency) {
    std::complex<double>** cwt = new std::complex<double>*[numScales];
    double frequencyStep = (maxFrequency - minFrequency) / (numScales - 1);
    for (int s = 0; s < numScales; s++) {
        cwt[s] = new std::complex<double>[n];
        double frequency = minFrequency + s * frequencyStep;
        auto result = generateMorletWaveletFrequency(sampleRate, frequency);
        std::complex<double>* wavelet = result.first;
        int wl = result.second;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                cwt[s][i] += signal[j] * wavelet[(i - j + wl) % wl];
            }
        }
        delete[] wavelet;
    }
    return cwt;
}

// Power
std::vector<double> calculatePowerSpectrum(std::complex<double>** cwt, int n, int numScales) {
    std::vector<double> powerSpectrum(n, 0.0);
    for (int i = 0; i < n; i++) {
        for (int s = 0; s < numScales; s++) {
            double magnitude = std::abs(cwt[s][i]);
            powerSpectrum[i] += magnitude * magnitude;
        }
    }
    return powerSpectrum;
}

std::vector<std::complex<double>> generateMorletWaveletVector(int n, double w0) {
    std::vector<std::complex<double>> wavelet(n);
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
    }

    return wavelet;
}

std::complex<double>* generateMorletWaveletDouble(int n, double w0) {
    std::complex<double>* wavelet = new std::complex<double>[n];
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
    }

    return wavelet;
}

std::complex<double>* generateNormalizedMorletWavelet(int n, double w0) {
    std::complex<double>* wavelet = new std::complex<double>[n];
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    // Calculate the wavelet and its norm
    double norm = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
        norm += std::norm(wavelet[i]);
    }

    // Normalize the wavelet
    for (int i = 0; i < n; i++) {
        wavelet[i] /= std::sqrt(norm);
    }

    return wavelet;
}

std::complex<double>* generateMaxLeveledMorletWavelet(int n, double w0) {
    std::complex<double>* wavelet = new std::complex<double>[n];
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    // Calculate the wavelet and its maximum level
    double maxLevel = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
        double level = std::abs(wavelet[i]);
        if (level > maxLevel) {
            maxLevel = level;
        }
    }

    // Level the wavelet to a maximum of 1
    for (int i = 0; i < n; i++) {
        wavelet[i] /= maxLevel;
    }

    return wavelet;
}

std::complex<double>* generateMorletWaveletFrequency(int n, double sampleRate, double frequency) {
    std::complex<double>* wavelet = new std::complex<double>[n];
    double w0 = 2 * M_PI * frequency / sampleRate;  // Convert frequency to radian frequency
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    // Calculate the wavelet and its maximum level
    double maxLevel = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
        double level = std::abs(wavelet[i]);
        if (level > maxLevel) {
            maxLevel = level;
        }
    }

    // Level the wavelet to a maximum of 1
    for (int i = 0; i < n; i++) {
        wavelet[i] /= maxLevel;
    }

    return wavelet;
}

std::pair<std::complex<double>*, int> generateMorletWaveletFrequency(double sampleRate, double frequency) {
    int n = (int)sampleRate / frequency;  // Calculate the number of samples based on the sample rate and frequency
    std::complex<double>* wavelet = new std::complex<double>[n];
    double w0 = 2 * M_PI * frequency / sampleRate;  // Convert frequency to radian frequency
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    // Calculate the wavelet and its maximum level
    double maxLevel = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
        double level = std::abs(wavelet[i]);
        if (level > maxLevel) {
            maxLevel = level;
        }
    }

    // Level the wavelet to a maximum of 1
    for (int i = 0; i < n; i++) {
        wavelet[i] /= maxLevel;
    }

    return std::make_pair(wavelet, n);
}

std::vector<double> generateRickerWavelet(int points, double a) {
    std::vector<double> wavelet(points);
    for (int i = 0; i < points; i++) {
        double x = ((i - points / 2) / (points / 2.0)) * a;
        wavelet[i] = (1 - 2 * M_PI * M_PI * x * x) * std::exp(-M_PI * M_PI * x * x);
    }
    return wavelet;
}

std::vector<double> generateRickerWavelet(double sampleRate, double frequency) {
    int points = (int)sampleRate / frequency;  // Calculate the number of points based on the sample rate and frequency
    double a = 1.0 / (2 * M_PI * frequency);   // Calculate the scale factor based on the frequency
    std::vector<double> wavelet(points);
    for (int i = 0; i < points; i++) {
        double x = ((i - points / 2) / (points / 2.0)) * a;
        wavelet[i] = (1 - 2 * M_PI * M_PI * x * x) * std::exp(-M_PI * M_PI * x * x);
    }
    return wavelet;
}

std::vector<double> generateGaussianWavelet(int points, double sigma) {
    std::vector<double> wavelet(points);
    for (int i = 0; i < points; i++) {
        double x = ((i - points / 2) / (points / 2.0)) * sigma;
        wavelet[i] = std::exp(-0.5 * x * x) / std::sqrt(2 * M_PI * sigma * sigma);
    }
    return wavelet;
}

std::vector<double> generateGaussianWavelet(double sampleRate, double frequency) {
    int points = (int)sampleRate / frequency;     // Calculate the number of points based on the sample rate and frequency
    double sigma = 1.0 / (2 * M_PI * frequency);  // Calculate the standard deviation based on the frequency
    std::vector<double> wavelet(points);
    for (int i = 0; i < points; i++) {
        double x = ((i - points / 2) / (points / 2.0)) * sigma;
        wavelet[i] = std::exp(-0.5 * x * x) / std::sqrt(2 * M_PI * sigma * sigma);
    }
    return wavelet;
}

std::vector<double> generateShannonWavelet(double sampleRate, double frequency) {
    // Calculate the number of samples for one period of the wavelet
    int numSamples = static_cast<int>(sampleRate / frequency);

    // Generate the wavelet
    std::vector<double> wavelet(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double x = 2.0 * M_PI * frequency * (i / sampleRate);
        if (x != 0) {
            wavelet[i] = sin(x) / x;
        } else {
            wavelet[i] = 1.0;  // sinc(0) = 1
        }
    }

    return wavelet;
}

std::vector<double> generateDbWavelet(double sampleRate, double frequency) {
    int points = (int)sampleRate / frequency;  // Calculate the number of points based on the sample rate and frequency
    std::vector<double> wavelet(points, 0.0);

    // Coefficients for the db4 wavelet
    double h[] = {0.4829629131445341, 0.8365163037378079, 0.2241438680420134, -0.1294095225512604};
    // Coefficients for the db6 wavelet
    // double h[] = {0.3326705529500825, 0.8068915093110928, 0.4598775021184914, -0.1350110200102546, -0.0854412738820267, 0.0352262918857095};
    // Coefficients for the db8 wavelet
    // double h[] = {0.2303778133088964, 0.7148465705529154, 0.6308807679298589, -0.0279837694168599, -0.1870348117190931, 0.0308413818355607, 0.0328830116668852, -0.0105974017850690};

    // Generate the wavelet
    for (int i = 0; i < points; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i + j) % points;
            wavelet[i] += h[j] * cos(2 * M_PI * frequency * index / sampleRate);
        }
    }

    return wavelet;
}
// class Coefficients {
//    private:
//     std::vector<double> db4;
//     std::vector<double> db6;
//     std::vector<double> db8;

//    public:
//     Coefficients() {
//         db4 = {0.4829629131445341, 0.8365163037378079, 0.2241438680420134, -0.1294095225512604};
//         db6 = {0.3326705529500825, 0.8068915093110928, 0.4598775021184914, -0.1350110200102546, -0.0854412738820267, 0.0352262918857095};
//         db8 = {0.2303778133088964, 0.7148465705529154, 0.6308807679298589, -0.0279837694168599, -0.1870348117190931, 0.0308413818355607, 0.0328830116668852, -0.0105974017850690};
//     }

//     std::vector<double> get(int db) {
//         switch (db) {
//             case 4:
//                 return db4;
//             case 6:
//                 return db6;
//             case 8:
//                 return db8;
//             default:
//                 throw std::invalid_argument("Invalid db value");
//         }
//     }
// };

// std::vector<double> generateDbWavelet(double sampleRate, double frequency, Coefficients& coefficients) {
//     int points = (int)sampleRate / frequency;  // Calculate the number of points based on the sample rate and frequency
//     std::vector<double> wavelet(points, 0.0);
//     std::vector<double> h = coefficients.get();

//     // Generate the wavelet
//     for (int i = 0; i < points; i++) {
//         for (int j = 0; j < h.size(); j++) {
//             int index = (i + j) % points;
//             wavelet[i] += h[j] * cos(2 * M_PI * frequency * index / sampleRate);
//         }
//     }

//     return wavelet;
// }

// std::complex<double>* generateMorletWaveletFrequency(double sampleRate, double frequency) {
//     int n = (int)sampleRate / frequency;  // Calculate the number of samples based on the sample rate and frequency
//     std::complex<double>* wavelet = new std::complex<double>[n];
//     double w0 = 2 * M_PI * frequency / sampleRate;  // Convert frequency to radian frequency
//     double sigma = 1.0 / (1.0 + std::pow(w0, 2));
//     double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

//     // Calculate the wavelet and its maximum level
//     double maxLevel = 0.0;
//     for (int i = 0; i < n; i++) {
//         double t = (i - n / 2.0) / (n / 2.0);
//         wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
//         double level = std::abs(wavelet[i]);
//         if (level > maxLevel) {
//             maxLevel = level;
//         }
//     }

//     // Level the wavelet to a maximum of 1
//     for (int i = 0; i < n; i++) {
//         wavelet[i] /= maxLevel;
//     }

//     return wavelet;
// }

void normalizeSignal(double* signal, int n) {
    double minVal = signal[0];
    double maxVal = signal[0];

    // Find the minimum and maximum values of the signal
    for (int i = 1; i < n; i++) {
        if (signal[i] < minVal) {
            minVal = signal[i];
        }
        if (signal[i] > maxVal) {
            maxVal = signal[i];
        }
    }

    // Normalize the signal
    for (int i = 0; i < n; i++) {
        signal[i] = (signal[i] - minVal) / (maxVal - minVal);
    }
}

void normalizeSignal11(double* signal, int n) {
    double maxAbsVal = std::abs(signal[0]);

    // Find the maximum absolute value of the signal
    for (int i = 1; i < n; i++) {
        double absVal = std::abs(signal[i]);
        if (absVal > maxAbsVal) {
            maxAbsVal = absVal;
        }
    }

    // Normalize the signal
    for (int i = 0; i < n; i++) {
        signal[i] /= maxAbsVal;
    }
}

void normalizeSignal4096(double* signal, int n) {
    double minVal = 0;
    double maxVal = 4096;

    // Normalize the signal
    for (int i = 0; i < n; i++) {
        signal[i] = ((signal[i] - minVal) / (maxVal - minVal)) * 2 - 1;
    }
}

std::vector<double> generateSinus(int n, double frequency, double sampleRate) {
    std::vector<double> signal(n);
    double twoPiF = 2.0 * M_PI * frequency;

    for (int i = 0; i < n; i++) {
        signal[i] = sin(twoPiF * i / sampleRate);
    }

    return signal;
}
