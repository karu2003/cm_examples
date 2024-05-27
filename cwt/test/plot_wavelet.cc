#include <math.h>

#include <cmath>
#include <complex>
#include <cstdint>
#include <utility>

#include "../../lib/matplotlibcpp/matplotlibcpp.h"
#include "../cwt.h"

namespace plt = matplotlibcpp;

int main() {
    int k = 100;                  // Number of points
    double w0 = 5.0;              // Frequency parameter
    double sampleRate = 192000.0;  // Sample rate
    double frequency = 500.0;   // Frequency

    // std::complex<double>* wavelet = generateMorletWavelet(k, w0);
    // std::complex<double>* wavelet = generateMorletWaveletFrequency(k, sampleRate, frequency);
    auto result = generateMorletWaveletFrequency(sampleRate, frequency);
    std::complex<double>* wavelet = result.first;
    int n = result.second;
    printf("n = %d\n", n);

    // Separate the real and imaginary parts
    std::vector<double> realPart(n), imagPart(n);
    for (int i = 0; i < n; i++) {
        realPart[i] = wavelet[i].real();
        imagPart[i] = wavelet[i].imag();
    }

    // Plot the real part
    plt::figure();
    plt::plot(realPart);
    plt::title("Real part of Morlet wavelet");

    // Plot the imaginary part
    plt::figure();
    plt::plot(imagPart);
    plt::title("Imaginary part of Morlet wavelet");

    // Show the plots
    plt::show();
    // Don't forget to delete the wavelet array
    delete[] wavelet;

    return 0;
}
