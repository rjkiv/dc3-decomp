#include "synth360/synapse_apo/FilterCoeffs.h"
#include "math/Trig.h"
#include <cmath>

namespace DSP {
	
	// thank you audio EQ cookbook
	
    void LowpassCoefficients(float *const coeffs, float sampleRate, float freq, float q) {
        if (freq > sampleRate * 0.5) {
            coeffs[0] = coeffs[3] = 1;
            coeffs[1] = coeffs[2] = coeffs[4] = coeffs[5] = 0;
            return;
        }
        double w0 = 2.0 * PI * freq / sampleRate;
        double sinW0 = sin(w0);
        double cosW0 = cos(w0);
        double alpha = sinW0 / (2 * q);

        coeffs[0] = (1 - cosW0) / 2;
        coeffs[1] = 1 - cosW0;
        coeffs[2] = (1 - cosW0) / 2;
        coeffs[3] = 1 + alpha;
        coeffs[4] = -2 * cosW0;
        coeffs[5] = 1 - alpha;

        for (int i = 0; i < 3; i++) {
            coeffs[i] /= coeffs[3];
        }
        for (int i = 4; i < 6; i++) {
            coeffs[i] /= coeffs[3];
        }
        coeffs[3] = 1;
    }

    void
    HighpassCoefficients(float *const coeffs, float sampleRate, float freq, float q) {
        if (freq > sampleRate * 0.5) {
            coeffs[3] = 1;
            coeffs[0] = coeffs[1] = coeffs[2] = coeffs[4] = coeffs[5] = 0;
            return;
        }
        double w0 = 2.0 * PI * freq / sampleRate;
        double sinW0 = sin(w0);
        double cosW0 = cos(w0);
        double alpha = sinW0 / (2 * q);

        coeffs[0] = (1 + cosW0) / 2;
        coeffs[1] = -(1 + cosW0);
        coeffs[2] = (1 + cosW0) / 2;
        coeffs[3] = 1 + alpha;
        coeffs[4] = -2 * cosW0;
        coeffs[5] = 1 - alpha;

        for (int i = 0; i < 3; i++) {
            coeffs[i] /= coeffs[3];
        }
        for (int i = 4; i < 6; i++) {
            coeffs[i] /= coeffs[3];
        }
        coeffs[3] = 1;
    }
}
