
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
#define IPI4 0.75112554446f
#define PI 3.14159265358979

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

void Chirp_One(float* output_signal, float start_freq, float end_freq, int num_points, float sampling_rate) {
    float duration = static_cast<float>(num_points - 1) / sampling_rate;
    for (int i = 0; i < num_points; ++i) {
        float t = static_cast<float>(i) / sampling_rate;
        float argument = 2 * PI * (t * start_freq + (t * t) / (2 * duration) * (end_freq - start_freq));
        output_signal[i] = sin(argument);
    }
}

void morletWavelet(float32_t scale, float32_t* output, uint32_t signal_length) {
    for (int t = 0; t < signal_length; t++) {
        float32_t tau = (float32_t)t / scale;
        output[t] = expf(-0.5f * tau * tau) * cosf(5 * tau);
    }
}

void generateMorletWavelet(float32_t* wavelet, uint32_t points, float32_t frequency, float32_t sampleRate) {
    float32_t sigma = 1.0f / (2.0f * PI * frequency);
    float32_t t, gaussian, sinusoid;
    float32_t duration = (float32_t)points / sampleRate;
    float32_t step = duration / points;

    for (uint32_t n = 0; n < points; n++) {
        t = n * step;
        gaussian = expf(-0.5f * powf((t - duration / 2.0f) / sigma, 2.0f));
        sinusoid = cosf(2.0f * PI * frequency * (t - duration / 2.0f));
        wavelet[n] = gaussian * sinusoid;
    }
}

void generateMorletWaveletWithSigma(float32_t* wavelet, uint32_t points, float32_t frequency, float32_t sigma, float32_t sampleRate) {
    float32_t t, gaussian, sinusoid;
    float32_t duration = (float32_t)points / sampleRate;
    float32_t step = duration / points;
    float32_t maxAmplitude = 0.0f;

    for (uint32_t n = 0; n < points; n++) {
        t = n * step;
        gaussian = expf(-0.5f * powf((t - duration / 2.0f) / sigma, 2.0f));
        sinusoid = cosf(2.0f * PI * frequency * (t - duration / 2.0f));
        wavelet[n] = gaussian * sinusoid;
    }
}

// void generateMorletWavelet(float32_t* wavelet, uint32_t points, float32_t frequency, float32_t sigma, float32_t sampleRate) {
//     float32_t t, gaussian, sinusoid;
//     float32_t duration = (float32_t)points / sampleRate;
//     float32_t step = duration / points;
//     float32_t maxAmplitude = 0.0f;

//     // Генерация волны Морле
//     for (uint32_t n = 0; n < points; n++) {
//         t = n * step;
//         gaussian = expf(-0.5f * powf((t - duration / 2.0f) / sigma, 2.0f));
//         sinusoid = cosf(2.0f * PI * frequency * (t - duration / 2.0f));
//         wavelet[n] = gaussian * sinusoid;
//         if (fabs(wavelet[n]) > maxAmplitude) {
//             maxAmplitude = fabs(wavelet[n]);
//         }
//     }

//     // Нормализация волны Морле
//     for (uint32_t n = 0; n < points; n++) {
//         wavelet[n] /= maxAmplitude;
//     }
// }

void generateFrequencyArray(float32_t* frequency, uint32_t points, float32_t f0, float32_t f1) {
    float32_t step = (f1 - f0) / (points - 1);
    for (uint32_t i = 0; i < points; i++) {
        frequency[i] = f0 + i * step;
    }
}

// void cwt(float32_t* signal, int signal_length, float32_t* scales, int scales_length, float32_t* output) {
//     int output_length = 2 * signal_length - 1;
//     for (int i = 0; i < scales_length; i++) {
//         float32_t scale = scales[i];
//         float32_t wavelet[SIGNAL_LENGTH];
//         morletWavelet(scale, wavelet, signal_length);
//         arm_conv_f32(signal, signal_length, wavelet, signal_length, output + i * output_length);
//     }
// }

