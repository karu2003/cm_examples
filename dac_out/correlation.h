#ifndef CORRELATION_H
#define CORRELATION_H

// #include <stdlib.h>
#include <math.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>

float find_coefficient(uint16_t signal1[], uint16_t signal2[], uint32_t signalLength);
void cross_correlation(uint16_t* signal1, uint16_t* signal2, int n, double* cross_correlation);
// void normalized_cross_correlation(uint16_t* signal1, uint16_t* signal2, int n, double* cross_correlation);
void autocorrelation(uint16_t* signal, int n, double* autocorrelation);
void normalized_autocorrelation(uint16_t* signal, int n, double* autocorrelation);

template <typename T>
T* crossCorrelation(const T* signalA, const T* signalB, int n) {
    T* result = new T[n];
    for (int i = 0; i < n; i++) {
        result[i] = 0;
    }
    for (int delay = 0; delay < n; delay++) {
        for (int i = 0; i < n; i++) {
            if (i + delay < n) {
                result[delay] += signalA[i] * signalB[i + delay];
            }
        }
    }
    return result;
}

template <typename T>
static std::vector<T> normalized(T* signal, int n) {
    std::vector<T> normalized_signal(n);

    T mean = std::accumulate(signal, signal + n, 0.0) / n;
    T stdDev = std::sqrt(std::inner_product(signal, signal + n, signal, 0.0) / n - mean * mean);

    for (int i = 0; i < n; i++) {
        normalized_signal[i] = (signal[i] - mean) / stdDev;
    }

    return normalized_signal;
}

template <typename T>
void normalized_cross_correlation(T* signal1, T* signal2, int n, double* cross_correlation) {
    double mean1 = std::accumulate(signal1, signal1 + n, 0.0) / n;
    double mean2 = std::accumulate(signal2, signal2 + n, 0.0) / n;
    double stdDev1 = std::sqrt(std::inner_product(signal1, signal1 + n, signal1, 0.0) / n - mean1 * mean1);
    double stdDev2 = std::sqrt(std::inner_product(signal2, signal2 + n, signal2, 0.0) / n - mean2 * mean2);

    for (int k = -n + 1; k < n; k++) {
        cross_correlation[k + n - 1] = 0;
        for (int i = std::max(0, k); i < std::min(n, n + k); i++) {
            cross_correlation[k + n - 1] += (signal1[i] - mean1) * (signal2[i - k] - mean2);
        }
        cross_correlation[k + n - 1] /= n * stdDev1 * stdDev2;
    }
}

#endif  // CORRELATION_H