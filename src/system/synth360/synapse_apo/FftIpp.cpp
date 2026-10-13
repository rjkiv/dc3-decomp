#include "synth360/synapse_apo/FftIpp.h"
#include "os/Debug.h"
#include "synth360/synapse_apo/FFT.h"
#include "utl/MemMgr.h"
#include <cstring>

FftIpp::FftIpp() : m_sFFT(0), m_uOrder(0) {}

FftIpp::~FftIpp() {}

void FftIpp::SetMode(int i1) {
    m_sFFT = i1;
    m_uOrder = 1;
    for (; 1 << m_uOrder < (int)m_sFFT; m_uOrder++)
        ;
    m_fReal.resize(i1);
    m_fImaginary.resize(m_sFFT);
    m_fTemp.resize(m_sFFT);
    m_fSinCosTable.resize(m_sFFT);
    CalculateSinCosTable((int)m_sFFT / 2, &m_fSinCosTable[0]);
}

void FftIpp::FftRealCcs(float const *__restrict f1, float *__restrict f2) {
    if ((unsigned int)m_sFFT) {
        memcpy(&m_fTemp[0], f1, m_sFFT * 4);
    }
    int iRetVal = FFTRealForward(&m_fTemp[0], m_sFFT, &m_fSinCosTable[0]);
    MILO_ASSERT(iRetVal == 0, 0x65);
    if ((unsigned int)m_sFFT != 0) {
        memcpy(f2, &m_fTemp[0], m_sFFT * 4);
    }
    f2[m_sFFT] = f2[1];
    f2[m_sFFT + 1] = 0;
    f2[1] = 0;
}

void FftIpp::FftReal(
    float const *__restrict f1, float *__restrict f2, float *__restrict f3
) {
    if (m_sFFT) {
        memcpy(&m_fTemp[0], f1, m_sFFT << 2);
    }
    FFTRealForward(&m_fTemp[0], m_sFFT, &m_fSinCosTable[0]);
    for (int i = 1; i < m_sFFT >> 1; i++) {
        f2[i] = m_fTemp[i];
        f3[i] = m_fTemp[i];
    }
    f3[0] = 0;
    f2[0] = m_fTemp[0];
    f2[m_sFFT] = m_fTemp[1];
    f3[m_sFFT] = 0;
}
