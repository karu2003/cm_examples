//
//  fcwt.h
//  fCWT
//
//  Created by Lukas Arts on 21/12/2020.
//  Copyright © 2021 Lukas Arts.
/*Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef FCWT_H
#define FCWT_H
#define FCWT_LIBRARY_API

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <complex>
#include "arm_math.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#define PI 3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f
#define sqrt2PI 2.50662827463100050241576528f
#define IPI4 0.75112554446f

using namespace std;

enum SCALETYPE { FCWT_LINSCALES,
                 FCWT_LOGSCALES,
                 FCWT_LINFREQS };

class Wavelet {
   public:
    Wavelet(){};
    virtual void generate(float *real, float *imag, int size, float scale) { printf("ERROR [generate time complex]: Override this virtual class"); };
    virtual void generate(int size) { printf("ERROR [generate freq]: Override this virtual class"); };
    virtual int getSupport(float scale) {
        printf("ERROR [getsupport]: Override this virtual class");
        return 0;
    };
    virtual void getWavelet(float scale, complex<float> *pwav, int pn) { printf("ERROR [getsupport]: Override this virtual class"); };

    int width;
    float four_wavelen;
    bool imag_frequency, doublesided;
    float *mother;
};

class Morlet : public Wavelet {
   public:
    FCWT_LIBRARY_API Morlet(float bandwidth);  // frequency domain
    ~Morlet() { free(mother); };

    void generate(int size);                                         // frequency domain
    void generate(float *real, float *imag, int size, float scale);  // time domain
    int getSupport(float scale) { return (int)(fb * scale * 3.0f); };
    void getWavelet(float scale, complex<float> *pwav, int pn);
    float fb;

   private:
    float ifb, fb2;
};

class Scales {
   public:
    FCWT_LIBRARY_API Scales(Wavelet *pwav, SCALETYPE st, int fs, float f0, float f1, int fn);

    void FCWT_LIBRARY_API getScales(float *pfreqs, int pnf);
    void FCWT_LIBRARY_API getFrequencies(float *pfreqs, int pnf);

    float *scales;
    int fs;
    float fourwavl;
    int nscales;

   private:
    void calculate_logscale_array(float base, float four_wavl, int fs, float f0, float f1, int fn);
    void calculate_linscale_array(float four_wavl, int fs, float f0, float f1, int fn);
    void calculate_linfreq_array(float four_wavl, int fs, float f0, float f1, int fn);
};

class FCWT {
   public:
    FCWT_LIBRARY_API FCWT(Wavelet *pwav, bool puse_normalization) : wavelet(pwav), use_normalization(puse_normalization) {};
    void FCWT_LIBRARY_API cwt(float32_t *pinput, int psize, complex<float> *poutput, Scales *scales);

    Wavelet *wavelet;

   private:
    void fftbased(arm_cfft_instance_f32 &cfft_instance, float32_t *Ihat, float32_t *O1, float32_t *out, const float32_t *mother, int size, float scale, bool imaginary, bool doublesided);
    void fft_normalize(complex<float> *out, int size);
    void daughter_wavelet_multiplication(float *input, float *output, const float *mother, float scale, int isize, bool imaginary, bool doublesided);

    int size;
    float fs, f0, f1, fn;
    bool use_normalization;
    arm_status status;
};

inline int find2power(int n) {
    int m, m2;
    m = 0;
    m2 = 1 << m; /* 2 to the power of m */
    while (m2 - n < 0) {
        m++;
        m2 <<= 1; /* m2 = m2*2 */
    }
    return (m);
}

#endif