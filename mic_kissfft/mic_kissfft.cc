// https://forums.freertos.org/t/fast-fourier-transform-kissfft-issue/14699
// https://www.hackster.io/AlexWulff/adc-sampling-and-fft-on-raspberry-pi-pico-f883dd
// https://github.com/AlexFWulff/awulff-pico-playground/blob/main/adc_fft/adc_fft.c
#include "third_party/kissfft/kiss_fft.h"
// #include "kiss_fftnd.h"
// #include "kiss_fftndr.h"
// #include "kiss_fftr.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
// #include "random.h"

# define NSAMP 1024

void do_fft(kiss_fft_cpx *in, kiss_fft_cpx *out, int direction) {
  kiss_fft_cfg cfg;
  printf("fft\n\r");
  /* solution 1
   if (direction==0) kfc_fft(N,in,out);
     else            kfc_ifft(N,in,out);
   kfc_cleanup();
  */

  /* solution 2 */
  if ((cfg = kiss_fft_alloc(NSAMP, direction, NULL, NULL)) != NULL) {
    kiss_fft(cfg, in, out);
    kiss_fft_free(cfg);
  } else
    printf("FFT out of memory\n\r");
  printf(":%f\n\r", in[NSAMP].r);
}

int main() {
  char pattern1[NSAMP];
//   RandomGenerate(pattern1,NSAMP);
  kiss_fft_cpx *in, *out1;//, *out2, *outm;
  int k;
//   in = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
//   out1 = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
//   out2 = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
//   outm = (kiss_fft_cpx *)malloc((NSAMP + 1) * sizeof(kiss_fft_cpx));
  in = new kiss_fft_cpx[NSAMP];
  out1 = new kiss_fft_cpx[NSAMP];
//   out2 = new kiss_fft_cpx[NSAMP];
//   outm = new kiss_fft_cpx[NSAMP];
  

  for (k = 0; k < NSAMP; k++) {
    in[k].r = (pattern1[k] << 30);
    in[k].i = 0;
  }
  for (k = 0; k < 3; k++) {
    printf("%X:", k );
    do_fft(in, out1, 0);
  }

//   for (k = 0; k < N; k++) {
//     in[k].r = (pattern2[k] << 30);
//     in[k].i = 0;
//   }
//   for (k = 0; k < 3; k++) {
//     printf("%X:", k );
//     do_fft(in, out2, 0);
//   }
}