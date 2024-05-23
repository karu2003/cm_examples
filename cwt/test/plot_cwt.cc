#include <Eigen/Dense>
#include <cmath>
#include <complex>
#include <cstdint>

#include "../../lib/matplotlibcpp/matplotlibcpp.h"
#include "../cwt.h"
#include "../dac_out/chirp.h"

namespace plt = matplotlibcpp;

void plotCWT(std::complex<double>** cwt, int n, int numScales) {
    // // Convert the CWT results to magnitude
    // std::vector<std::vector<double>> magnitude(numScales, std::vector<double>(n));
    // for (int s = 0; s < numScales; s++) {
    //     for (int i = 0; i < n; i++) {
    //         magnitude[s][i] = std::abs(cwt[s][i]);
    //     }
    // }

    // Eigen::MatrixXd magnitude(numScales, n);
    // for (int s = 0; s < numScales; s++) {
    //     for (int i = 0; i < n; i++) {
    //         magnitude(s, i) = std::abs(cwt[s][i]);
    //     }
    // }

    // double** magnitude = new double*[numScales];
    // for (int s = 0; s < numScales; s++) {
    //     magnitude[s] = new double[n];
    //     for (int i = 0; i < n; i++) {
    //         magnitude[s][i] = std::abs(cwt[s][i]);
    //     }
    // }

    std::vector<std::vector<float>> magnitude(numScales, std::vector<float>(n));
    for (int s = 0; s < numScales; s++) {
        for (int i = 0; i < n; i++) {
            magnitude[s][i] = std::abs(cwt[s][i]);
        }
    }

    // Convert the magnitude to an Eigen::MatrixXd
    // Eigen::MatrixXd mat(numScales, n);
    // for (int s = 0; s < numScales; s++) {
    //     for (int i = 0; i < n; i++) {
    //         mat(s, i) = magnitude[s][i];
    //     }
    // }

    // plt::imshow(magnitude, 1, plt::colormaps::jet);
    // pybind11::exec(R"(import matplotlib.pyplot as plt plt.set_cmap('jet'))");
    // plt::imshow(magnitude, 1);

    const float* magnitude_ptr = &(magnitude[0]);
    plt::imshow(magnitude_ptr);
    // plt::imshow(mat);
    plt::colorbar();
    plt::show();
}

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