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
// #include "libs/base/mutex.h"
#include "libs/base/tasks.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
// add dac_timer.h and chirp.h to the includes
#include "../dac_out/dac_timer.h"
#include "../dac_out/chirp.h"


namespace coralmicro {
namespace {
GeneratorSettings g_DAC_Settings;
uint64_t lastMicros_Led;
#define LED_TIME int(1 * 1000000)
bool volatile on = true;
bool volatile g_switch_to_m7_signal = false;

static float duration = 1.0f;
static float f0 = 7000.0f;
static float f1 = 17000.0f;
// u_int32_t nSamp = 0;


void Print_Message() {
    printf("[M4] Samlerate %f\r\n", g_DAC_Settings.Samlerate);
    printf("[M4] Duration %f\r\n", g_DAC_Settings.Duration);
    printf("[M4] F0 %f\r\n", g_DAC_Settings.F0);
    printf("[M4] F1 %f\r\n", g_DAC_Settings.F1);
    printf("[M4] Amplitude %f\r\n", g_DAC_Settings.amp);
    printf("[M4] Phase %f\r\n", g_DAC_Settings.phi);
    printf("[M4] TypeF %lu\r\n", (uint32_t)g_DAC_Settings.TypeF);
    printf("[M4] AutoRestart %d\r\n", g_DAC_Settings.AutoRestart);
    printf("[M4] RunBack %d\r\n", g_DAC_Settings.RunBack);
    printf("[M4] Start_DAC %d\r\n", g_DAC_Settings.StartDAC);
}

void HandleM7_Message(const uint8_t data[kIpcMessageBufferDataSize]) {
    const auto* app_msg = reinterpret_cast<const GeneratorAppMessage*>(data);
    if (app_msg->type == GeneratorMessageType::kSetStatus) {
        g_DAC_Settings = app_msg->Settings;
        Print_Message();

        IpcMessage ack_msg{};
        ack_msg.type = IpcMessageType::kApp;
        auto* app_msg =
            reinterpret_cast<GeneratorAppMessage*>(&ack_msg.message.data);
        app_msg->type = GeneratorMessageType::kAck;
        IpcM4::GetSingleton()->SendMessage(ack_msg);
    }
}

void generator_task(void* param) {
    lastMicros_Led = TimerMicros();
    printf("[M4] Started, Signal Genarator \r\n");
    LedSet(Led::kStatus, true);
    // nSamp = genSampTbl(f0, SAMLERATE, 1., 0, &chirpform);

    // coralmicro::nSamp = chirpGen(SAMLERATE, duration, f0, f1, 1.0, 0., &chirpform);
    // coralmicro::DacTimerInit();

    while (true) {
        if (TimerMicros() - lastMicros_Led >= LED_TIME) {
            lastMicros_Led = TimerMicros();
            on = !on;
            LedSet(Led::kUser, on);
        }
    }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
    (void)param;
    TaskHandle_t gen_task;
    coralmicro::IpcM4::GetSingleton()->RegisterAppMessageHandler(
        coralmicro::HandleM7_Message);
    // xTaskCreate([](void* param) {while (true) { }}, "wake task",
    // configMINIMAL_STACK_SIZE, nullptr, 1U, nullptr);
    xTaskCreate(coralmicro::generator_task, "generator_task",
                configMINIMAL_STACK_SIZE * 10, nullptr, 1U, &gen_task);
    vTaskSuspend(nullptr);
}
