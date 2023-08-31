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

#include "libs/base/analog.h"
// #include "libs/audio/audio_service.h"

#include <cstdio>
#include "libs/base/timer.h"
#include "libs/base/led.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "math.h"

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
# define DAC_BIT (float)12.
# define DAC_MAX 4095
# define DAC_OFF 2047.5
# define SAMLERATE 50000.
# define SAMLE_COUNT int((1/SAMLERATE)*1000000)
# define LED_TIME int(1*1000000)
static float fSamp = SAMLERATE; 
static uint16_t duration = 1;
const double pi = 3.14159265358979;
const double pi2 = 6.28318530717958;

uint16_t genSampTbl (float freq, float fSamp, float amp, float offset, uint16_t **waveform ) {
    uint16_t nSamp = (uint16_t) (fSamp / freq);
    *waveform = new uint16_t[nSamp];
    for (int i = 0; i < nSamp; i++) {
        (*waveform)[i] =(uint16_t) (DAC_OFF + DAC_OFF *(amp * sin((2 * pi * i) / nSamp)));
    }
    return nSamp;
}
void dispTable (uint16_t *waveform, uint16_t nSamp, char *text ) {
    printf ("//%s: %s", __func__, text);    
    printf("int myTable[%d] = {", nSamp);
    for (int n = 0; n < nSamp; n++)
        printf ("%4d ",waveform [n]);    
    printf("};\n\r");
}

[[noreturn]] void Main() {
  uint16_t *waveform;
  uint16_t nSamp;
  uint16_t nStep = 0;
  float freq = 5000.0f;
  lastMicros = TimerMicros();
  lastMicros_Led = TimerMicros();  
  printf("DAC output Example!\r\n");

//   GpioConfigureInterrupt(
//       Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
//       [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle); },
//       /*debounce_interval_us=*/50 * 1e3);

  // Turn on Status LED to show the board is on.
  bool on = true;
  LedSet(Led::kStatus, on);
  DacInit();
  DacWrite(0);
  DacEnable(true);
  nSamp = genSampTbl (freq, fSamp, 1.0, 0, &waveform);

  while (true) {
    // vTaskSuspend(nullptr);
    // dispTable (waveform, nSamp, s); 
    if (TimerMicros() - lastMicros >= SAMLE_COUNT) {
        lastMicros = TimerMicros();
        DacWrite(waveform[nStep]);
        nStep = nStep +1;
        if (nStep >= nSamp) {
            nStep = 0;
        }
    }
    if (TimerMicros() - lastMicros_Led >= LED_TIME) {
        lastMicros_Led = TimerMicros();
        on = !on;
        LedSet(Led::kUser, on);
        printf("nSamp: %d\r\n", nSamp);
        printf("LED: %s\r\n", on ? "on" : "off");
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


