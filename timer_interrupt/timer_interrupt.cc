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
#include "libs/base/gpio.h"
#include "libs/base/led.h"
// #include "libs/base/mutex.h"
// #include "libs/base/tasks.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"

static uint64_t lastMicros_Led;

#define DAC_MAX 4095
#define DAC_OFF 2047.5
#define LED_TIME int(1 * 1000000)

namespace coralmicro {
namespace {

// void dac_main(void *param);

TimerHandle_t dac_timer;
static bool on = true;

// void dac_main(void *param) {
//   // DacWrite((int)DAC_OFF);
//   on = !on;
//   LedSet(Led::kUser, on);
// }

void dac_timer_callback(TimerHandle_t timer) {
  on = !on;
  LedSet(Led::kUser, on);
}

// void dac_timer_callback(TimerHandle_t timer) {
//   xTaskCreate(dac_main, "dac_main", configMINIMAL_STACK_SIZE * 10, nullptr,
//               kAppTaskPriority, nullptr);
// }

[[noreturn]] void Main() {
  TaskHandle_t dac_task;
  lastMicros_Led = TimerMicros();

  dac_timer = xTimerCreate("dac_timer", pdMS_TO_TICKS(1000), pdTRUE, nullptr,
                           dac_timer_callback);
  if (dac_timer == NULL) {
    printf("The timer was not created.");
  } else {
    /* Start the timer.  No block time is specified, and
    even if one was it would be ignored because the RTOS
    scheduler has not yet been started. */
    if (xTimerStart(dac_timer, 0) != pdPASS) {
      printf("The timer could not be set into the Active state.");
    }
  }

  // xTimerStart(dac_timer, 0);
  LedSet(Led::kStatus, on);
  DacInit();
  DacWrite(0);
  DacEnable(true);
  // GpioConfigureInterrupt(
  //     Gpio::kUserButton, GpioInterruptMode::kIntModeFalling,
  //     [handle = xTaskGetCurrentTaskHandle()]() { xTaskResumeFromISR(handle);
  //     },
  //     /*debounce_interval_us=*/50 * 1e3);

  // Turn on Status LED to show the board is on.

  // while (true) {
  //   vTaskSuspend(nullptr);
  // }
  // vTaskStartScheduler();
  // return 0;
while (true){
      if (TimerMicros() - lastMicros_Led >= LED_TIME) {
      lastMicros_Led = TimerMicros();
      printf("LED: %s\r\n", on ? "on" : "off");
    }

}
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
  (void)param;
  coralmicro::Main();
}
