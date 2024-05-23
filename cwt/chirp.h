#ifndef DAC_OUT_H_
#define DAC_OUT_H_

#include <math.h>
#define DAC_BIT (float)12.
#define DAC_MAX 4095
#define DAC_OFF 2047.5
const float pi = 3.14159265358979;
const float pi2 = 6.28318530717958;

inline static double log_freq_func(double w0, double w1, double indx) {
  return pow(10.0, log10(w0) + (log10(w1) - log10(w0)) * indx);
} /* log_freq_func */

inline static double quad_freq_func(double w0, double w1, double indx) {
  return w0 + (w1 - w0) * indx * indx;
} /* log_freq_func */

inline static float linear_freq_func(float w0, float w1, float indx) {
  return w0 + (w1 - w0) * indx;
} /* linear_freq_func */

inline void linspace(float a, float b, uint32_t c, float **linspaced) {
  uint32_t i;
  *linspaced = new float[c];
  float delta = (b - a) / (c - 1);
  for (i = 0; i < c; ++i) {
    (*linspaced)[i] = a + (i * delta);
  }
  return;
}

inline uint32_t genSampTbl(float freq, float fSamp, float amp, float offset,
                           volatile uint16_t **waveform) {
  uint32_t nSamp, i;
  nSamp = lrint(fSamp / freq);
  *waveform = new uint16_t[nSamp];
  for (i = 0; i < nSamp; i++) {
    (*waveform)[i] =
        (uint16_t)(DAC_OFF + DAC_OFF * (amp * sin((2 * pi * i) / nSamp)));
  }
  return nSamp;
}

inline uint32_t chirpGen(float fSamp, float duration, float start_freq,
                         float end_freq, float amp, float phase,
                         volatile uint16_t **chirpform) {
  float w0, w1;
  float current_phase, instantaneous_w;
  uint32_t nSamp, i;
  w0 = 2.0 * pi * start_freq / fSamp;
  w1 = 2.0 * pi * end_freq / fSamp;
  nSamp = lrint(duration * fSamp);
  *chirpform = new uint16_t[nSamp];
  current_phase = phase;
  // instantaneous_w = w0;

  for (i = 0; i < nSamp; i++) {
    (*chirpform)[i] =
        (uint16_t)(DAC_OFF + DAC_OFF * (amp * sin(current_phase)));
    // (*chirpform)[i] = amp * sin(current_phase);
    instantaneous_w = linear_freq_func(w0, w1, (1.0 * i) / nSamp);
    current_phase = fmod((current_phase + instantaneous_w), 2.0 * pi);
  }
  //   phi *= ( (phi[-1] - phi[-1] % (2*np.pi)) / phi[-1] )
  return nSamp;
}

inline uint32_t lchirp(float fSamp, float duration, float f0, float f1,
                       float **phi) {
  uint32_t nSamp, i;
  float beta;
  nSamp = lrint(duration * fSamp);
  linspace(0, duration, nSamp, phi);
  beta = (f1 - f0) / duration;
  for (i = 0; i < nSamp; i++) {
    (*phi)[i] =
        2.0 * pi * (f0 * (*phi)[i] + 0.5 * beta * (*phi)[i] * (*phi)[i]);
    (*phi)[i] = sin((*phi)[i]);
  }
  return nSamp;
}

double* GenChirpSignal(int n, double f0, double f1, double sampleRate)
{
    // Generate the chirp signal
    double T = n / sampleRate;
    double* chirp = new double[n];
    for (int i = 0; i < n; ++i) {
        double t = i * T / n;
        chirp[i] = sin(2 * M_PI * (f0 * t + (f1 - f0) * t * t / (2 * T)));
    }
    return chirp;
}

#endif  // DAC_OUT_H_