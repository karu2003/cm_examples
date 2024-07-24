
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <time.h>

#include "arm_math.h"
#include "libs/base/gpio.h"
// #include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

// #include <inttypes.h>

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

void Chirp_One(float* signal, float fstart, float fend, int signalLength, float fs, float amplitude) {
    float t;
    float k = (fend - fstart) / signalLength;
    for (int i = 0; i < signalLength; ++i) {
        t = (float)i / fs;
        signal[i] = amplitude * sin(2 * M_PI * (fstart * t + 0.5 * k * t * t));
    }
}

// void Chirp_One(float* output_signal, float start_freq, float end_freq, int num_points, float sampling_rate, float amplitude) {
//     float duration = static_cast<float>(num_points - 1) / sampling_rate;
//     for (int i = 0; i < num_points; ++i) {
//         float t = static_cast<float>(i) / sampling_rate;
//         float argument = 2 * PI * (t * start_freq + (t * t) / (2 * duration) * (end_freq - start_freq));
//         output_signal[i] = amplitude * sin(argument);
//     }
// }

float CalculateMean(const float32_t* data, uint32_t size) {
    float sum = 0.0f;
    for (uint32_t i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum / size;
}

float CalculateStdDev(const float32_t* data, uint32_t size, float mean) {
    float sum = 0.0f;
    for (uint32_t i = 0; i < size; ++i) {
        sum += (data[i] - mean) * (data[i] - mean);
    }
    return sqrt(sum / size);
}

float CalculateCorrelationCoefficient(const float32_t* srcA, const float32_t* srcB, uint32_t size) {
    float meanA = CalculateMean(srcA, size);
    float meanB = CalculateMean(srcB, size);
    float stdDevA = CalculateStdDev(srcA, size, meanA);
    float stdDevB = CalculateStdDev(srcB, size, meanB);

    float correlation = 0.0f;
    for (uint32_t i = 0; i < size; ++i) {
        correlation += (srcA[i] - meanA) * (srcB[i] - meanB);
    }
    correlation /= (size * stdDevA * stdDevB);
    return correlation;
}

void cross_correlate(const float* signal1, const float* signal2, float* result, size_t size) {
    memset(result, 0, (2 * size - 1) * sizeof(float));

    for (size_t lag = 0; lag < 2 * size - 1; ++lag) {
        for (size_t i = 0; i < size; ++i) {
            size_t j = lag - i;
            if (j < size) {
                result[lag] += signal1[i] * signal2[j];
            }
        }
    }
}

extern "C" [[noreturn]] void app_main(void* param) {
    uint32_t size = 1024;
    uint32_t i;
    float sampling_rate = 96000.0f;
    float start_freq = 7000.0f;
    float end_freq = 17000.0f;
    const float amplitude = 1.0;

    float32_t* srcA = (float32_t*)pvPortMalloc(size * sizeof(float32_t));
    float32_t* srcB = (float32_t*)pvPortMalloc(size * sizeof(float32_t));
    float32_t* result = (float32_t*)pvPortMalloc((2 * size - 1) * sizeof(float32_t));

    if (srcA == nullptr || srcB == nullptr || result == nullptr) {
        printf("Memory allocation failed\n");
        while (true) {
            // Бесконечный цикл для остановки программы
        }
    }

    // memset(srcA, 0, size * sizeof(float32_t));
    // memset(srcB, 0, size * sizeof(float32_t));
    // memset(result, 0, (2 * size - 1) * sizeof(float32_t));

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    Chirp_One(srcA, start_freq, end_freq, size, sampling_rate, amplitude);
    Chirp_One(srcB, start_freq, end_freq, size, sampling_rate, amplitude/2);

    printf("Starting ARM Correlation\n\r");
    printf("Press the user button to start the Correlation\n\r");

    while (true) {
        vTaskSuspend(nullptr);

        // printf("Correlation duration: ");
        arm_correlate_f32(srcA, size, srcB, size, result);
        // cross_correlate(srcA, srcB, result, size);

        // float correlationCoefficient = CalculateCorrelationCoefficient(srcA, srcB, size);

        // for (i = 0; i < size; i++) {
        // for (i = 0; i < size - 1; i++) {
        //     printf("%f,%f,%f\n", srcA[i], srcB[i], result[i]);
        // }

        // printf("Correlation Coefficient: %f\n", correlationCoefficient);

        // printf Correlation
        for (i = 0; i < 2 * size - 1; i++) {
            printf("%f\n", result[i]);
        }
        measure_function_time([&]() { arm_correlate_f32(srcA, size, srcB, size, result); });
        measure_function_time([&]() { cross_correlate(srcA, srcB, result, size); });
    }
    vPortFree(srcA);
    vPortFree(srcB);
    vPortFree(result);
}
}  // namespace
}  // namespace coralmicro
