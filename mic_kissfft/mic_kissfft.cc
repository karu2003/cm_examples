// https://forums.freertos.org/t/fast-fourier-transform-kissfft-issue/14699
// https://www.hackster.io/AlexWulff/adc-sampling-and-fft-on-raspberry-pi-pico-f883dd
// https://github.com/AlexFWulff/awulff-pico-playground/blob/main/adc_fft/adc_fft.c

#include <cstdio>

#include "libs/base/gpio.h"
#include <test_signal.h>
// #include "test_signal.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include <inttypes.h>
#include "hamming.h"
#include "kiss_fftr.h"
// #include "kiss_fft.h"
#include "libs/base/timer.h"

#define DAC_OFF 2047.5

namespace coralmicro {
namespace {

Hamming hamming_window(NSAMP);

extern "C" [[noreturn]] void app_main(void *param) {
#if FIXED_POINT == 32
  long maxrange = LONG_MAX;
#else
  long maxrange = SHRT_MAX; /* works fine for float too*/
#endif

  uint64_t lastMicros;
  extern float pattern1[];
  float freqs[NSAMP];
  int i, k;
  kiss_fft_cpx *fft_in = new kiss_fft_cpx[NSAMP];
  kiss_fft_cpx *fft_out = new kiss_fft_cpx[NSAMP];

  float heavyside_function[NSAMP];

  float f_max = FSAMP;
  float f_res = f_max / NSAMP;
  for (int i = 0; i < NSAMP; i++) {
    freqs[i] = f_res * i;
  }

  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);

  for (k = 0; k < NSAMP; k++) {
    heavyside_function[k] = pattern1[k];
  }

  hamming_window.applyWindow(heavyside_function);

  for (k = 0; k < NSAMP; k++) {
    fft_in[k].r = (maxrange >> 1) * heavyside_function[k];
    // fft_in[k].r = pattern1[k];
    fft_in[k].i = 0;
  }

  while (true) {
    vTaskSuspend(nullptr);
    bool is_inverse_fft = false;
    kiss_fft_cfg cfg_f = kiss_fft_alloc(NSAMP, is_inverse_fft, 0,
                                        0);  // typedef: struct kiss_fft_state*

    // compute power and calculate max freq component
    lastMicros = TimerMicros();
    kiss_fft(cfg_f, fft_in, fft_out);
    lastMicros = TimerMicros() - lastMicros;

    float max_power = 0;
    int max_idx = 0;

    // any frequency bin over NSAMP/2 is aliased (nyquist sampling theorum)
    for (i = 0; i < NSAMP / 2; i++) {
      float power = fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i;
      printf("%f,%f\n\r", freqs[i], power);
      vTaskDelay(pdMS_TO_TICKS(8));
      if (power > max_power) {
        max_power = power;
        max_idx = i;
      }
    }

    float max_freq = freqs[max_idx];
    // printf("Greatest Frequency Component: %0.1f Hz\n\r", max_freq);
    // printf("calculation time: %lu uS\n\r",
    // static_cast<uint32_t>(lastMicros));
  }
}
}  // namespace
}  // namespace coralmicro
