
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

// void printTFM(std::complex<float>* tfm, int n, int fn, float f0, float f1) {
//     printf("%d %d %d %d\n", n, fn, (int)f0, (int)f1);

//     int totalLength = n * fn;
//     for (int i = 0; i < totalLength; ++i) {
//         printf("%f,%f\n", tfm[i].real(), tfm[i].imag());
//     }
// }

void plotTFM(std::complex<float>* tfm, int n, int fn, float f0, float f1) {
    printf("§%d %d %d %d\n", n, fn, (int)f0, (int)f1);

    int totalLength = n * fn;
    for (int i = 0; i < totalLength; ++i) {
        printf("%f,%f\n", tfm[i].real(), tfm[i].imag());
    }
    printf("\n");
}

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    float twopi = 2.0 * 3.1415;

    int n = 1000;            // signal length 1000
    const int fs = 192000;   // sampling frequency 192000
    float noise = 1.0;       // noise amplitude
    const float wvl = 2.0f;  // wavelet sigma
    const float f0 = 3400;
    const float f1 = 34000;
    const int fn = 20;  // 200
    int chirp_n = 500;
    const float fstart = 7000;
    const float fend = 17000;

    // input: n real numbers
    float* sig = new float[n];

    // output: n x scales x 2 (complex numbers consist of two parts)
    std::complex<float>* tfm = new std::complex<float>[n * fn];

    // // initialize with 1 Hz cosine wave
    // for (int i = 0; i < n; ++i) {
    //     sig[i] = cos(twopi * static_cast<float>(i) / static_cast<float>(fs));
    // }

    Chirp_One(sig, fstart, fend, n, fs);

    Wavelet* wavelet;

    // Initialize a Morlet wavelet having sigma=1.0;
    Morlet morl(wvl);
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

        // lastMicros = TimerMicros();
        fcwt.cwt(&sig[0], n, &tfm[0], &scs);
        // lastMicros = TimerMicros() - lastMicros;
        // printf("calculation time: %lu uS\n\r", static_cast<uint32_t>(lastMicros));

        // measure_function_time([&]() { fcwt.cwt(&sig[0], n, &tfm[0], &scs); });
        plotTFM(tfm, n, fn, f0, f1);

        // printTFM(tfm, n, fn, f0, f1);
    }
    delete[] sig;
    delete[] tfm;
}
}  // namespace
}  // namespace coralmicro
