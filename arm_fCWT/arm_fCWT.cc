
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arm_math.h"
#include "libs/base/gpio.h"
#include "libs/base/random.h"
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

bool RandomGenerate_M(void* buf, size_t size) {
    // Пример реализации: заполняем буфер случайными числами
    float32_t* array = (float32_t*)buf;
    for (size_t i = 0; i < size / sizeof(float32_t); ++i) {
        array[i] = (float32_t)rand() / (float32_t)RAND_MAX;  // Генерация случайного числа от 0 до 1
    }
    return true;
}

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

// Собственная реализация свертки
void manual_convolution(const float32_t* srcA, uint32_t srcALen, const float32_t* srcB, uint32_t srcBLen, float32_t* result) {
    for (uint32_t i = 0; i < srcALen + srcBLen - 1; ++i) {
        result[i] = 0.0;
        for (uint32_t j = 0; j < srcBLen; ++j) {
            if (i >= j && (i - j) < srcALen) {
                result[i] += srcA[i - j] * srcB[j];
            }
        }
    }
}

// Функция для выполнения свертки с CMSIS-DSP
void cmsis_convolution(const float32_t* srcA, uint32_t srcALen, const float32_t* srcB, uint32_t srcBLen, float32_t* result) {
    arm_conv_f32(srcA, srcALen, srcB, srcBLen, result);
}

// Функция для заполнения массивов случайными числами
void fill_array_with_random_numbers(float32_t* array, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        array[i] = (float32_t)rand() / (float32_t)RAND_MAX;  // Генерация случайного числа от 0 до 1
    }
}

// Пример измеряемой функции
void example_function(void) {
    // Код функции для измерения
    for (volatile int i = 0; i < 100000; i++);
}

__attribute__((optimize("O0"))) void add_nops(int k) {
    for (int i = 0; i < k; i++) {
        asm volatile("nop");
    }
}

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros_cmsis;
    uint64_t lastMicros_manual;
    uint32_t size = 512;  // CPU freezes with 1024 size

    int n = 1000;         // signal length
    const int fs = 1000;  // sampling frequency
    float twopi = 2.0 * 3.1415;

    // 3000 frequencies spread logartihmically between 1 and 32 Hz
    const float f0 = 0.1;
    const float f1 = 20;
    const int fn = 10;

    // input: n real numbers
    std::vector<float> sig(n);

    // input: n complex numbers
    std::vector<complex<float>> sigc(n);

    // output: n x scales x 2 (complex numbers consist of two parts)
    std::vector<complex<float>> tfm(n * fn);

    // initialize with 1 Hz cosine wave
    for (auto& el : sig) {
        el = cos(twopi * ((float)(&el - &sig[0]) / (float)fs));
    }

    // initialize with 1 Hz cosine wave
    for (auto& el : sigc) {
        el = complex<float>(cos(twopi * ((float)(&el - &sigc[0]) / (float)fs)), 0.0f);
    }

    Wavelet* wavelet;

    // Initialize a Morlet wavelet having sigma=1.0;
    Morlet morl(1.0f);
    wavelet = &morl;

    Scales scs(wavelet, FCWT_LINFREQS, fs, f0, f1, fn);

    // Find nearest power of 2
    const int nt = find2power(n);
    const int newsize = 1 << nt;

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM fCWT\n\r");
    printf("Press the user button to start the fCWT\n\r");
    // printf("Size = %d\n\r", n);
    // printf("New size = %d\n\r", newsize);
    morl.generate(newsize);

    while (true) {
        vTaskSuspend(nullptr);
        for (int i = 0; i < scs.nscales; i++) {
            float endpointf = fmin(newsize / 2.0, (newsize * 2.0) / scs.scales[i]);
            float step = scs.scales[i] / 2.0;
            int endpoint = static_cast<int>(endpointf);
            int batchsize = endpoint;
            float maximum = newsize - 1;
            int s1 = newsize - 1;
            int start = batchsize * i;
            int end = batchsize * (i + 1);
            printf("EndpointF: %f\n\r", endpointf);
            printf("Step: %f\n\r", step);
            printf("Endpoint: %d\n\r", endpoint);
            printf("Maximum: %f\n\r", maximum);
            printf("S1: %d\n\r", s1);
            printf("Start: %d\n", start);
            printf("End: %d\n", end);
            for (int q1 = start; q1 < end; q1++) {
                float q = (float)q1;
                float tmp = min(maximum, step * q);
                printf("Tmp int: %d\n", (int)tmp);
            }

            vTaskDelay(pdMS_TO_TICKS(8));
        }

        // for (int i = 0; i < n; i++) {
        //     printf("%f\n\r", sig[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }

        // for (int i = 0; i < newsize; i++) {
        //     printf("%f\n\r", morl.mother[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }
        // printf("%d\n\r", scs.nscales);
        // for (int i = 0; i < scs.nscales; i++) {
        //     printf("%f\n\r", scs.scales[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }
    }
}
}  // namespace
}  // namespace coralmicro
