//
//  fcwt.cpp
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

//
//  fcwt.cpp
//  fCWT-testing
//
//  Created by Lukas Arts on 21/12/2020.
//  Copyright © 2020 Lukas Arts.
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

#include "fcwt.h"

Morlet::Morlet(float bandwidth) {
    four_wavelen = 0.9876f;
    fb = bandwidth;
    fb2 = 2.0f * fb * fb;
    ifb = 1.0f / fb;
    imag_frequency = false;
    doublesided = false;
    mother = NULL;
}

void Morlet::generate(int size) {
    // Frequency domain, because we only need size. Default scale is always 2;
    width = size;

    float tmp1;
    float toradians = (2 * PI) / (float)size;
    float norm = sqrt(2 * PI) * IPI4;

    mother = (float *)malloc(sizeof(float) * width);

    // calculate array
    for (int w = 0; w < width; w++) {
        tmp1 = (2.0f * ((float)w * toradians) * fb - 2.0f * PI * fb);
        tmp1 = -(tmp1 * tmp1) / 2;
        mother[w] = (norm * exp(tmp1));
    }
}
void Morlet::generate(float *real, float *imag, int size, float scale) {
    // Time domain because we know size from scale
    float tmp1, tmp2;
    width = getSupport(scale);
    float norm = (float)size * ifb * IPI4;

    // cout << scale << " [";
    for (int t = 0; t < width * 2 + 1; t++) {
        tmp1 = (float)(t - width) / scale;
        tmp2 = exp(-(tmp1 * tmp1) / (fb2));

        real[t] = norm * tmp2 * cos(tmp1 * 2.0f * PI) / scale;
        imag[t] = norm * tmp2 * sin(tmp1 * 2.0f * PI) / scale;
        // cout << real[t]*real[t]+imag[t]*imag[t] << ",";
    }
    // cout << "]" << endl;
}

void Morlet::getWavelet(float scale, complex<float> *pwav, int pn) {
    int w = getSupport(scale);

    float *real = (float *)malloc(sizeof(float) * max(w * 2 + 1, pn));
    float *imag = (float *)malloc(sizeof(float) * max(w * 2 + 1, pn));
    for (int t = 0; t < max(w * 2 + 1, pn); t++) {
        real[t] = 0;
        imag[t] = 0;
    }

    generate(real, imag, pn, scale);

    for (int t = 0; t < pn; t++) {
        pwav[t].real(real[t]);
        pwav[t].imag(imag[t]);
    }

    delete real;
    delete imag;
};

//==============================================================//
//================== Scales =====================================//
//==============================================================//

Scales::Scales(Wavelet *wav, SCALETYPE st, int afs, float af0, float af1, int afn) {
    fs = afs;
    scales = (float *)malloc(afn * sizeof(float));
    fourwavl = wav->four_wavelen;
    nscales = afn;

    if (st == SCALETYPE::FCWT_LOGSCALES)
        calculate_logscale_array(2.0f, wav->four_wavelen, afs, af0, af1, afn);
    else if (st == SCALETYPE::FCWT_LINSCALES)
        calculate_linscale_array(wav->four_wavelen, afs, af0, af1, afn);
    else
        calculate_linfreq_array(wav->four_wavelen, afs, af0, af1, afn);
}

void Scales::getScales(float *pfreqs, int pnf) {
    for (int i = 0; i < pnf; i++) {
        pfreqs[i] = scales[i];
    };
};

void Scales::getFrequencies(float *pfreqs, int pnf) {
    for (int i = 0; i < pnf; i++) {
        pfreqs[i] = ((float)fs) / scales[i];
    };
};

void Scales::calculate_logscale_array(float base, float four_wavl, int fs, float f0, float f1, int fn) {
    // If a signal has fs=100hz and you want to measure [0.1-50]Hz, you need scales 2 to 1000;
    float nf0 = f0;
    float nf1 = f1;
    float s0 = (fs / nf1);
    float s1 = (fs / nf0);

    // Cannot pass the nyquist frequency
    assert(("Max frequency cannot be higher than the Nyquist frequency (fs/2)", f1 <= fs / 2));

    float power0 = log(s0) / log(base);
    float power1 = log(s1) / log(base);
    float dpower = power1 - power0;

    for (int i = 0; i < fn; i++) {
        float power = power0 + (dpower / (fn - 1)) * i;
        scales[i] = pow(base, power);
    }
}

void Scales::calculate_linfreq_array(float four_wavl, int fs, float f0, float f1, int fn) {
    float nf0 = f0;
    float nf1 = f1;
    // If a signal has fs=100hz and you want to measure [0.1-50]Hz, you need scales 2 to 1000;

    // Cannot pass the nyquist frequency
    assert(("Max frequency cannot be higher than the Nyquist frequency (fs/2)", f1 <= fs / 2));
    float df = nf1 - nf0;

    for (int i = 0; i < fn; i++) {
        scales[fn - i - 1] = (((float)fs) / (nf0 + (df / fn) * (float)i));
    }
}

void Scales::calculate_linscale_array(float four_wavl, int fs, float f0, float f1, int fn) {
    float nf0 = f0;
    float nf1 = f1;
    // If a signal has fs=100hz and you want to measure [0.1-50]Hz, you need scales 2 to 1000;
    float s0 = fs / nf1;
    float s1 = fs / nf0;

    // Cannot pass the nyquist frequency
    assert(("Max frequency cannot be higher than the Nyquist frequency (fs/2)", f1 <= fs / 2));
    float ds = s1 - s0;

    for (int i = 0; i < fn; i++) {
        scales[i] = (s0 + (ds / fn) * i);
    }
}


