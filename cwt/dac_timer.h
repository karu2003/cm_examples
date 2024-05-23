#ifndef DAC_TIMER
#define DAC_TIMER

// #define SAMLERATE 200000.
namespace coralmicro {
extern volatile uint16_t *chirpform;
extern volatile uint32_t nStep;
extern volatile uint32_t Dir;
extern volatile uint32_t nSamp;

void DacTimerInit(float SampleRate);
void SetSampleRate(float SampleRate);
}  // namespace coralmicro
#endif
