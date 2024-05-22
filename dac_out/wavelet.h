#ifndef WAVELET_H
#define WAVELET_H

#include <cmath>
#include <complex>

// Function to generate a Morlet wavelet
std::complex<double>* generateMorletWavelet(int n, double w0 = 5.0);
std::complex<double>* generateNormalizedMorletWavelet(int n, double w0 = 5.0);
std::complex<double>* generateMaxLeveledMorletWavelet(int n, double w0 = 5.0);
std::complex<double>* generateMorletWavelet_F(int n, double sampleRate, double frequency = 18000.0);

#endif // WAVELET_H