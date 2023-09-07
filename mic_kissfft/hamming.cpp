#include <stdlib.h>
#include <math.h>
#include "hamming.h"

#define PI 3.14159265358979323846

Hamming::Hamming(int window_size)
{
    //Save actual windows size.
    m_window_size = window_size;

    //Allocate the memory for the constants
    m_coef = static_cast<float *>(malloc(sizeof(float) * m_window_size));
    
    // create the constants for a hamming window
    const float arg =  (2.0 * PI) / window_size;
    for (int i = 0; i < window_size; i++)
    {
        float n_coef_value = 0.54 - (0.46 * cos(arg * (i+ 0.5)));
        m_coef[i] = n_coef_value;
    }
}

Hamming::~Hamming()
{
    free(m_coef);
}

void Hamming::applyWindow(float *input)
{
    for (int i = 0; i < m_window_size; i++)
        input[i] = input[i] * m_coef[i];
}

// void Hamming::showCoefficients(void)
// {
//     int i;
//     for (i = 0; i < m_window_size - 1; i++)
//         { 
//             Serial.printf("%.4f",m_coef[i]);
//             Serial.print(',');
//         }

//     Serial.printf("%.4f",m_coef[i]);
// }
