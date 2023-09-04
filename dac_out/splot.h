
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

inline void serial_Plot_U(uint16_t *waveform, uint16_t nSamp) {
  int n;
  for (n = 0; n < nSamp; n++) {
    printf("%u,%u\n\r", n, waveform[n]);
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

inline void serial_Plot_F(float *waveform, uint16_t nSamp) {
  int n;
  for (n = 0; n < nSamp; n++) {
    printf("%u,%6f\n\r", n, waveform[n]);
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}
