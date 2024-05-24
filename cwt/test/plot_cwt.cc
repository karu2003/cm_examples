// #include <Eigen/Dense>
#include <math.h>

#include <cmath>
#include <complex>
#include <cstdint>

#include "../../lib/matplotlibcpp/matplotlibcpp.h"
#include "../cwt.h"
#include "../dac_out/chirp.h"
// #include "pybind11/embed.h"

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
        double t = static_cast<double>(i) / sampleRate;                       // Current time
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
    double minFrequency = 5000.0;    // Start frequency of the chirp in Hz
    double maxFrequency = 10000.0;   // End frequency of the chirp in Hz
    volatile uint16_t* signal;
    volatile uint32_t nSamp;

    // Generate the chirp signal
    nSamp = chirpGen(sampleRate, n / sampleRate, startFrequency, endFrequency, 1.0, 0.0, &signal);
    // Convert the signal to double
    double* doubleSignal = new double[nSamp];
    for (int i = 0; i < nSamp; i++) {
        doubleSignal[i] = static_cast<double>(signal[i]);
    }

    std::vector<double> signalVector(signal, signal + n);
    plt::plot(signalVector);
    plt::show();

    // Perform the CWT
    // std::complex<double>** cwt = performCWT(signal, n, sampleRate, numScales, minFrequency, maxFrequency);

    // if (cwt != nullptr) {
    //     plotCWT(cwt, n, numScales);
    // }

    // Convert the CWT coefficients to absolute values
    // std::vector<std::vector<double>> absCwt(cwt.size(), std::vector<double>(cwt[0].size()));
    // for (size_t i = 0; i < cwt.size(); i++) {
    //     for (size_t j = 0; j < cwt[i].size(); j++) {
    //         absCwt[i][j] = std::abs(cwt[i][j]);
    //     }
    // }

    // // Plot the scalogram
    // plt::figure();
    // plt::imshow(absCwt, plt::extent(times[0], times.back(), frequencies[0], frequencies.back()), plt::origin::lower, plt::interpolation::bilinear);
    // // plt::colorbar();
    // plt::xlabel("Time");
    // plt::ylabel("Frequency");
    // plt::title("Scalogram");
    // plt::show();

    // Don't forget to delete the signal and cwt arrays
    // delete[] signal;
    // for (int s = 0; s < numScales; s++) {
    //     delete[] cwt[s];
    // }
    // delete[] cwt;

    return 0;
}