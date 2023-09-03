#ifndef DAC_OUT_H_
#define DAC_OUT_H_

#include <math.h>
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

inline void linspace(float a, float b, uint16_t c, float **linspaced) {
  *linspaced = new float[c];
  float delta = (b - a) / (c - 1);
  for (int i = 0; i < c; ++i) {
    (*linspaced)[i] = a + (i * delta);
  }
  return;
}

inline uint16_t genSampTbl(float freq, float fSamp, float amp, float offset,
                           uint16_t **waveform) {
  uint16_t nSamp = (uint16_t)(fSamp / freq);
  *waveform = new uint16_t[nSamp];
  for (int i = 0; i < nSamp; i++) {
    (*waveform)[i] =
        (uint16_t)(DAC_OFF + DAC_OFF * (amp * sin((2 * pi * i) / nSamp)));
  }
  return nSamp;
}

inline uint16_t chirpGen(float fSamp, float duration, float start_freq,
                         float end_freq, float amp, float phase,
                         uint16_t **chirpform) {
  float w0, w1;
  float current_phase, instantaneous_w;
  uint16_t nSamp, i;
  w0 = 2.0 * pi * start_freq / fSamp;
  w1 = 2.0 * pi * end_freq / fSamp;
  nSamp = lrint(duration * fSamp);
  *chirpform = new uint16_t[nSamp];
  current_phase = phase;
  instantaneous_w = w0;

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

// inline float _lchirp(uint16_t N, float tmin, float tmax, float fmin, float fmax,
//                      float **phi) {
//   float a, b;
//   linspace(tmin, tmax, N, &phi);

//   a = (fmin - fmax) / (tmin - tmax);
//   b = (fmin * tmax - fmax * tmin) / (tmax - tmin);

//   phi = (a / 2) * (pow(t, 2) - pow(tmin, 2)) + b * (t - tmin);
//   phi *= (2 * pi);
//   return
// }

// inline void lchirp(N, tmin = 0, tmax = 1, fmin = 0, fmax = None,
//                    zero_phase_tmin = True, cos = True)
//     : phi = _lchirp(N, tmin, tmax, fmin, fmax) if zero_phase_tmin
//     : phi
//       *= ((phi[-1] - phi[-1] % (2 * np.pi)) / phi[-1]) else
//     : phi
//       -= (phi[-1] % (2 * np.pi)) fn = np.cos if cos else np.sin return fn(phi)

#endif  // DAC_OUT_H_