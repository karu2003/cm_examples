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
        plt::figure();
        plt::imshow(flatMagnitudePtr, numScales, n, 1);
    }
}

int main() {
    int n = 500;                       // Number of points
    double sampleRate = 44100;  // Sample rate in Hz
    double startFrequency = 500.0;     // Start frequency of the chirp in Hz
    double endFrequency = 30000.0;     // End frequency of the chirp in Hz
    int numScales = 100;               // Number of scales for the CWT
    double minFrequency = 7000.0;      // Start frequency of CWT in Hz
    double maxFrequency = 17000.0;     // End frequency of CWT in Hz
    double frequency = 7000.0;         // Frequency of the sinusoidal signal in Hz

    volatile uint16_t* signal;
    volatile uint32_t nSamp;

    nSamp = chirpGen(sampleRate, n / sampleRate, startFrequency, endFrequency, 1.0, 0.0, &signal);
    // Convert the signal to double
    double* doubleSignal = new double[nSamp];
    for (int i = 0; i < nSamp; i++) {
        doubleSignal[i] = static_cast<double>(signal[i]);
    }
    normalizeSignal4096(doubleSignal, nSamp);
    std::complex<double>** cwt_chirp = performCWT(doubleSignal, nSamp, sampleRate, numScales, minFrequency, maxFrequency);

    // std::vector<double> signalVector(doubleSignal, doubleSignal + n);
    // plt::figure();
    // plt::plot(signalVector);
    // plt::title("Chirp signal " + std::to_string(startFrequency) + " to " + std::to_string(endFrequency) + " Hz " + std::to_string(nSamp) + " samples");

    std::vector<double> signalSinus = generateSinus(n, frequency, sampleRate);
    // plt::figure();
    // plt::plot(signalSinus);
    // plt::title("Sinusoidal signal " + std::to_string(frequency) + " Hz " + std::to_string(n) + " samples");
    // // Convert to double array
    int k = signalSinus.size();
    double* array = new double[k];
    std::copy(signalSinus.begin(), signalSinus.end(), array);

    std::complex<double>** cwt_sin = performCWT(array, n, sampleRate, numScales, minFrequency, maxFrequency);

    if (cwt_sin != nullptr) {
        plotCWT(cwt_sin, n, numScales);
        plt::title("CWT of sinusoidal signal " + std::to_string(frequency) + " Hz " + std::to_string(n) + " samples");
    }

    if (cwt_chirp != nullptr) {
        plotCWT(cwt_chirp, n, numScales);
        plt::title("CWT of chirp signal " + std::to_string(startFrequency) + " to " + std::to_string(endFrequency) + " Hz " + std::to_string(nSamp) + " samples");
    }

    plt::show();

    // // Convert the CWT coefficients to absolute values
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
    delete[] cwt_sin;
    delete[] cwt_chirp;

    return 0;
}