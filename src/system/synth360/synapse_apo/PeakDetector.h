#pragma once
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x34
        class PeakDetector {
        public:
            PeakDetector(const std::vector<float> &, unsigned int, unsigned int);
            ~PeakDetector();
            void Detect(unsigned int);

        private:
            float gaussianWindow(unsigned int) const;

            const std::vector<float> &m_delayLine; // 0x0
            float m_fOutsidePitchLag; // 0x4
            unsigned int m_sMinPitchLag; // 0x8
            unsigned int m_sMaxPitchLag; // 0xc
            unsigned int m_iLastPeak; // 0x10
            unsigned int m_iPeakCand; // 0x14
            unsigned int m_iStartPeakPeriod; // 0x18
            unsigned int m_sPitchLag; // 0x1c
            bool m_bInsidePeak; // 0x20
            float m_fLockedPitchLag; // 0x24
            float m_fLastPeak; // 0x28
            float m_fMaxPeakCand; // 0x2c
            float m_fNewlyDetectedGranule; // 0x30
        };
    }
}
