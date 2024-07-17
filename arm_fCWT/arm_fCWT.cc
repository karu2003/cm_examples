
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arm_math.h"
#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "fcwt.h"

#include <inttypes.h>

#define CPU_CLOCK_HZ (CLOCK_GetFreq(kCLOCK_CpuClk))

namespace coralmicro {
namespace {

// Функция для инициализации циклового счетчика DWT
static inline void dwt_init(void) {
    // Разрешение доступа к регистрам DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Сброс циклового счетчика
    DWT->CYCCNT = 0;
    // Включение циклового счетчика
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// Функция для получения текущего значения циклового счетчика
static inline uint32_t dwt_get_cycles(void) {
    return DWT->CYCCNT;
}

// Функция для измерения времени выполнения
// void measure_function_time(void (*func)(void)) {
void measure_function_time(std::function<void()> func) {
    dwt_init();                                // Инициализация DWT
    uint32_t start_cycles = dwt_get_cycles();  // Стартовое значение циклового счетчика
    func();
    uint32_t end_cycles = dwt_get_cycles();            // Конечное значение циклового счетчика
    uint32_t cycle_count = end_cycles - start_cycles;  // Подсчет количества тактов

    // Convert to microseconds
    float duration = (float)cycle_count / (float)CPU_CLOCK_HZ;
    printf("Duration: %f uS\n\r", duration * 1e6);
    // printf("Cycle count: %u \n\r", cycle_count);
}

__attribute__((optimize("O0"))) void add_nops(int k) {
    for (int i = 0; i < k; i++) {
        asm volatile("nop");
    }
}

void Chirp_One(float* output_signal, float start_freq, float end_freq, int num_points, float sampling_rate) {
    float duration = static_cast<float>(num_points - 1) / sampling_rate;
    for (int i = 0; i < num_points; ++i) {
        float t = static_cast<float>(i) / sampling_rate;
        float argument = 2 * PI * (t * start_freq + (t * t) / (2 * duration) * (end_freq - start_freq));
        output_signal[i] = sin(argument);
    }
}

extern "C" [[noreturn]] void app_main(void* param) {
    // uint64_t lastMicros;
    // uint32_t size = 512;  // CPU freezes with 1024 size

    int n = 1000;         // signal length
    const int fs = 1000;  // sampling frequency
    float twopi = 2.0 * 3.1415;

    const float f0 = 0.1;
    const float f1 = 20;
    const int fn = 10;

    // input: n real numbers
    float* sig = new float[n];

    // output: n x scales x 2 (complex numbers consist of two parts)
    std::complex<float>* tfm = new std::complex<float>[n * fn];

    // // initialize with 1 Hz cosine wave
    for (int i = 0; i < n; ++i) {
        sig[i] = cos(twopi * static_cast<float>(i) / static_cast<float>(fs));
    }

    // Chirp_One(sig, f0, f1, n, fs);

    Wavelet* wavelet;

    // Initialize a Morlet wavelet having sigma=1.0;
    Morlet morl(1.0f);
    wavelet = &morl;

    FCWT fcwt(wavelet, false);

    Scales scs(wavelet, FCWT_LINFREQS, fs, f0, f1, fn);

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM fCWT\n\r");
    printf("Press the user button to start the fCWT\n\r");

    while (true) {
        vTaskSuspend(nullptr);
        fcwt.cwt(&sig[0], n, &tfm[0], &scs);
        // for (int i = 0; i < scs.nscales; i++) {

        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }

        // for (int i = 0; i < n; i++) {
        //     printf("%f\n\r", sig[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }

        // printf("%d\n\r", scs.nscales);
        // for (int i = 0; i < scs.nscales; i++) {
        //     printf("%f\n\r", scs.scales[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }
    }
    delete[] sig;
    delete[] tfm;
}
}  // namespace
}  // namespace coralmicro
