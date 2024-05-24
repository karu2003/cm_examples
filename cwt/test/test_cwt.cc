#include <cmath>
#include <complex>
#include <cstdint>

#include "../cwt.h"
#include "../dac_out/chirp.h"

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

    nSamp = chirpGen(sampleRate, n / sampleRate, startFrequency, endFrequency, 1.0, 0.0, &signal);
    // Convert the signal to double
    double* doubleSignal = new double[nSamp];
    for (int i = 0; i < nSamp; i++) {
        doubleSignal[i] = static_cast<double>(signal[i]);
    }

    std::complex<double>** cwt = performCWT(doubleSignal, n, sampleRate, numScales, minFrequency, maxFrequency);

    delete[] doubleSignal;
    for (int s = 0; s < numScales; s++) {
        delete[] cwt[s];
    }
    delete[] cwt;

    return 0;
}
