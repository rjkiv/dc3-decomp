#include "synth360/synapse_apo/SpectralAnalysis.h"
#include "math/Trig.h"

void DSP::SpectralAnalysis::SetMode(unsigned int ui1, unsigned int ui2) {
    m_sWindowLen = ui1;
    m_sFft = 8;
    if (ui2 == -1) {
        ui2 = ui1;
    }
    for (; m_sFft < m_sWindowLen + ui2; m_sFft <<= 1)
        ;
    m_nBins = (m_sFft >> 1) + 1;
    m_fft.SetMode(m_sFft);
    m_fft2.SetMode(m_sFft >> 1);
    m_aSig.assign(m_sFft, 0);
    m_aSp.resize(m_sFft + 2);
    m_aSpReal.resize((m_sFft >> 1) + 1);
    m_aSpImag.resize((m_sFft >> 1) + 1);
    m_fSines.resize(m_sFft >> 1);
    m_fCosines.resize(m_sFft >> 1);

    for (int i = 0; i < m_sFft >> 1; i++) {
        float x = ((float)i * PI) / (float)(m_sFft >> 1);
        m_fSines[i] = sinf(x);
        m_fCosines[i] = cosf(x);
    }
}
