#include <../out/mic_kissfft/test_signal.h>
// #include <limits.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <complex>
// #include <vector>
// #include <memory>

#include "arm_math.h"
// #include "hamming.h"
#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

#define FFT_SIZE 1024
#define SAMPLING_FREQUENCY 384000.0f              // Частота дискретизации, например, 48 кГц
#define START_FREQ 7000.0f                        // Начальная частота чирп-сигнала
#define END_FREQ 17000.0f                         // Конечная частота чирп-сигнала
#define TARGET_END_FREQ 17000.0f                  // Желаемая конечная частота чирп-сигнала
#define DURATION (FFT_SIZE / SAMPLING_FREQUENCY)  // Продолжительность сигнала

// Инициализация структуры для CFFT
arm_cfft_instance_f32 S;

// Массивы для входных и выходных данных
float input_data[FFT_SIZE];
std::complex<float> output_data[FFT_SIZE];
float power_spectrum[FFT_SIZE];  // Массив для хранения мощности спектра

// void copyFloatArrayToComplexVector(const float* floatArray, size_t size, std::vector<std::complex<float>>& complexVector) {
//     for (size_t i = 0; i < size; i += 2) {
//         // Assuming size is even and every pair of floats represents a complex number
//         std::complex<float> complexNumber(floatArray[i], floatArray[i + 1]);
//         complexVector.push_back(complexNumber);
//     }
// }

float calculate_end_frequency(float start_freq, float target_end_freq, float duration) {
    float total_phase = start_freq * duration + 0.5f * target_end_freq * duration;
    float N = round(total_phase);  // Находим ближайшее целое число
    return (N - start_freq * duration) * 2.0f / duration;
}

void init_chirp_signal(float end_freq) {
    float k = (end_freq - START_FREQ) / DURATION;
    float phase = 0.0f;

    for (int i = 0; i < FFT_SIZE; i++) {
        float t = (float)i / SAMPLING_FREQUENCY;
        phase = 2 * M_PI * (START_FREQ * t + 0.5f * k * t * t);
        input_data[i] = sinf(phase);
    }

    // // Нормализация сигнала для амплитуды от -1 до 1
    // float max_amplitude = 0.0f;
    // for (int i = 0; i < FFT_SIZE; i++) {
    //     if (fabs(input_data[i]) > max_amplitude) {
    //         max_amplitude = fabs(input_data[i]);
    //     }
    // }

    // for (int i = 0; i < FFT_SIZE; i++) {
    //     input_data[i] /= max_amplitude;
    // }
}

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    arm_status status;
    // float* input_data = new float[FFT_SIZE];
    const int signal_length = 1024;   // Длина сигнала в выборках
    float sampling_rate = 348000.0f;  // Частота дискретизации в Гц
    float start_freq = 7000.0f;       // Начальная частота в Гц
    float end_freq = 17000.0f;        // Конечная частота в Гц (может быть скорректирована функцией)

    // Расчет конечной частоты
    end_freq = calculate_end_frequency(START_FREQ, TARGET_END_FREQ, DURATION);
    printf("Adjusted end frequency: %f\n", end_freq);

    // Инициализация структуры для CFFT
    status = arm_cfft_init_f32(&S, FFT_SIZE);
    if (status != ARM_MATH_SUCCESS) {
        printf("CFFT initialization error!\n");
    }

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    printf("Starting ARM FFT\n\r");
    printf("Press the user button to start the FFT\n\r");

    // init_chirp_signal(input_data, FFT_SIZE, START_FREQ, end_freq, SAMPLING_FREQUENCY);
    // init_chirp_signal(input_data, signal_length, start_freq, end_freq, sampling_rate);
    init_chirp_signal(end_freq);

    // Преобразование real float в float32_t для CFFT
    float32_t* input_float32 = (float32_t*)pvPortMalloc(2 * FFT_SIZE * sizeof(float32_t));
    if (input_float32 == NULL) {
        printf("Memory allocation error!\n");
    }

    // float32_t input_float32[2 * FFT_SIZE];

    while (true) {
        vTaskSuspend(nullptr);

        for (int i = 0; i < FFT_SIZE; i++) {
            input_float32[2 * i] = input_data[i];  // Реальная часть
            input_float32[2 * i + 1] = 0.0f;       // Мнимая часть равна нулю
        }

        lastMicros = TimerMicros();
        // Выполнение прямого CFFT
        arm_cfft_f32(&S, input_float32, 0, 1);
        // // Выполнение обратного CFFT
        // // arm_cfft_f32(&S, input_float32, 1, 1);
        lastMicros = TimerMicros() - lastMicros;

        // Копирование результата в std::complex<float>
        for (int i = 0; i < FFT_SIZE; i++) {
            output_data[i] = std::complex<float>(input_float32[2 * i], input_float32[2 * i + 1]);
        }

        // Вычисление мощности спектра
        for (int i = 0; i < FFT_SIZE; i++) {
            float real = output_data[i].real();
            float imag = output_data[i].imag();
            power_spectrum[i] = real * real + imag * imag;
        }

        // float min_val, max_val;
        // arm_min_f32(power_spectrum, FFT_SIZE / 2, &min_val, NULL);
        // arm_max_f32(power_spectrum, FFT_SIZE / 2, &max_val, NULL);

        // for (int i = 0; i < FFT_SIZE / 2; ++i) {
        //     power_spectrum[i] = (power_spectrum[i] - min_val) / (max_val - min_val);
        // }

        // Поиск максимального значения в массиве мощности спектра
        float max_power = *std::max_element(power_spectrum, power_spectrum + FFT_SIZE);

        // Нормализация мощности спектра
        for (int i = 0; i < FFT_SIZE; i++) {
            power_spectrum[i] = power_spectrum[i] / max_power;
        }

        // for (int i = 0; i < FFT_SIZE / 2; ++i) {
        //     printf("%f\n\r", power_spectrum[i]);
        //     vTaskDelay(pdMS_TO_TICKS(8));
        // }

        for (int i = 0; i < FFT_SIZE / 2; ++i) {
            printf("%f\n\r", input_data[i]);
            vTaskDelay(pdMS_TO_TICKS(8));
        }

        printf("calculation time: %lu uS\n\r", static_cast<uint32_t>(lastMicros));
    }

    vPortFree(input_float32);
    delete[] input_data;
    ;
}
}  // namespace
}  // namespace coralmicro
