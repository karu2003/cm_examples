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

#include <math.h>

#include <cstdio>

#include "chirp.h"
#include "libs/base/analog.h"
#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/timer.h"
#include "math.h"
#include "splot.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// writes it to the DAC (pin 9 on the right-side header).
// Note: The DAC outputs a max of 1.8V.
//
// To build and flash from coralmicro root:
//    bash build.sh
//    python3 scripts/flashtool.py -e analog

namespace coralmicro {
namespace {
// [start-sphinx-snippet:dac_out]

static uint64_t lastMicros;
static uint64_t lastMicros_Led;
static uint16_t val = 0;
#define DAC_BIT (float)12.
#define DAC_MAX 4095
#define DAC_OFF 2047.5
#define SAMLERATE 200000.
#define SAMLE_COUNT int((1 / SAMLERATE) * 1000000)
#define LED_TIME int(1 * 1000000)
static float fSamp = SAMLERATE;
static float duration = 1.0;
static float f0 = 7000.0;
static float f1 = 17000.0;

[[noreturn]] void Main() {
  uint16_t *waveform;
  uint16_t *chirpform;
  float *chirpform_F;
  uint16_t *chirpform_L;
  float *nLinspace;
  uint16_t nSamp;
  uint16_t nStep = 0;
  float freq = 500.0f;
  lastMicros = TimerMicros();
  lastMicros_Led = TimerMicros();
  printf("DAC output Example!\r\n");
  char k[40];

  GpioConfigureInterrupt(
      Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
      [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
      /*debounce_interval_us=*/50 * 1e3);

  // Turn on Status LED to show the board is on.
  bool on = true;
  LedSet(Led::kStatus, on);
  DacInit();
  DacWrite(0);
  DacEnable(true);
  // nSamp = genSampTbl (freq, fSamp, 1.0, 0, &waveform);

  nSamp = chirpGen(fSamp, duration, f0, f1, 0.5, 0., &chirpform);

  // nSamp = lchirp(fSamp, duration, f0, f1, &chirpform_F);
  // chirpform = new uint16_t[nSamp];
  // for (int i = 0; i < nSamp; i++) {
  //   (chirpform)[i] = (uint16_t)(DAC_OFF + DAC_OFF *(chirpform_F)[i]);
  // }
  int Dir = 1;
  nStep = 0;
  while (true) {
    // vTaskSuspend(nullptr);
    // dispTable_F(chirpform,nSamp,k);
    // serial_Plot_F(nLinspace, nSamp);
    // serial_Plot_U(chirpform,nSamp);
    // serial_Plot_F(chirpform_F,nSamp);
    // dispTable (waveform, nSamp, s);
    if (TimerMicros() - lastMicros >= SAMLE_COUNT) {
      lastMicros = TimerMicros();
      DacWrite(chirpform[nStep]);
      // DacWrite(waveform[nStep]);
      nStep = nStep + Dir;
      if (nStep == nSamp - 1 or nStep == 0) {
        Dir = Dir * -1;
      }
    }
    if (TimerMicros() - lastMicros_Led >= LED_TIME) {
      lastMicros_Led = TimerMicros();
      on = !on;
      LedSet(Led::kUser, on);
      // printf("nSamp: %d\r\n", nSamp);
      // printf("LED: %s\r\n", on ? "on" : "off");

      // printf("nStep: %d\r\n", test[nStep]);
      // nStep = nStep + Dir;
      // if (nStep == NTEST - 1 or nStep == 0) {
      //   Dir = Dir * -1;
      // }
    }
  }
}
// [end-sphinx-snippet:dac_out]
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
  (void)param;
  coralmicro::Main();
}
