
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

namespace coralmicro {
namespace {

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

extern "C" [[noreturn]] void app_main(void* param) {
    uint32_t duration_cmsis = 0;
    uint32_t duration_manual = 0;
    uint64_t lastMicros_cmsis;
    uint64_t lastMicros_manual;
    printf("Starting ARM Convolution\n\r");
    printf("Press the user button to start the convolution\n\r");

    // // Инициализация генератора случайных чисел
    // srand(time(NULL));

    // Исходные массивы данных (размер 1024)
    float32_t srcA[1024];
    float32_t srcB[1024];

    // Заполнение массивов случайными числами
    // fill_array_with_random_numbers(srcA, 1024);
    // fill_array_with_random_numbers(srcB, 1024);
    if (!RandomGenerate(srcA, sizeof(srcA)) || !RandomGenerate(srcB, sizeof(srcB))) {
        printf("Ошибка при генерации случайных чисел.\n");
        vTaskDelete(nullptr);
    }

    // Размеры массивов
    uint32_t srcALen = 1024;
    uint32_t srcBLen = 1024;

    // Результирующие массивы (длина = srcALen + srcBLen - 1)
    float32_t result_manual[2047];
    float32_t result_cmsis[2047];

    // // Измерение времени выполнения для собственной реализации
    // TickType_t start_manual = xTaskGetTickCount();
    // manual_convolution(srcA, srcALen, srcB, srcBLen, result_manual);
    // TickType_t end_manual = xTaskGetTickCount();
    // duration_manual = (end_manual - start_manual) * portTICK_PERIOD_MS * 1000;  // Время в наносекундах

    // // Измерение времени выполнения для CMSIS-DSP
    // TickType_t start_cmsis = xTaskGetTickCount();
    // cmsis_convolution(srcA, srcALen, srcB, srcBLen, result_cmsis);
    // TickType_t end_cmsis = xTaskGetTickCount();
    // duration_cmsis = (end_cmsis - start_cmsis) * portTICK_PERIOD_MS * 1000;  // Время в наносекундах

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    while (true) {
        vTaskSuspend(nullptr);
        printf("Manual convolution time: %lu ns\n", duration_manual);
        printf("CMSIS convolution time: %lu ns\n", duration_cmsis);
    }
}
}  // namespace
}  // namespace coralmicro
