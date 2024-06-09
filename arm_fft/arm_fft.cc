#include <../out/mic_kissfft/test_signal.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm_math.h"
#include "hamming.h"
#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

// arm_rfft_fast_instance_f32 fft_instance;
// arm_cfft_radix4_instance_f32 fft_instance;

int find2power(int n) {
    int m, m2;
    m = 0;
    m2 = 1 << m; /* 2 to the power of m */
    while (m2 - n < 0) {
        m++;
        m2 <<= 1; /* m2 = m2*2 */
    }
    return (m);
}

void normalize(float* data, int size) {
    float min_val = *std::min_element(data, data + size);
    float max_val = *std::max_element(data, data + size);

    for (int i = 0; i < size; ++i) {
        data[i] = (data[i] - min_val) / (max_val - min_val);
    }
}

Hamming hamming_window(NSAMP);

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    int k;
    extern float pattern1[];

    float heavyside_function[NSAMP];

    for (int i = 0; i < NSAMP; ++i) {
        heavyside_function[i] = pattern1[i];
    }

    hamming_window.applyWindow(heavyside_function);

    const int nt = find2power(NSAMP);
    const int newsize = 1 << nt;

    float32_t* cin = new float32_t[newsize];
    float32_t* cout = new float32_t[newsize];
    float32_t* power = new float32_t[newsize / 2];

    memset(cin, 0, newsize * sizeof(float32_t));
    memset(cout, 0, newsize * sizeof(float32_t));
    memset(power, 0, newsize * sizeof(float32_t));

    for (k = 0; k < NSAMP; k++) {
        cin[k] = heavyside_function[k];
    }

    arm_rfft_fast_instance_f32 fft_instance;
    arm_status status;

    status = arm_rfft_fast_init_f32(&fft_instance, newsize);

    if (status != ARM_MATH_SUCCESS) {
        printf("Error initializing FFT\n\r");
        while (true) {
        }
    }

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM FFT\n\r");
    printf("Press the user button to start the FFT\n\r");
    printf("New size: %d\n\r", newsize);

    while (true) {
        vTaskSuspend(nullptr);
        memset(cin, 0, newsize * sizeof(float32_t));

        for (int i = 0; i < NSAMP; i++) {
            cin[i] = heavyside_function[i];
        }

        lastMicros = TimerMicros();
        arm_rfft_fast_f32(&fft_instance, cin, cout, 0);
        // arm_rfft_f32(&fft_instance, cin, cout);
        lastMicros = TimerMicros() - lastMicros;

        for (int i = 0; i < newsize / 2; ++i) {
            float real = cout[2 * i];
            float imag = cout[2 * i + 1];
            power[i] = real * real + imag * imag;
        }

        float min_val, max_val;
        arm_min_f32(power, newsize / 2, &min_val, NULL);
        arm_max_f32(power, newsize / 2, &max_val, NULL);

        for (int i = 0; i < newsize / 2; ++i) {
            power[i] = (power[i] - min_val) / (max_val - min_val);
        }

        // normalize(power, newsize / 2);

        for (int i = 0; i < newsize / 2; ++i) {
            printf("%f\n\r", power[i]);
            vTaskDelay(pdMS_TO_TICKS(8));
        }

        printf("calculation time: %lu uS\n\r", static_cast<uint32_t>(lastMicros));
    }

    delete[] cin;
    delete[] cout;
}
}  // namespace
}  // namespace coralmicro
