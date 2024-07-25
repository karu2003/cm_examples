
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

void generate_triangle_wave(float32_t* array, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        float t = static_cast<float>(i) / (size - 1);  // Нормализуем индекс
        array[i] = 1.0f - fabs(2.0f * t - 1.0f);       // Генерация треугольного сигнала
    }
}

void generate_square_wave(std::vector<float>& signal, float frequency, float sample_rate, float duration, float duty_cycle) {
    int total_samples = static_cast<int>(duration * sample_rate);
    signal.resize(total_samples);

    int samples_per_period = static_cast<int>(sample_rate / frequency);
    int high_samples = static_cast<int>(samples_per_period * duty_cycle);
    int low_samples = samples_per_period - high_samples;

    for (int i = 0; i < total_samples; ++i) {
        int position_in_period = i % samples_per_period;
        if (position_in_period < high_samples) {
            signal[i] = 1.0f;  // Высокое состояние
        } else {
            signal[i] = -1.0f;  // Низкое состояние
        }
    }
}

void generate_square_wave(float32_t* array, uint32_t size, float duty_cycle) {
    for (uint32_t i = 0; i < size; ++i) {
        if (i < size * duty_cycle) {
            array[i] = 1.0f;  // Первая половина массива заполняется 1
        } else {
            array[i] = 0.0f;  // Вторая половина массива заполняется 0
        }
    }
    // for (uint32_t i = 0; i < size; ++i) {
    //     if (i < size / 2) {
    //         array[i] = 1.0f;  // Первая половина массива заполняется 1
    //     } else {
    //         array[i] = 0.0f;  // Вторая половина массива заполняется 0
    //     }
    // }
}

extern "C" [[noreturn]] void app_main(void* param) {
    // uint32_t duration_cmsis = 0;
    // uint32_t duration_manual = 0;
    uint64_t lastMicros_cmsis;
    uint64_t lastMicros_manual;
    uint32_t size = 512;  // CPU freezes with 1024 size
    // Manual convolution on the array 512 duration: Duration: 6026.217248 uS
    // CMSIS convolution on the array 512 duration: Duration: 2492.818283 uS

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    // // Инициализация генератора случайных чисел
    // srand(time(NULL));

    float32_t srcA[size];
    float32_t srcB[size];

    // Заполнение массивов случайными числами
    // fill_array_with_random_numbers(srcA, 1024);
    // fill_array_with_random_numbers(srcB, 1024);
    if (!RandomGenerate(srcA, sizeof(srcA)) || !RandomGenerate(srcB, sizeof(srcB))) {
        printf("Error when generating random numbers. Exiting...\n\r");
        vTaskDelete(nullptr);
    }

    // generate_triangle_wave(srcA, size);
    generate_square_wave(srcA, size, 0.5f);
    generate_square_wave(srcB, size, 0.5f);

    // Размеры массивов
    uint32_t srcALen = size;
    uint32_t srcBLen = size;

    // Результирующие массивы (длина = srcALen + srcBLen - 1)
    uint32_t cmsis_size = size * 2 - 1;
    float32_t result_manual[cmsis_size];
    float32_t result_cmsis[cmsis_size];

    printf("Starting ARM Convolution\n\r");
    printf("Press the user button to start the convolution\n\r");

    while (true) {
        vTaskSuspend(nullptr);
        printf("Measuring\n\r");
        printf("Manual convolution duration: ");
        measure_function_time([&]() { manual_convolution(srcA, srcALen, srcB, srcBLen, result_manual); });
        printf("CMSIS convolution duration: ");
        measure_function_time([&]() { cmsis_convolution(srcA, srcALen, srcB, srcBLen, result_cmsis); });
        // for (uint32_t i = 0; i < size - 1; i++) {  // 2 * size - 1
        //     printf("%f,%f,%f,%f\n", srcA[i], srcB[i], result_manual[i], result_cmsis[i]);
        // }
        for (uint32_t i = 0; i < cmsis_size; i++) {  // 2 * size - 1
            printf("%d,%f\n", i, result_cmsis[i]);
        }
    }
}
}  // namespace
}  // namespace coralmicro
