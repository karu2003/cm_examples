
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

#define SIGNAL_LENGTH 500  // Длина сигнала
#define SCALES 20          // Количество масштабов
#define PI 3.14159265358979

void Chirp_One(float* output_signal, float start_freq, float end_freq, int num_points, float sampling_rate) {
    float duration = static_cast<float>(num_points - 1) / sampling_rate;
    for (int i = 0; i < num_points; ++i) {
        float t = static_cast<float>(i) / sampling_rate;
        float argument = 2 * PI * (t * start_freq + (t * t) / (2 * duration) * (end_freq - start_freq));
        output_signal[i] = sin(argument);
    }
}

void morletWavelet(float32_t scale, float32_t* output, int length) {
    for (int t = 0; t < length; t++) {
        float32_t tau = (float32_t)t / scale;
        output[t] = expf(-0.5f * tau * tau) * cosf(5 * tau);
    }
}

// void morletWavelet(float32_t scale, float32_t* output, int length) {
//     for (int t = 0; t < length; t++) {
//         float32_t tau = (float32_t)t / scale;
//         output[t] = expf(-0.5f * tau * tau) * cosf(5 * tau);
//     }
// }

void cwt(float32_t* signal, int signal_length, float32_t* scales, int scales_length, float32_t* output) {
    for (int i = 0; i < scales_length; i++) {
        float32_t scale = scales[i];
        float32_t wavelet[SIGNAL_LENGTH];
        morletWavelet(scale, wavelet, signal_length);
        // printf("Scale %f:\n", scale);
        // for (int j = 0; j < signal_length; j++) {
        //     printf("%f\n", wavelet[j]);
        //     // vTaskDelay(pdMS_TO_TICKS(8));
        // }

        arm_conv_f32(signal, signal_length, wavelet, signal_length, output + i * signal_length);
    }
}

// void cwt(float32_t* signal, int signal_length, float32_t* scales, int scales_length, float32_t* output) {
//     int output_length = 2 * signal_length - 1;
//     for (int i = 0; i < scales_length; i++) {
//         float32_t scale = scales[i];
//         float32_t wavelet[SIGNAL_LENGTH];
//         morletWavelet(scale, wavelet, signal_length);
        
//         // Свертка сигнала с вейвлетом
//         arm_conv_f32(signal, signal_length, wavelet, signal_length, output + i * output_length);
//     }
// }

void printCWTResults(float32_t* cwtResults, int scales, int length, float f0, float f1) {
    printf("%d %d %d %d\n", length, scales, (int)f0, (int)f1);
    for (int i = 0; i < scales; i++) {
        // printf("Scale %d:\n", i);
        for (int j = 0; j < length; j++) {
            printf("%f\n", cwtResults[i * length + j]);
        }
        // printf("\n");
    }
}

// void printCWTResults(float32_t* cwtResults, int scales, int length) {
//     int output_length = 2 * length - 1;
//     for (int i = 0; i < scales; i++) {
//         printf("Scale %d:\n", i);
//         for (int j = 0; j < output_length; j++) {
//             printf("%f ", cwtResults[i * output_length + j]);
//         }
//         printf("\n");
//     }
// }

extern "C" [[noreturn]] void app_main(void* param) {
    float32_t freq_min = 7000.0f;   // Минимальная частота в Гц
    float32_t freq_max = 17000.0f;  // Максимальная частота в Гц
    float32_t f_c = 0.849f;         // Центральная частота вейвлета Морле

    // uint64_t lastMicros_cmsis;
    float32_t signal[SIGNAL_LENGTH];
    // float32_t cwtResult[SCALES][SIGNAL_LENGTH];
    const int fs = 192000;
    const float fstart = 7000;
    const float fend = 17000;

    float32_t* scales = (float32_t*)pvPortMalloc(SCALES * sizeof(float32_t));
    if (scales == NULL) {
        printf("Memory allocation for scales failed!\n");
        vTaskSuspend(NULL);  // Останавливаем задачу, если не удалось выделить память
    }

    // for (int i = 0; i < SCALES; i++) {
    //     scales[i] = 1.0f + (float32_t)i / 2.0f;
    // }

    // Заполнение массива масштабов
    for (int i = 0; i < SCALES; i++) {
        float32_t freq = freq_min + i * (freq_max - freq_min) / (SCALES - 1);
        scales[i] = f_c / freq;
    }

    // printf("Scales:\n");
    // for (int i = 0; i < SCALES; i++) {
    //     printf("%f\n", scales[i]);
    // }

    int output_length = 2 * SIGNAL_LENGTH - 1;

    // Выделение памяти под результаты CWT
    cwtResult = (float32_t*)pvPortMalloc(SCALES * output_length * sizeof(float32_t));
    if (cwtResult == NULL) {
        printf("Memory allocation for CWT results failed!\n");
        vTaskSuspend(NULL);  // Останавливаем задачу, если не удалось выделить память
    }

    Chirp_One(signal, fstart, fend, SIGNAL_LENGTH, fs);

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM Conv CWT\n\r");
    printf("Press the user button to start the CWT\n\r");

    while (true) {
        vTaskSuspend(nullptr);
        // for (int i = 0; i < SIGNAL_LENGTH; i++) {
        //     printf("%f\n", signal[i]);
        // }
        cwt(signal, SIGNAL_LENGTH, scales, SCALES, cwtResult);
        printf("CWT done\n");
        // plotCWT(cwtResult, SIGNAL_LENGTH, SCALES, fstart, fend);
        printCWTResults(cwtResult, SCALES, SIGNAL_LENGTH, fstart, fend);

        // printf("CMSIS convolution duration: ");
        // measure_function_time([&]() { cmsis_convolution(srcA, srcALen, srcB, srcBLen, result_cmsis); });
    }
    vPortFree(scales);
    vPortFree(cwtResult);
}
}  // namespace
}  // namespace coralmicro
