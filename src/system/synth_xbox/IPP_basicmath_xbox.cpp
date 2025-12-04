#include "synth_xbox/IPP_basicmath_xbox.h"
#include "IPP_basicmath_xbox.h"

namespace IPP {
    void Add_InPlace(unsigned int size, const float *f1, float *f2) {
        if (size == 0)
            return;
        for (unsigned int i = 0; i < size; ++i) {
            f2[i] = f1[i] + f2[i];
        }
    }

    void MulConstant_InPlace(unsigned int size, float *f1, float f2) {
        if (size == 0)
            return;
        for (int i = 0; i < size; i++) {
            f1[i] *= f2;
        }
    }

    void Mul_InPlace(unsigned int size, const float *f1, float *f2) {
        if (size == 0)
            return;

        for (int i = 0; i < size; i++) {
            f2[i] = f1[i] * f2[i];
        }
    }

    void Mul(unsigned int size, const float *f1, const float *f2, float *f3) {
        if (size == 0)
            return;
        for (int i = 0; i < size; i++) {
            f3[i] = f1[i] * f2[i];
        }
    }

}
