// #include <Eigen/Dense>
#include <cmath>
#include <math.h>
#include <complex>
#include <cstdint>

#include "../../lib/matplotlibcpp/matplotlibcpp.h"
#include "../cwt.h"
#include "pybind11/embed.h"

namespace plt = matplotlibcpp;

void plotCWT(std::complex<double>** cwt, int n, int numScales) {
    std::vector<std::vector<float>> magnitude(numScales, std::vector<float>(n));
    for (int s = 0; s < numScales; s++) {
        for (int i = 0; i < n; i++) {
            magnitude[s][i] = std::abs(cwt[s][i]);
        }
    }

    std::vector<float> flatMagnitude;
    for (const auto& row : magnitude) {
        flatMagnitude.insert(flatMagnitude.end(), row.begin(), row.end());
    }

    if (!flatMagnitude.empty()) {
        const float* flatMagnitudePtr = flatMagnitude.data();

        plt::imshow(flatMagnitudePtr, numScales, n, 1);
        plt::show();
    }
}

void generateChirp(double* signal, int n, double sampleRate, double startFrequency, double endFrequency) {
    double T = static_cast<double>(n) / sampleRate;  // Total time
    for (int i = 0; i < n; i++) {
        double t = static_cast<double>(i) / sampleRate;  // Current time
        double f = startFrequency + (endFrequency - startFrequency) * t / T;  // Current frequency
        signal[i] = std::sin(2.0 * M_PI * f * t);
    }
}

int main() {
    int n = 1000;                    // Number of points
    double sampleRate = 44100.0;     // Sample rate in Hz
    double startFrequency = 1000.0;  // Start frequency of the chirp in Hz
    double endFrequency = 20000.0;   // End frequency of the chirp in Hz
    int numScales = 100;             // Number of scales for the CWT

    // Generate the chirp signal
    double* signal = new double[n];
    generateChirp(signal, n, sampleRate, startFrequency, endFrequency);

    std::vector<double> signalVector(signal, signal + n);

    plt::plot(signalVector);
    plt::show();

    // Perform the CWT
    std::complex<double>** cwt = performCWT(signal, n, sampleRate, numScales);

    if (cwt != nullptr) {
        plotCWT(cwt, n, numScales);
    }

    // Don't forget to delete the signal and cwt arrays
    delete[] signal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
}