void cwt(const float32_t* signal, uint32_t signal_length, const float32_t* scales, uint32_t num_scales, float32_t* output, int fs) {
    float32_t sigma = 2.0f;
    int output_length = signal_length * 2 - 1;
    // float32_t* wavelet = new float32_t[signal_length];
    // float32_t* temp_output = new float32_t[output_length];
    float32_t* wavelet = (float32_t*)pvPortMalloc(signal_length * sizeof(float32_t));
    float32_t* temp_output = (float32_t*)pvPortMalloc(output_length * sizeof(float32_t));

    if (wavelet == nullptr || temp_output == nullptr || output == nullptr) {
        printf("Memory allocation failed\n");
        while (true) {
            // Бесконечный цикл для остановки программы
        }
    }

    for (int i = 0; i < num_scales; i++) {
        float32_t scale = scales[i];
        // printf("Scale: %f\n", scale);

        morletWavelet(scale, wavelet, signal_length);
        // generateMorletWavelet(wavelet, signal_length, scale, fs);

        // for (int j = 0; j < signal_length; j++) {
        //     printf("%f\n", wavelet[j]);
        // }

        // for (int j = 0; j < signal_length; j++) {
        //     printf("%f\n", signal[j]);
        // }

        memset(temp_output, 0, output_length * sizeof(float32_t));

        arm_conv_f32(signal, signal_length, wavelet, signal_length, temp_output);

        for (int j = 0; j < signal_length; j++) {
            // output[i * signal_length + j] = temp_output[j];
            printf("%f,%f\n", wavelet[j], temp_output[j]);
        }

        // // for (int j = 0; j < output_length; j++) {
        // //     output[i * output_length + j] = temp_output[j];
        // // }

        // for (int j = 0; j < signal_length; j++) {
        //     output[i * output_length + j] = temp_output[j];
        // }
    }

    // delete[] wavelet;
    // delete[] temp_output;
    vPortFree(wavelet);
    vPortFree(temp_output);
}

void printCWTResults(float32_t* cwtResults, int scales, int length, float f0, float f1) {
    printf("%d %d %d %d\n", length, scales, (int)f0, (int)f1);

    // int output_length = 2 * length - 1;
    int output_length = scales * length;
    for (int i = 0; i < output_length; i++) {
        printf("%f\n", cwtResults[i]);
    }
    // for (int i = 0; i < scales; i++) {
    //     // printf("Scale %d:\n", i);
    //     for (int j = 0; j < output_length; j++) {
    //         printf("%f\n", cwtResults[i * output_length + j]);
    //     }
    //     // printf("\n");
    // }
}

void printCSV(float* cwtResult, int scales, int signal_length, float freq_min, float freq_max) {
    printf("Scale,Frequency,Value\n");
    for (int i = 0; i < scales; ++i) {
        for (int j = 0; j < signal_length; ++j) {
            float frequency = freq_min + (freq_max - freq_min) * i / (scales - 1);
            printf("%d,%f,%f\n", i, frequency, cwtResult[i * signal_length + j]);
        }
    }
}

extern "C" [[noreturn]] void app_main(void* param) {
    float32_t freq_min = 3400.0f;
    float32_t freq_max = 34000.0f;
    float32_t f_c = 0.849f;
    float fb = 2.0f;

    // uint64_t lastMicros_cmsis;
    uint32_t signal_size = 1024;
    uint32_t scale = 20;
    const int fs = 192000;
    const float fstart = 7000;
    const float fend = 17000;

    float32_t* signal = (float32_t*)pvPortMalloc(signal_size * sizeof(float32_t));

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    float32_t* scales = (float32_t*)pvPortMalloc(scale * sizeof(float32_t));
    if (scales == NULL) {
        printf("Memory allocation for scales failed!\n");
        vTaskSuspend(NULL);
    }

    for (int i = 0; i < scale; i++) {
        float32_t freq = freq_min + i * (freq_max - freq_min) / (scale - 1);
        scales[i] = f_c / freq;
    }

    // generateFrequencyArray(scales, scale, freq_min, freq_max);

    // Chirp_One(signal, fstart, fend, size, fs);

    // int output_length = 2 * SIGNAL_LENGTH - 1;
    // float32_t* cwtResult = (float32_t*)pvPortMalloc(SCALES * output_length * sizeof(float32_t));
    float32_t* cwtResult = (float32_t*)pvPortMalloc(scale * signal_size * sizeof(float32_t));
    if (cwtResult == NULL) {
        printf("Memory allocation for CWT results failed!\n");
        vTaskSuspend(NULL);
    }

    printf("Starting ARM Conv CWT\n\r");
    printf("Press the user button to start the CWT\n\r");

    // for (int i = 0; i < scale; i++) {
    //     printf("%f\n", scales[i]);
    // }

    while (true) {
        vTaskSuspend(nullptr);
        // for (int i = 0; i < signal_size; i++) {
        //     printf("%f\n", signal[i]);
        // }
        cwt(signal, signal_size, scales, scale, cwtResult, fs);
        // printf("CWT done\n");
        // printCWTResults(cwtResult, SCALES, SIGNAL_LENGTH, freq_min, freq_max);
        // printCSV(cwtResult, SCALES, SIGNAL_LENGTH, freq_min, freq_max);

        // printf("CMSIS convolution duration: ");
        // measure_function_time([&]() { cwt(signal, SIGNAL_LENGTH, scales, SCALES, cwtResult); });
    }
    vPortFree(scales);
    vPortFree(cwtResult);
}
}  // namespace
}  // namespace coralmicro
