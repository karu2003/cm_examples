#ifndef CWT_H
#define CWT_H

#include <stdint.h> 
#include <complex>
#include <vector>

// Function declarations
std::complex<double>** performCWT(double* signal, int n, double sampleRate, int numScales);
std::vector<double> calculatePowerSpectrum(std::complex<double>** cwt, int n, int numScales);
std::vector<std::complex<double>> generateMorletWavelet_V(int n, double w0);
std::complex<double>* generateMorletWavelet(int n, double w0);
std::complex<double>* generateNormalizedMorletWavelet(int n, double w0);
std::complex<double>* generateMaxLeveledMorletWavelet(int n, double w0);
std::complex<double>* generateMorletWavelet_F(int n, double sampleRate, double frequency);

#endif  // CWT_H