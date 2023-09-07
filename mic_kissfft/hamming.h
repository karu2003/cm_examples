#include <inttypes.h>

class Hamming
{
private:
    float *m_coef;
    int m_window_size;

public:
    Hamming(int window_size);
    ~Hamming();
    void applyWindow(float *input);
    void showCoefficients(void);

};