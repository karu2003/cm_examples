
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arm_math.h"
// #include "board.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/fsl_device_registers.h"
#include "libs/base/gpio.h"
#include "libs/base/random.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

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

    func();  // Вызов измеряемой функции

    uint32_t end_cycles = dwt_get_cycles();            // Конечное значение циклового счетчика
    uint32_t cycle_count = end_cycles - start_cycles;  // Подсчет количества тактов

    // Перевод в наносекунды
    uint64_t time_ns = (uint64_t)cycle_count * 1000000000 / CPU_CLOCK_HZ;

    printf("Function execution time: %llu nS\n\r", time_ns);
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

// // Функция-обёртка для manual_convolution
// void manual_convolution_wrapper(void) {
//     manual_convolution(srcA, srcALen, srcB, srcBLen, result_manual);
// }

// void cmsis_convolution_wrapper(void) {
//     cmsis_convolution(srcA, srcALen, srcB, srcBLen, result_cmsis);
// }

// Пример измеряемой функции
void example_function(void) {
    // Код функции для измерения
    for (volatile int i = 0; i < 100000; i++);
}

extern "C" [[noreturn]] void app_main(void* param) {
    // BOARD_InitBootPins();
    // BOARD_InitBootClocks();
    // BOARD_InitDebugConsole();

    uint32_t duration_cmsis = 0;
    uint32_t duration_manual = 0;
    uint64_t lastMicros_cmsis;
    uint64_t lastMicros_manual;

    // SystemCoreClockUpdate();

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    // // Инициализация генератора случайных чисел
    // srand(time(NULL));

    // Исходные массивы данных (размер 1024)
    float32_t srcA[1024];
    float32_t srcB[1024];

    // Заполнение массивов случайными числами
    // fill_array_with_random_numbers(srcA, 1024);
    // fill_array_with_random_numbers(srcB, 1024);
    if (!RandomGenerate(srcA, sizeof(srcA)) || !RandomGenerate(srcB, sizeof(srcB))) {
        printf("Error when generating random numbers. Exiting...\n\r");
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

    printf("Starting ARM Convolution\n\r");
    printf("Press the user button to start the convolution\n\r");

    while (true) {
        vTaskSuspend(nullptr);
        printf("Measuring\n\r");
        measure_function_time([&]() { example_function(); });
        dwt_init();                                // Инициализация DWT
        // add 100 nop ASM instructions
        
        
        for (volatile int i = 0; i < 100; i++);
        TickType_t start_cycles = xTaskGetTickCount();  // Стартовое значение циклового счетчика

        uint32_t start_cycles = dwt_get_cycles();  // Стартовое значение циклового счетчика
        printf("Start cycles: %u\n\r", start_cycles);
                                                   // measure_function_time([&]() { manual_convolution(srcA, srcALen, srcB, srcBLen, result_manual); });
                                                   // measure_function_time([&]() { cmsis_convolution(srcA, srcALen, srcB, srcBLen, result_cmsis); });
                                                   // measure_function_time(manual_convolution_wrapper, "manual_convolution");
                                                   // measure_function_time(cmsis_convolution_wrapper, "cmsis_convolution");
                                                   // printf("Manual convolution time: %lu ns\n", duration_manual);
                                                   // printf("CMSIS convolution time: %lu ns\n", duration_cmsis);
    }
}
}  // namespace
}  // namespace coralmicro
