// #include <../out/mic_kissfft/test_signal.h>
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
#define SAMPLING_FREQUENCY 96000.0f               // Частота дискретизации, например, 48 кГц
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

// float linear_freq_func(float w0, float w1, float indx) {
//     return w0 + (w1 - w0) * indx;
// }

// void generate_chirp_signal(float* signal, int sampleRate, float startFreq, float endFreq, int numSamples) {
//     float w0, w1;
//     float current_phase, instantaneous_w, phase;
//     phase = 0.0;
//     w0 = 2.0 * M_PI * startFreq / sampleRate;
//     w1 = 2.0 * M_PI * endFreq / sampleRate;
//     current_phase = phase;

//     for (int i = 0; i < numSamples; i++) {
//         input_data[i] = arm_sin_f32(current_phase);
//         instantaneous_w = linear_freq_func(w0, w1, (1.0 * i) / numSamples);
//         current_phase = fmod((current_phase + instantaneous_w), 2.0 * M_PI);
//     }
// }

void generate_chirp_signal(float* signal, float start_freq, float end_freq, float sample_rate, int num_samples) {
    float phase_increment = 2.0 * M_PI / num_samples;  // Приращение фазы на каждом шаге
    float phase = 0.0;                                 // Начальная фаза

    for (int i = 0; i < num_samples; ++i) {
        // Рассчитываем текущую частоту для линейного чирпа
        float current_freq = start_freq + (end_freq - start_freq) * i / num_samples;
        // Рассчитываем приращение фазы на основе текущей частоты
        float phase_increment_freq = 2.0 * M_PI * current_freq / sample_rate;
        // Увеличиваем фазу на фиксированное значение для линейного изменения фазы от 0 до 2π
        phase += phase_increment;
        // Нормализуем фазу, чтобы она оставалась в пределах 2*PI
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        // Рассчитываем значение сигнала с учетом текущей фазы
        signal[i] = arm_sin_f32(phase);
    }
}

// Функция для вычисления фазы линейного чирп-сигнала
void _lchirp(float32_t* phi, int N, float32_t fmin, float32_t fmax, float32_t fs) {
    float32_t tmin = 0;
    float32_t tmax = (N - 1) / fs;

    fmax = (fmax != 0) ? fmax : fs / 2.0f;

    float32_t t, a, b, delta_t;
    delta_t = 1.0f / fs;
    a = (fmin - fmax) / (tmin - tmax);
    b = (fmin * tmax - fmax * tmin) / (tmax - tmin);

    for (int i = 0; i < N; ++i) {
        t = i * delta_t;
        phi[i] = (a / 2.0f) * (t * t - tmin * tmin) + b * (t - tmin);
        phi[i] *= 2 * PI;
    }
}

// Основная функция для генерации линейного чирп-сигнала
void lchirp(float32_t* signal, int N, float32_t fmin, float32_t fmax, float32_t fs, bool zero_phase_tmin, bool use_cos) {
    float32_t phi[N];
    _lchirp(phi, N, fmin, fmax, fs);

    float32_t phi_last = phi[N - 1];
    float32_t phi_factor = (phi_last - fmod(phi_last, 2 * PI)) / phi_last;

    for (int i = 0; i < N; ++i) {
        if (zero_phase_tmin) {
            phi[i] *= phi_factor;
        } else {
            phi[i] -= fmod(phi_last, 2 * PI);
        }

        if (use_cos) {
            signal[i] = arm_cos_f32(phi[i]);
        } else {
            signal[i] = arm_sin_f32(phi[i]);
        }
    }
}

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    arm_status status;
    // float* input_data = new float[FFT_SIZE];
    const int signal_length = 1024;  // Длина сигнала в выборках
    float sampling_rate = 384000.0f;  // Частота дискретизации в Гц
    float start_freq = 7000.0f;      // Начальная частота в Гц
    float end_freq = 17000.0f;        // Конечная частота в Гц (может быть скорректирована функцией)

    // Расчет конечной частоты
    // end_freq = calculate_end_frequency(START_FREQ, TARGET_END_FREQ, DURATION);
    // printf("Adjusted end frequency: %f\n", end_freq);

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
    // init_chirp_signal(end_freq);
    // generate_chirp_signal(input_data, sampling_rate, start_freq, end_freq, signal_length);
    lchirp(input_data, signal_length, start_freq, end_freq, sampling_rate, false, false);

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
