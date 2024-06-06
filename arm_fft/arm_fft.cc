#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm_math.h"
#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

arm_rfft_fast_instance_f32 fft_instance;
// arm_cfft_radix4_instance_f32 fft_instance;

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    // initialize input data for FFT
    float input[] = {11.0f, 3.0f, 4.05f, 9.0f, 10.3f, 8.0f, 4.934f, 5.11f};
    uint16_t nfft = sizeof(input) / sizeof(float);

    float32_t* cin = new float32_t[nfft];
    float32_t* cout = new float32_t[nfft];

    // initialize the FFT instance

    arm_rfft_fast_init_f32(&fft_instance, nfft);
    // initialize data storage
    memset(cin, 0, nfft * sizeof(float32_t));
    memset(cout, 0, nfft * sizeof(float32_t));

    GpioConfigureInterrupt(
        Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
        [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
        /*debounce_interval_us=*/50 * 1e3);

    while (true) {
        vTaskSuspend(nullptr);
        // copy the input array to cin
        for (int i = 0; i < nfft; ++i) {
            cin[i] = input[i];
        }
        lastMicros = TimerMicros();
        arm_rfft_fast_f32(&fft_instance, cin, cout, 0);
        lastMicros = TimerMicros() - lastMicros;
    }
    printf("calculation time: %lu uS\n\r", static_cast<uint32_t>(lastMicros));
    delete[] input;
    delete[] cin;
    delete[] cout;
}
}  // namespace
}  // namespace coralmicro
