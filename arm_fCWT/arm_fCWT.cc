
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

extern "C" [[noreturn]] void app_main(void* param) {
    // uint64_t lastMicros;
    // uint32_t size = 512;  // CPU freezes with 1024 size

    int n = 512;          // signal length 1000
    const int fs = 1000;  // sampling frequency
    float twopi = 2.0 * 3.1415;

    const float f0 = 0.1;
    const float f1 = 20;
    const int fn = 10;

    // input: n real numbers
    std::vector<float> sig(n);

    // output: n x scales x 2 (complex numbers consist of two parts)
    std::vector<complex<float>> tfm(n * fn);

    // initialize with 1 Hz cosine wave
    for (auto& el : sig) {
        el = cos(twopi * ((float)(&el - &sig[0]) / (float)fs));
    }

    Wavelet* wavelet;

    // Initialize a Morlet wavelet having sigma=1.0;
    Morlet morl(1.0f);
    wavelet = &morl;

    FCWT fcwt(wavelet, false);

    Scales scs(wavelet, FCWT_LINFREQS, fs, f0, f1, fn);

    // fcwt.cwt(&sig[0], n, &tfm[0], &scs);

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM fCWT\n\r");
    printf("Press the user button to start the fCWT\n\r");

    while (true) {
        vTaskSuspend(nullptr);
        // for (int i = 0; i < scs.nscales; i++) {

        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }

        for (int i = 0; i < n; i++) {
            printf("%f\n\r", sig[i]);
            vTaskDelay(pdMS_TO_TICKS(8));
        }

        // printf("%d\n\r", scs.nscales);
        // for (int i = 0; i < scs.nscales; i++) {
        //     printf("%f\n\r", scs.scales[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }
    }
}
}  // namespace
}  // namespace coralmicro
