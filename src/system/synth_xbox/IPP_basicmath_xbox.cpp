#include "synth_xbox/IPP_basicmath_xbox.h"

namespace IPP {
    void Add_InPlace(unsigned int size, const float *src, float *dst) {
        for (int i = 0; i < size; i++) {
            dst[i] += src[i];
        }
    }

    void MulConstant_InPlace(unsigned int size, float *dst, float fConstant) {
        for (int i = 0; i < size; i++) {
            dst[i] *= fConstant;
        }
    }

    void Mul_InPlace(unsigned int size, const float *src, float *dst) {
        for (int i = 0; i < size; i++) {
            dst[i] *= src[i];
        }
    }

    void Mul(unsigned int size, const float *srcA, const float *srcB, float *dst) {
        for (int i = 0; i < size; i++) {
            dst[i] = srcA[i] * srcB[i];
        }
    }

}
