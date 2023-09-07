#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kiss_fft.h"
#include "kiss_fftr.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

extern "C" [[noreturn]] void app_main(void* param) {
  // initialize input data for FFT
  float input[] = {11.0f, 3.0f, 4.05f, 9.0f, 10.3f, 8.0f, 4.934f, 5.11f};
  int nfft = sizeof(input) / sizeof(float);  // nfft = 8

  // allocate input/output 1D arrays
  kiss_fft_cpx* cin = new kiss_fft_cpx[nfft];
  kiss_fft_cpx* cout = new kiss_fft_cpx[nfft];

  // initialize data storage
  memset(cin, 0, nfft * sizeof(kiss_fft_cpx));
  memset(cout, 0, nfft * sizeof(kiss_fft_cpx));

  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);

  while (true) {
    vTaskSuspend(nullptr);
    // copy the input array to cin
    for (int i = 0; i < nfft; ++i) {
      cin[i].r = input[i];
      cin[i].i = 0;
    }
    // setup the size and type of FFT: forward
    bool is_inverse_fft = false;
    kiss_fft_cfg cfg_f = kiss_fft_alloc(nfft, is_inverse_fft, 0,
                                        0);  // typedef: struct kiss_fft_state*
    // execute transform for 1D
    for (int i = 0; i < nfft; ++i) {
      printf("#%d  %f %fj\n\r", i, cin[i].r, cin[i].i);
    }
    kiss_fft(cfg_f, cin, cout);

    // transformed: DC is stored in cout[0].r and cout[0].i
    printf("Forward Transform:\n\r");
    for (int i = 0; i < nfft; ++i) {
      printf("#%d, %f, %fj\n\r", i, cout[i].r, cout[i].i);
    }

    // setup the size and type of FFT: backward
    is_inverse_fft = true;
    kiss_fft_cfg cfg_i = kiss_fft_alloc(nfft, is_inverse_fft, 0, 0);

    // execute the inverse transform for 1D
    kiss_fft(cfg_i, cout, cin);

    // original input data
    printf("Inverse Transform:\n\r");
    for (int i = 0; i < nfft; ++i) {
      printf("#%d  %f\n\r", i,
             cin[i].r /
                 nfft);  // div by N to scale data back to the original range
    }
    // release resources
    kiss_fft_free(cfg_f);
    kiss_fft_free(cfg_i);
  }
  delete[] cin;
  delete[] cout;
}
}  // namespace
}  // namespace coralmicro
