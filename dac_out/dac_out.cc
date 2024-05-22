// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// #include <math.h>
#include <cstdio>

#include "chirp.h"
#include "correlation.h"
#include "dac_timer.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/timer.h"
#include "math.h"
#include "splot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "wavelet.h"

// writes it to the DAC (pin 9 on the right-side header ).
// Note: The DAC outputs a max of 1.8V.
//
// To build and flash from coralmicro root:
//    make -C out -j8
//    python3 coralmicro/scripts/flashtool.py --build_dir out --elf_path
//    out/dac_out/dac_out

namespace coralmicro {
namespace {
// [start-sphinx-snippet:dac_out]

static uint64_t lastMicros_Led;
static uint64_t lastMicros;
#define LED_TIME int(1 * 1000000)
static float duration = 0.001f;
static float f0 = 7000.0f;
static float f1 = 17000.0f;
float *chirpform_F;
bool on = true;
float SampleRate = 200000.0f;
}  // namespace
// add string variable to  calculateAndPrint

// void calculateAndPrint(uint16_t *signal1, uint16_t *signal2, uint32_t nSamp,
//                        const std::string &message) {
//     uint64_t lastMicros = TimerMicros();
//     float coeff = find_coefficient(signal1, signal2, nSamp);
//     uint64_t duration_corr = TimerMicros() - lastMicros;
//     printf("%s calculation time : %lu uS\n\r", message.c_str(),
//            static_cast<uint32_t>(duration_corr));
//     printf("%s Corr. coeff: %20f\r\n", message.c_str(), coeff);
// }

// void print_autocorrelation_to_serial(double *autocorrelation, int n) {
//     for (int i = 0; i < n; i++) {
//         printf("autocorrelation[%d] = %f\n", i, autocorrelation[i]);
//     }
// }

[[noreturn]] void Main() {
    lastMicros_Led = TimerMicros();
    printf("DAC output Example!\r\n");

    // GpioConfigureInterrupt(
    //     Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
    //     [handle = xTaskGetCurrentTaskHandle()]() {
    //     xTaskResumeFromISR(handle); },
    //     /*debounce_interval_us=*/50 * 1e3);

    LedSet(Led::kStatus, on);

    // nSamp = genSampTbl(f0, SAMLERATE, 1., 0, &chirpform);
    nSamp = chirpGen(SampleRate, duration, f0, f1, 1.0, 0., &chirpform);
    uint16_t signal1[nSamp] = {0};

    // for (uint16_t i = 0; i < nSamp; i++) {
    //     signal1[i] = rand() % 65536;
    // }

    // nSamp = lchirp(SAMLERATE, duration, f0, f1, &chirpform_F);
    // chirpform = new uint16_t[nSamp];
    // for (int i = 0; i < nSamp; i++) {
    //   (chirpform)[i] = (uint16_t)(DAC_OFF + DAC_OFF * (chirpform_F)[i]);
    // }

    DacTimerInit(SampleRate);

    uint16_t *signal2 = const_cast<uint16_t *>(chirpform);

    // float *chirpformFloat = new float[nSamp];

    // for (int i = 0; i < nSamp; i++) {
    //     chirpformFloat[i] = static_cast<float>(chirpform[i]);
    // }
    double *autocorrelation = new double[2 * nSamp - 1];
    // double *array_r = new double[nSamp];

    double *chirp = GenChirpSignal(nSamp, f0, f1, SampleRate);
    double *chirpL = GenChirpSignal(nSamp, f1, f0, SampleRate);

    std::complex<double> *wavelet = generateMorletWavelet_F(nSamp,SampleRate,100000.);
    std::vector<double> realPart(nSamp), imagPart(nSamp);
    for (int i = 0; i < nSamp; i++) {
        realPart[i] = wavelet[i].real();
        // imagPart[i] = wavelet[i].imag();
    }
    std::complex<double> *wavelet5 = generateMorletWavelet_F(nSamp,SampleRate,50000.);
    std::vector<double> realPart5(nSamp), imagPart5(nSamp);
    for (int i = 0; i < nSamp; i++) {
        realPart5[i] = wavelet5[i].real();
        // imagPart5[i] = wavelet[i].imag();
    }

    while (true) {
        // vTaskSuspend(nullptr);
        // serial_Plot_U(chirpform,nSamp);

        if (TimerMicros() - lastMicros_Led >= LED_TIME) {
            lastMicros_Led = TimerMicros();
            on = !on;
            LedSet(Led::kUser, on);
            if (on) {
                normalized_autocorrelation(signal2, nSamp, autocorrelation);
                // serial_Plot_Proc(autocorrelation, 2 * nSamp - 1);
                // serial_Plot_Proc(autocorrelation, nSamp);

                // std::vector<double> norm_chirp = normalized(chirp, nSamp);
                // double *crossCorr = crossCorrelation(norm_chirp.data(), norm_chirp.data(), nSamp);
                // serial_Plot_Proc(crossCorr, nSamp);

                serial_Plot_Proc(realPart.data(), nSamp);

            } else {
                for (uint32_t i = 0; i < nSamp; i++) {
                    signal1[i] = rand() % 4096;
                }
                normalized_cross_correlation(signal2, signal1, nSamp, autocorrelation);
                // normalized_cross_correlation(chirp, chirp, nSamp, autocorrelation);
                // serial_Plot_Proc(autocorrelation, 2 * nSamp - 1);
                // serial_Plot_Proc(autocorrelation, nSamp);

                serial_Plot_Proc(realPart5.data(), nSamp);
            }
        }
    }
    // [end-sphinx-snippet:dac_out]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
    (void)param;
    coralmicro::Main();
}
