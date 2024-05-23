#include <cmath>
#include <complex>



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

int main() {
    int n = 1000;  // Number of points
    double sampleRate = 44100.0;  // Sample rate in Hz
    double startFrequency = 1000.0;  // Start frequency of the chirp in Hz
    double endFrequency = 20000.0;  // End frequency of the chirp in Hz
    int numScales = 100;  // Number of scales for the CWT

    // Generate the chirp signal
    double* signal = new double[n];
    generateChirp(signal, n, sampleRate, startFrequency, endFrequency);

    // Perform the CWT
    std::complex<double>** cwt = performCWT(signal, n, sampleRate, numScales);

    // Don't forget to delete the signal and cwt arrays
    delete[] signal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
}

#include "matplotlibcpp.h"
#include <vector>

namespace plt = matplotlibcpp;

// ...

void plotCWT(std::complex<double>** cwt, int n, int numScales) {
    // Convert the CWT results to magnitude
    std::vector<std::vector<double>> magnitude(numScales, std::vector<double>(n));
    for (int s = 0; s < numScales; s++) {
        for (int i = 0; i < n; i++) {
            magnitude[s][i] = std::abs(cwt[s][i]);
        }
    }

    // Plot the CWT results
    plt::imshow(magnitude, 1, plt::colormaps::jet);
    plt::colorbar();
    plt::show();
}

int main() {
    // ...

    // Perform the CWT
    std::complex<double>** cwt = performCWT(signal, n, sampleRate, numScales);

    // Plot the CWT results
    plotCWT(cwt, n, numScales);

    // Don't forget to delete the signal and cwt arrays
    delete[] signal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
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

int main() {
    // ...

    // Perform the CWT
    std::complex<double>** cwt = performCWT(signal, n, sampleRate, numScales);

    // Calculate the power spectrum
    std::vector<double> powerSpectrum = calculatePowerSpectrum(cwt, n, numScales);

    // Don't forget to delete the signal and cwt arrays
    delete[] signal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
}

// Plot the power spectrum

#include "matplotlibcpp.h"
#include <vector>

namespace plt = matplotlibcpp;

// ...

void plotPowerSpectrum(const std::vector<double>& powerSpectrum) {
    plt::plot(powerSpectrum);
    plt::title("Power Spectrum");
    plt::xlabel("Frequency");
    plt::ylabel("Power");
    plt::show();
}

int main() {
    // ...

    // Calculate the power spectrum
    std::vector<double> powerSpectrum = calculatePowerSpectrum(cwt, n, numScales);

    // Plot the power spectrum
    plotPowerSpectrum(powerSpectrum);

    // Don't forget to delete the signal and cwt arrays
    delete[] signal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
}