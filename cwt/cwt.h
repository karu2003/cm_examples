#ifndef CWT_H
#define CWT_H

#include <stdint.h> 
#include <complex>
#include <vector>

// Function declarations
std::complex<double>** performCWT(double* signal, int n, double sampleRate, int numScales, double minFrequency, double maxFrequency);
std::vector<double> calculatePowerSpectrum(std::complex<double>** cwt, int n, int numScales);
std::vector<std::complex<double>> generateMorletWaveletVector(int n, double w0);
std::complex<double>* generateMorletWavelet(int n, double w0);
std::complex<double>* generateNormalizedMorletWavelet(int n, double w0);
std::complex<double>* generateMaxLeveledMorletWavelet(int n, double w0);
std::complex<double>* generateMorletWaveletFrequency(int n, double sampleRate, double frequency);
// std::complex<double>* generateMorletWaveletFrequency(double sampleRate, double frequency);
std::pair<std::complex<double>*, int> generateMorletWaveletFrequency(double sampleRate, double frequency);
void normalizeSignal(double* signal, int n);
void normalizeSignal11(double* signal, int n);
void normalizeSignal4096(double* signal, int n);
std::vector<double> generateSinus(int n, double frequency, double sampleRate);

// std::vector<double> generateDbWavelet(double sampleRate, double frequency, Coefficients& coefficients);

// class Coefficients {
//    private:
//     std::vector<double> db4;
//     std::vector<double> db6;
//     std::vector<double> db8;

//    public:
//     Coefficients();
//     std::vector<double> get(int db);
// };

#endif  // CWT_H