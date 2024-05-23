#include <cmath>
#include <complex>
#include <vector>
#include "cwt.h"

// Perform the CWT
std::complex<double>** performCWT(double* signal, int n, double sampleRate, int numScales) {
    std::complex<double>** cwt = new std::complex<double>*[numScales];
    for (int s = 0; s < numScales; s++) {
        cwt[s] = new std::complex<double>[n];
        std::complex<double>* wavelet = generateMorletWavelet_F(n, sampleRate, s + 1);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                cwt[s][i] += signal[j] * wavelet[(i - j + n) % n];
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

std::vector<std::complex<double>> generateMorletWavelet_V(int n, double w0) {
    std::vector<std::complex<double>> wavelet(n);
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
    }

    return wavelet;
}

std::complex<double>* generateMorletWavelet(int n, double w0) {
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

std::complex<double>* generateMorletWavelet_F(int n, double sampleRate, double frequency) {
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
