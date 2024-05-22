#include "correlation.h"

float find_coefficient(uint16_t signal1[], uint16_t signal2[],
                       uint32_t signalLength) {
    uint16_t sum_signal1 = 0, sum_signal2 = 0, sum_signal1_signal2 = 0;
    uint32_t squareSum_signal1 = 0, squareSum_signal2 = 0;

    for (uint16_t i = 0; i < signalLength; i++) {
        sum_signal1 += signal1[i];
        sum_signal2 += signal2[i];
        sum_signal1_signal2 += signal1[i] * signal2[i];
        squareSum_signal1 += signal1[i] * signal1[i];
        squareSum_signal2 += signal2[i] * signal2[i];
    }
    float correlation =
        (float)(signalLength * sum_signal1_signal2 -
                sum_signal1 * sum_signal2) /
        sqrt((signalLength * squareSum_signal1 - sum_signal1 * sum_signal1) *
             (signalLength * squareSum_signal2 - sum_signal2 * sum_signal2));
    return correlation;
}

// void cross_correlation(uint16_t* signal1, uint16_t* signal2, int n,
//                        double* cross_correlation) {
//     for (int k = -n + 1; k < n; k++) {
//         cross_correlation[k + n - 1] = 0;
//         for (int i = std::max(0, k); i < std::min(n, n + k); i++) {
//             cross_correlation[k + n - 1] += signal1[i] * signal2[i - k];
//         }
//     }
// }

// float find_coefficient(uint16_t signal1[], uint16_t signal2[], uint32_t signalLength) {
//     float sum1 = 0, sum2 = 0, sum1Sq = 0, sum2Sq = 0, pSum = 0;
//     for(uint32_t i = 0; i < signalLength; ++i) {
//         sum1 += signal1[i];
//         sum2 += signal2[i];
//         sum1Sq += pow(signal1[i], 2);
//         sum2Sq += pow(signal2[i], 2);
//         pSum += signal1[i] * signal2[i];
//     }
//     float num = pSum - (sum1 * sum2 / signalLength);
//     float den = sqrt((sum1Sq - pow(sum1, 2) / signalLength) * (sum2Sq - pow(sum2, 2) / signalLength));
//     if(den == 0) return 0;
//     return num / den;
// }

void cross_correlation(uint16_t* signal1, uint16_t* signal2, int n,
                       double* cross_correlation) {
    double max_value = 0;
    for (int k = -n + 1; k < n; k++) {
        cross_correlation[k + n - 1] = 0;
        for (int i = std::max(0, k); i < std::min(n, n + k); i++) {
            cross_correlation[k + n - 1] += signal1[i] * signal2[i - k];
        }
        max_value = std::max(max_value, std::abs(cross_correlation[k + n - 1]));
    }
    for (int k = -n + 1; k < n; k++) {
        cross_correlation[k + n - 1] /= max_value;
    }
}

void autocorrelation(uint16_t* signal, int n, double* autocorrelation) {
    cross_correlation(signal, signal, n, autocorrelation);
}

void normalized_autocorrelation(uint16_t* signal, int n, double* autocorrelation) {
    normalized_cross_correlation(signal, signal, n, autocorrelation);
}

// #include <matplotlibcpp.h>
// #include "correlation.h" // Assuming the correlation functions are in this header

// namespace plt = matplotlibcpp;

// void plot_autocorrelation(uint16_t* signal, int n) {
//     double* autocorrelation = new double[2*n-1];
//     autocorrelation(signal, n, autocorrelation);

//     std::vector<double> x(2*n-1), y(2*n-1);
//     for (int i = 0; i < 2*n-1; i++) {
//         x[i] = i - n + 1;
//         y[i] = autocorrelation[i];
//     }

//     plt::plot(x, y);
//     plt::show();

//     delete[] autocorrelation;
// }

// void cross_correlation(uint16_t* signal1, int n1, uint16_t* signal2, int n2, double* cross_correlation) {
//     int n = std::max(n1, n2);
//     for (int k = -n + 1; k < n; k++) {
//         cross_correlation[k + n - 1] = 0;
//         for (int i = std::max(0, k); i < std::min(n, n + k); i++) {
//             uint16_t x = (i < n1) ? signal1[i] : 0;
//             uint16_t y = (i - k < n2) ? signal2[i - k] : 0;
//             cross_correlation[k + n - 1] += x * y;
//         }
//     }
// }

// #include <numeric>

// void normalized_cross_correlation(uint16_t* signal1, int n1, uint16_t* signal2, int n2, double* cross_correlation) {
//     double mean1 = std::accumulate(signal1, signal1 + n1, 0.0) / n1;
//     double mean2 = std::accumulate(signal2, signal2 + n2, 0.0) / n2;
//     double stdDev1 = std::sqrt(std::inner_product(signal1, signal1 + n1, signal1, 0.0) / n1 - mean1 * mean1);
//     double stdDev2 = std::sqrt(std::inner_product(signal2, signal2 + n2, signal2, 0.0) / n2 - mean2 * mean2);

//     int n = std::max(n1, n2);
//     for (int k = -n + 1; k < n; k++) {
//         cross_correlation[k + n - 1] = 0;
//         for (int i = std::max(0, k); i < std::min(n, n + k); i++) {
//             double x = (i < n1) ? signal1[i] - mean1 : 0;
//             double y = (i - k < n2) ? signal2[i - k] - mean2 : 0;
//             cross_correlation[k + n - 1] += x * y;
//         }
//         cross_correlation[k + n - 1] /= n * stdDev1 * stdDev2;
//     }
// }
