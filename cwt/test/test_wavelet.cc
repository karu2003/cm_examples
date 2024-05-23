#include <cmath>
#include <complex>
#include <vector>
#include "../cwt.h"


#include "matplotlibcpp.h"
#include <vector>

namespace plt = matplotlibcpp;

int n = 1000;  // Number of points
std::complex<double>* wavelet = generateMorletWavelet(n);

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