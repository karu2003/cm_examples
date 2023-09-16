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

#include <cstdio>

#include "generator_message.h"
#include "libs/base/ipc_m4.h"
#include "libs/base/led.h"
#include "libs/base/main_freertos_m4.h"
#include "libs/base/mutex.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"

namespace coralmicro {
namespace {

void HandleM7Message(const uint8_t data[kIpcMessageBufferDataSize]) {
  const auto* app_msg = reinterpret_cast<const GeneratorAppMessage*>(data);
  if (app_msg->type == GeneratorMessageType::kSetStatus) {
    printf("[M4] Samlerate %f\r\n", app_msg->Settings.Samlerate);
    printf("[M4] Duration %f\r\n", app_msg->Settings.Duration);
    printf("[M4] F0 %f\r\n", app_msg->Settings.F0);
    printf("[M4] F1 %f\r\n", app_msg->Settings.F1);
    printf("[M4] TypeF %x\r\n", app_msg->Settings.TypeF);
    printf("[M4] AutoRestart %d\r\n", app_msg->Settings.AutoRestart);
    printf("[M4] RunBack %d\r\n", app_msg->Settings.RunBack);
    printf("[M4] Start_DAC %d\r\n", app_msg->Settings.StartDAC);
  }
}

bool volatile g_switch_to_m7_signal = false;

}  // namespace

[[noreturn]] void Main() {
  printf("[M4] Started, DAC out...\r\n");
  IpcM4::GetSingleton()->RegisterAppMessageHandler(HandleM7Message);
  LedSet(Led::kStatus, true);

  printf("[M4] stopped\r\n");
  vTaskSuspend(nullptr);
  while (true) {
  }
}
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
