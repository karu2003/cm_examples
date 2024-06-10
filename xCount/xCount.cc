
#include <cstdio>

#include "libs/base/gpio.h"
#include "libs/base/led.h"
#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"

namespace coralmicro {
namespace {

void vTaskFunction(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 1000;
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        TickType_t xCurrentTime = xTaskGetTickCount();
        printf("Current tick count: %lu\n", xCurrentTime);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

[[noreturn]] void Main() {
    xTaskCreate(vTaskFunction, "Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskSuspend(nullptr);
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void *param) {
    (void)param;
    coralmicro::Main();
    vTaskSuspend(nullptr);
}
