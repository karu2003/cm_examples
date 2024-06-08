#include "pffft.h"

#include <../out/mic_kissfft/test_signal.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <cmath>

// #include "../lib/hamming/hamming.h"
#include "hamming.h"
#include "fftpack.h"
#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

Hamming hamming_window(NSAMP);

void normalize(float *data, int size) {
    float min_val = *std::min_element(data, data + size);
    float max_val = *std::max_element(data, data + size);

    for (int i = 0; i < size; ++i) {
        data[i] = (data[i] - min_val) / (max_val - min_val);
    }
}

extern "C" [[noreturn]] void app_main(void *param) {
    uint64_t lastMicros;
    int k;
    float freqs[NSAMP];
    // initialize input data for FFT
    // float input[] = {11.0f, 3.0f, 4.05f, 9.0f, 10.3f, 8.0f, 4.934f, 5.11f};
    // int NSAMP = sizeof(input) / sizeof(float);
    extern float pattern1[];

    float *cin = (float *)pffft_aligned_malloc(NSAMP * sizeof(float));
    float *fft_out = (float *)pffft_aligned_malloc(NSAMP * sizeof(float));
    float *work = (float *)pffft_aligned_malloc(NSAMP * sizeof(float));

    float heavyside_function[NSAMP];

    PFFFT_Setup *setup = pffft_new_setup(NSAMP, PFFFT_REAL);  // PFFFT_COMPLEX

    // initialize data storage
    memset(cin, 0, NSAMP * sizeof(float));
    memset(fft_out, 0, NSAMP * sizeof(float));
    memset(work, 0, NSAMP * sizeof(float));

    for (k = 0; k < NSAMP; k++) {
        heavyside_function[k] = pattern1[k];
    }

    hamming_window.applyWindow(heavyside_function);

    for (k = 0; k < NSAMP; k++) {
        cin[k] = heavyside_function[k];
    }

    float f_max = FSAMP;
    float f_res = f_max / NSAMP;
    for (int i = 0; i < NSAMP; i++) {
        freqs[i] = f_res * i;
    }

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("PFFFT: Real FFT\n\r");

    while (true) {
        vTaskSuspend(nullptr);

        lastMicros = TimerMicros();
        pffft_transform_ordered(setup, cin, fft_out, work, PFFFT_FORWARD);
        // pffft_transform(setup, cin, fft_out, work, PFFFT_FORWARD);
        lastMicros = TimerMicros() - lastMicros;

        float *power = (float *)pffft_aligned_malloc(NSAMP / 2 * sizeof(float));
        for (int i = 0; i < NSAMP / 2; ++i) {
            float real = fft_out[2 * i];
            float imag = fft_out[2 * i + 1];
            power[i] = real * real + imag * imag;
        }

        normalize(power, NSAMP / 2);

        for (int i = 0; i < NSAMP / 2; ++i) {
            printf("%f\n\r", power[i]);
            vTaskDelay(pdMS_TO_TICKS(8));
        }

        // float max_power = 0;
        // int max_idx = 0;

        // for (int i = 0; i < NSAMP / 2; ++i) {
        //     printf("%f,%f\n\r", freqs[i], power[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        //     if (power[i] > max_power) {
        //         max_power = power[i];
        //         max_idx = i;
        //     }
        // }

        // // any frequency bin over NSAMP/2 is aliased (nyquist sampling theorum)
        // for (i = 0; i < NSAMP / 2; i++) {
        //     float power = fft_out[i] * fft_out[i];
        //     printf("%f,%f\n\r", freqs[i], power);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        //     if (power > max_power) {
        //         max_power = power;
        //         max_idx = i;
        //     }
        // }

        // printf("calculation time: %lu uS amount of points %d\n\r", static_cast<uint32_t>(lastMicros), NSAMP);
    }
    pffft_destroy_setup(setup);
    pffft_aligned_free(cin);
    pffft_aligned_free(fft_out);
    pffft_aligned_free(work);
}
}  // namespace
}  // namespace coralmicro
