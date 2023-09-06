// https://forums.freertos.org/t/fast-fourier-transform-kissfft-issue/14699
// https://www.hackster.io/AlexWulff/adc-sampling-and-fft-on-raspberry-pi-pico-f883dd
// https://github.com/AlexFWulff/awulff-pico-playground/blob/main/adc_fft/adc_fft.c

#include <cstdio>

#include "libs/base/gpio.h"
#include "test_signal.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "kiss_fft.h"
#include "tools/kiss_fftr.h"

#define DAC_OFF 2047.5

namespace coralmicro {
namespace {

void do_fft(kiss_fft_cpx *in, kiss_fft_cpx *out, int direction) {
  kiss_fft_cfg cfg;
  // printf("fft\n\r");
  /* solution 1
   if (direction==0) kfc_fft(N,in,out);
     else            kfc_ifft(N,in,out);
   kfc_cleanup();
  */

  /* solution 2 */
  if ((cfg = kiss_fft_alloc(NSAMP, direction, NULL, NULL)) != NULL) {
    kiss_fft(cfg, in, out);
    kiss_fft_free(cfg);
  } else
    printf("FFT out of memory\n\r");
  printf(":%f\n\r", in[NSAMP].r);
}

extern "C" [[noreturn]] void app_main(void *param) {
  extern float pattern1[];
  float freqs[NSAMP];
  kiss_fft_cpx *fft_in;
  kiss_fft_cpx *fft_out;
  int i, k;
  fft_in = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
  fft_out = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
  // fft_in = new kiss_fft_cpx[NSAMP+1];
  // fft_out = new kiss_fft_cpx[NSAMP+1];

  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);

  for (k = 0; k < NSAMP; k++) {
    fft_in[k].r = DAC_OFF + DAC_OFF * pattern1[k];
    fft_in[k].i = 0;
  }

  while (true) {
    vTaskSuspend(nullptr);
    // compute power and calculate max freq component
    for(k = 0; k < NSAMP; k++) {
      printf("%u,%f\n\r",k, fft_in[k].r);
      vTaskDelay(pdMS_TO_TICKS(5));
    }
    do_fft(fft_in, fft_out, 0);
    float max_power = 0;
    int max_idx = 0;
    // any frequency bin over NSAMP/2 is aliased (nyquist sampling theorum)
    for (i = 0; i < NSAMP / 2; i++) {
      float power = fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i;
      // printf("%u,%f\n\r", i, power);
      if (power > max_power) {
        max_power = power;
        max_idx = i;
      }
    }

    float max_freq = freqs[max_idx];
    // printf("Greatest Frequency Component: %0.1f Hz\n",max_freq);
    // for(k = 0; k < NSAMP; k++) {
    //   printf("%u,%f\n\r",k, fft_out[k].r);
    //   vTaskDelay(pdMS_TO_TICKS(2));
    // }
  }
}

}  // namespace
}  // namespace coralmicro