
// spot.h
#ifndef SPOT_H
#define SPOT_H

#include <cstdint>
#include <string>

#include "libs/base/timer.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

inline void dispTable(uint16_t *waveform, uint16_t nSamp, char *text) {
    printf("//%s: %s", __func__, text);
    printf("int myTable[%d] = {", nSamp);
    for (int n = 0; n < nSamp; n++) printf("%4d ", waveform[n]);
    printf("};\n\r");
}

inline void dispTable_F(float *waveform, uint16_t nSamp, char *text) {
    printf("//%s: %s", __func__, text);
    printf("float myTable[%d] = {", nSamp);
    for (int n = 0; n < nSamp; n++) {
        printf("%6f ", waveform[n]);
    }
    printf("};\r\n");
}

#include <type_traits>

template <typename T>
inline void serial_Plot(T *waveform, uint32_t nSamp) {
    uint32_t n;
    for (n = 0; n < nSamp; n++) {
        if constexpr (std::is_same<T, uint16_t>::value) {
            printf("%u,%u\n\r", n, waveform[n]);
        } else if constexpr (std::is_same<T, float>::value) {
            printf("%u,%6f\n\r", n, waveform[n]);
        } else if constexpr (std::is_same<T, double>::value) {
            printf("%u,%10f\n\r", n, waveform[n]);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// inline void serial_Plot_U(uint16_t *waveform, uint16_t nSamp) {
//     int n;
//     for (n = 0; n < nSamp; n++) {
//         printf("%u,%u\n\r", n, waveform[n]);
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

// inline void serial_Plot_F(float *waveform, uint32_t nSamp) {
//     uint32_t n;
//     for (n = 0; n < nSamp; n++) {
//         printf("%u,%6f\n\r", n, waveform[n]);
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

// inline void serial_Plot_D(double *waveform, uint32_t nSamp) {
//     uint32_t n;
//     for (n = 0; n < nSamp; n++) {
//         printf("%u,%10f\n\r", n, waveform[n]);
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

// inline void serial_Plot_Proc(double *waveform, uint32_t nSamp) {
//     uint32_t n;
//     for (n = 0; n < nSamp; n++) {
//         printf("%10f\n\r", waveform[n]);
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

inline void serial_Plot_ProcU(uint16_t *waveform, uint16_t nSamp) {
    int n;
    for (n = 0; n < nSamp; n++) {
        printf("%u\n\r", waveform[n]);
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

template <typename T>
inline void serial_Plot_Proc(T *waveform, uint32_t nSamp) {
    uint32_t n;
    for (n = 0; n < nSamp; n++) {
        if constexpr (std::is_floating_point<T>::value) {
            printf("%10f\n\r", waveform[n]);
        } else if constexpr (std::is_integral<T>::value) {
            printf("%u\n\r", waveform[n]);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

inline void calculateAndPrint(uint16_t *signal1, uint16_t *signal2, uint32_t nSamp, const std::string &message) {
    uint64_t lastMicros = coralmicro::TimerMicros();
    float coeff = find_coefficient(signal1, signal2, nSamp);
    uint64_t duration_corr = coralmicro::TimerMicros() - lastMicros;
    printf("%s calculation time : %lu uS\n\r", message.c_str(), static_cast<uint32_t>(duration_corr));
    printf("%s Corr. coeff: %20f\r\n", message.c_str(), coeff);
}

inline void print_autocorrelation_to_serial(double *autocorrelation, int n) {
    for (int i = 0; i < n; i++) {
        printf("autocorrelation[%d] = %f\n", i, autocorrelation[i]);
    }
}

// inline void serial_Plot_D(double *data, uint32_t size) {
//     for (uint32_t i = 0; i < size; i++) {
//         char value_as_char = static_cast<int>(data[i]);
//         printf("%u,%c\n\r", i, value_as_char);
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

#endif  // SPOT_H
