// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// #include <math.h>
#include <cstdio>

#include "chirp.h"
#include "dac_timer.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/timer.h"
#include "math.h"
#include "splot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"

// writes it to the DAC (pin 9 on the right-side header ).
// Note: The DAC outputs a max of 1.8V.
//
// To build and flash from coralmicro root:
//    make -C out -j8
//    python3 coralmicro/scripts/flashtool.py --build_dir out --elf_path
//    out/dac_out/dac_out

namespace coralmicro {
namespace {
// [start-sphinx-snippet:dac_out]

static uint64_t lastMicros_Led;
// static uint64_t lastMicros;
#define LED_TIME int(1 * 1000000)
static float duration = 0.001f;
static float f0 = 7000.0f;
static float f1 = 17000.0f;
// float *chirpform_F;
bool on = true;
float SampleRate = 200000.0f;
}  // namespace

[[noreturn]] void Main() {
    lastMicros_Led = TimerMicros();
    printf("DAC output Example!\r\n");

    // GpioConfigureInterrupt(
    //     Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
    //     [handle = xTaskGetCurrentTaskHandle()]() {
    //     xTaskResumeFromISR(handle); },
    //     /*debounce_interval_us=*/50 * 1e3);

    LedSet(Led::kStatus, on);

    // nSamp = genSampTbl(f0, SAMLERATE, 1., 0, &chirpform);
    nSamp = chirpGen(SampleRate, duration, f0, f1, 1.0, 0., &chirpform);

    // nSamp = lchirp(SAMLERATE, duration, f0, f1, &chirpform_F);
    // chirpform = new uint16_t[nSamp];
    // for (int i = 0; i < nSamp; i++) {
    //   (chirpform)[i] = (uint16_t)(DAC_OFF + DAC_OFF * (chirpform_F)[i]);
    // }

    DacTimerInit(SampleRate);

    while (true) {
        // vTaskSuspend(nullptr);
        // serial_Plot_U(chirpform,nSamp);

        if (TimerMicros() - lastMicros_Led >= LED_TIME) {
            lastMicros_Led = TimerMicros();
            on = !on;
            LedSet(Led::kUser, on);
        }
    }
    // [end-sphinx-snippet:dac_out]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
    (void)param;
    coralmicro::Main();
}
