#include <cmath>
#include <complex>
// #include <vector>

// std::vector<std::complex<double>> generateMorletWavelet(int n, double w0 = 5.0) {
//     std::vector<std::complex<double>> wavelet(n);
//     double sigma = 1.0 / (1.0 + std::pow(w0, 2));
//     double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

//     for (int i = 0; i < n; i++) {
//         double t = (i - n / 2.0) / (n / 2.0);
//         wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
//     }

//     return wavelet;
// }

std::complex<double>* generateMorletWavelet(int n, double w0 = 5.0) {
    std::complex<double>* wavelet = new std::complex<double>[n];
    double sigma = 1.0 / (1.0 + std::pow(w0, 2));
    double constant = 1.0 / std::sqrt(sigma * std::sqrt(M_PI));

    for (int i = 0; i < n; i++) {
        double t = (i - n / 2.0) / (n / 2.0);
        wavelet[i] = constant * std::exp(-t * t / (2 * sigma)) * std::exp(std::complex<double>(0, w0 * t));
    }

    return wavelet;
}

std::complex<double>* generateNormalizedMorletWavelet(int n, double w0 = 5.0) {
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

std::complex<double>* generateMaxLeveledMorletWavelet(int n, double w0 = 5.0) {
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

std::complex<double>* generateMorletWavelet_F(int n, double sampleRate, double frequency = 18000.0) {
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

// #include "matplotlibcpp.h"
// #include <vector>

// namespace plt = matplotlibcpp;

// int n = 1000;  // Number of points
// std::complex<double>* wavelet = generateMorletWavelet(n);

// // Separate the real and imaginary parts
// std::vector<double> realPart(n), imagPart(n);
// for (int i = 0; i < n; i++) {
//     realPart[i] = wavelet[i].real();
//     imagPart[i] = wavelet[i].imag();
// }

// // Plot the real part
// plt::figure();
// plt::plot(realPart);
// plt::title("Real part of Morlet wavelet");

// // Plot the imaginary part
// plt::figure();
// plt::plot(imagPart);
// plt::title("Imaginary part of Morlet wavelet");

// // Show the plots
// plt::show();

// // Don't forget to delete the wavelet array
// delete[] wavelet;