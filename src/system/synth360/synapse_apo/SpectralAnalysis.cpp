#include "synth360/synapse_apo/SpectralAnalysis.h"
#include <cmath>

static const double izPI = 3.141592653589793;
static const double LOG_2_INV = 1.442695040888954;

void DSP::SpectralAnalysis::SetMode(unsigned int ui1, unsigned int ui2) {
    m_sWindowLen = ui1;
    m_sFft = 8;
    if (ui2 == -1) {
        ui2 = ui1;
    }
    for (; m_sFft < m_sWindowLen + ui2; m_sFft <<= 1)
        ;
    m_nBins = (m_sFft / 2) + 1;
    m_fft.SetMode(m_sFft);
    m_fft2.SetMode(m_sFft / 2);
    m_aSig.assign(m_sFft, 0);
    m_aSp.resize(m_sFft + 2);
    m_aSpReal.resize((m_sFft / 2) + 1);
    m_aSpImag.resize((m_sFft / 2) + 1);
    m_fSines.resize(m_sFft / 2);
    m_fCosines.resize(m_sFft / 2);

    for (unsigned int i = 0; i < m_sFft / 2; i++) {
        double x = (i * izPI) / (double)(m_sFft / 2);
        double sine = sin(x);
        m_fSines[i] = sine;
        double cosine = cos(x);
        m_fCosines[i] = cosine;
    }
}