//==============================================================//
//================== FCWT =====================================//
//==============================================================//

void FCWT::daughter_wavelet_multiplication(float32_t *input, float32_t *output, const float32_t *mother, float scale, int isize, bool imaginary, bool doublesided) {
    float isizef = static_cast<float>(isize);
    float endpointf = fmin(isizef / 2.0, (isizef * 2.0) / scale);
    float step = scale / 2.0;
    int endpoint = static_cast<int>(endpointf);
    float maximum = isizef - 1;
    int s1 = isize - 1;

    for (int q1 = 0; q1 < endpoint; q1++) {
        float q = static_cast<float>(q1);
        float tmp = fmin(maximum, step * q);

        output[2 * q1] = input[2 * q1] * mother[static_cast<int>(tmp)];
        output[2 * q1 + 1] = input[2 * q1 + 1] * mother[static_cast<int>(tmp)] * (1 - 2 * imaginary);
    }

    if (doublesided) {
        for (int q1 = 0; q1 < endpoint; q1++) {
            float q = static_cast<float>(q1);
            float tmp = fmin(maximum, step * q);

            output[2 * (s1 - q1)] = input[2 * (s1 - q1)] * mother[static_cast<int>(tmp)] * (1 - 2 * imaginary);
            output[2 * (s1 - q1) + 1] = input[2 * (s1 - q1) + 1] * mother[static_cast<int>(tmp)];
        }
    }
}

void FCWT::fftbased(arm_cfft_instance_f32 &cfft_instance, float32_t *Ihat, float32_t *O1, float32_t *out, const float32_t *mother, int size, float scale, bool imaginary, bool doublesided) {
    // Генерация дочерней вейвлет-функции и умножение с преобразованным входным сигналом
    daughter_wavelet_multiplication(Ihat, O1, mother, scale, size, imaginary, doublesided);

    // Выполнение обратного FFT с использованием ARM CMSIS-DSP
    arm_cfft_f32(&cfft_instance, O1, 1, 1);

    // Копирование результата в выходной буфер
    memcpy(out, O1, 2 * size * sizeof(float32_t));
}

void FCWT::cwt(float32_t *pinput, int psize, complex<float> *poutput, Scales *scales, bool complexinput) {
    arm_cfft_instance_f32 cfft_instance;
    size = psize;

    // Найти ближайшую степень двойки
    const int nt = find2power(size);
    const int newsize = 1 << nt;

    // Инициализация промежуточного результата
    float32_t *Ihat = (float32_t *)pvPortMalloc(2 * newsize * sizeof(float32_t));
    float32_t *O1 = (float32_t *)pvPortMalloc(2 * newsize * sizeof(float32_t));
    memset(Ihat, 0, 2 * newsize * sizeof(float32_t));
    memset(O1, 0, 2 * newsize * sizeof(float32_t));

    // Инициализация FFT планов
    arm_cfft_init_f32(&cfft_instance, newsize);

    // Выполнение прямого FFT на входном сигнале
    float32_t *input;
    if (complexinput) {
        input = (float32_t *)pvPortMalloc(2 * newsize * sizeof(complex<float>));
        memcpy(input, pinput, sizeof(complex<float>) * size);
        arm_cfft_f32(&cfft_instance, input, 0, 1);
        memcpy(Ihat, input, 2 * newsize * sizeof(float32_t));
    } else {
        input = (float32_t *)pvPortMalloc(newsize * sizeof(float32_t));
        memset(input, 0, newsize * sizeof(float32_t));
        memcpy(input, pinput, sizeof(float32_t) * size);
        arm_rfft_fast_instance_f32 rfft_instance;
        arm_rfft_fast_init_f32(&rfft_instance, newsize);
        arm_rfft_fast_f32(&rfft_instance, input, Ihat, 0);
    }
    vPortFree(input);

    // Генерация материнской вейвлет-функции
    wavelet->generate(newsize);

    // Конвертирование вектора ИХАТ
    for (int i = 1; i < (newsize >> 1); i++) {
        Ihat[2 * (newsize - i)] = Ihat[2 * i];
        Ihat[2 * (newsize - i) + 1] = -Ihat[2 * i + 1];
    }

    complex<float> *out = poutput;

    for (int i = 0; i < scales->nscales; i++) {
        // Свертка на основе FFT в частотной области
        fftbased(cfft_instance, Ihat, O1, (float32_t *)out, wavelet->mother, newsize, scales->scales[i], wavelet->imag_frequency, wavelet->doublesided);
        if (use_normalization) fft_normalize(out, newsize);
        out += size;
    }

    // Очистка
    vPortFree(Ihat);
    vPortFree(O1);
}

void FCWT::fft_normalize(complex<float> *out, int size) {
    for (int i = 0; i < size; i++) {
        out[i] = out[i] / static_cast<float>(size);
    }
}




void FCWT::cwt(float *pinput, int psize, complex<float> *poutput, Scales *scales) {
    cwt(pinput, psize, poutput, scales, false);
}

void FCWT::cwt(complex<float> *pinput, int psize, complex<float> *poutput, Scales *scales) {
    cwt((float *)pinput, psize, poutput, scales, true);
}

void FCWT::cwt(float *pinput, int psize, Scales *scales, complex<float> *poutput, int pn1, int pn2) {
    assert((psize * scales->nscales) == (pn1 * pn2));
    cwt(pinput, psize, poutput, scales);
}

void FCWT::cwt(complex<float> *pinput, int psize, Scales *scales, complex<float> *poutput, int pn1, int pn2) {
    assert((psize * scales->nscales) == (pn1 * pn2));
    cwt(pinput, psize, poutput, scales);
}
