#ifndef DAC_OUT_H_
#define DAC_OUT_H_

#include <math.h>

inline  static double log_freq_func (double w0, double w1, double indx)
{	return pow (10.0, log10 (w0) + (log10 (w1) - log10 (w0)) * indx) ;
} /* log_freq_func */

inline  static double quad_freq_func (double w0, double w1, double indx)
{	return w0 + (w1 - w0) * indx * indx ;
} /* log_freq_func */

inline  static float linear_freq_func (float w0, float w1, float indx)
{	return w0 + (w1 - w0) * indx ;
} /* linear_freq_func */


#endif  // DAC_OUT_H_