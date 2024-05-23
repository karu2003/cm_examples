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
