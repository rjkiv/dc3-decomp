#include "synth360/synapse_apo/FFT.h"

// pretty sure these are all static ints
// aka confined to this TU
int fft_matrix_forward_columnwise(float *, long, float *);
int fft_matrix_inverse_columnwise(float *, long, float *);
int fft_square_matrix(float *, long, long, float *);
int fft_altivec(float *, float *, unsigned long, long, float *);
int fft_scalar(float *, float *, unsigned long, long, float *);
int fft_pingpong(float *, unsigned long, long, float *);
int fft_recursive(float *, unsigned long, long, float *);
int fft_real_forward_altivec(float *, long, float *);
int fft_real_forward_scalar(float *, unsigned long, float *);

int FFTRealForward(float *f1, unsigned long ul, float *f2) {
    if (ul < 0x20) {
        return fft_real_forward_scalar(f1, ul, f2);
    } else {
        return fft_real_forward_altivec(f1, ul, f2);
    }
}

int FFTComplex(float *f1, long l2, long l3, float *f4) {
    if (l2 <= 0x8000) {
        return fft_pingpong(f1, l2, l3, f4);
    }
    bool b1 = true;
    if (l2 == 1) {
        b1 = false;
    } else {
        for (int u3 = 2; u3 < l2; u3 <<= 1) {
            b1 ^= true;
        }
    }
    if (!b1) {
        return fft_square_matrix(f1, l2, l3, f4);
    } else {
        return fft_recursive(f1, l2, l3, f4);
    }
}
