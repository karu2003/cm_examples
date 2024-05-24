#ifndef CHIRP_H
#define CHIRP_H

#include <math.h>
#include <cstdio>
#include <stdint.h> 
#define DAC_BIT (float)12.
#define DAC_MAX 4095
#define DAC_OFF 2047.5
const float pi = 3.14159265358979;
const float pi2 = 6.28318530717958;

// Function declarations
double log_freq_func(double w0, double w1, double indx);
double quad_freq_func(double w0, double w1, double indx);
float linear_freq_func(float w0, float w1, float indx);
void linspace(float a, float b, uint32_t c, float **linspaced);
uint32_t genSampTbl(float freq, float fSamp, float amp, float offset, volatile uint16_t **waveform);
uint32_t chirpGen(float fSamp, float duration, float start_freq, float end_freq, float amp, float phase, volatile uint16_t **chirpform);
uint32_t lchirp(float fSamp, float duration, float f0, float f1, float **phi);
void generateChirp(double *signal, int n, double sampleRate, double startFrequency, double endFrequency);


#endif // CHIRP_H