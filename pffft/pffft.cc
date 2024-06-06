#include "pffft.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/base/gpio.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {

extern "C" [[noreturn]] void app_main(void* param) {
    uint64_t lastMicros;
    // initialize input data for FFT
    float input[] = {11.0f, 3.0f, 4.05f, 9.0f, 10.3f, 8.0f, 4.934f, 5.11f};
    uint16_t nfft = sizeof(input) / sizeof(float);

    PFFFT_Setup* setup = pffft_new_setup(N, PFFFT_REAL);

    float* cin = new float[nfft];
    float* output = new float[nfft];
    float* work = new float[nfft];

    // initialize data storage
    memset(cin, 0, nfft * sizeof(float));
    memset(output, 0, nfft * sizeof(float));
    memset(work, 0, nfft * sizeof(float));

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
        pffft_transform_ordered(setup, input, output, work, PFFFT_FORWARD);
        lastMicros = TimerMicros() - lastMicros;
    }
    printf("calculation time: %lu uS\n\r", static_cast<uint32_t>(lastMicros));
    pffft_destroy_setup(setup);
}
}  // namespace
}  // namespace coralmicro
