#pragma once
#include "synth360/synapse_apo/SpectralAnalysis.h"
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x128
        class PitchDetector {
        public:
            PitchDetector(const std::vector<float> &, unsigned int, unsigned int);
            ~PitchDetector();

            void Detect(unsigned int);

        private:
            const std::vector<float> &m_delayLine; // 0x0
            unsigned int m_sMinPitchLag; // 0x4
            unsigned int m_sMaxPitchLag; // 0x8
            float m_fPitchLag; // 0xc
            float m_fTonality; // 0x10
            float m_fEnergy; // 0x14
            SpectralAnalysis m_spectralAnalysis; // 0x18
            std::vector<float> m_fTemp; // 0x100
            std::vector<float> m_fWin; // 0x10c
            std::vector<float> m_fWin2; // 0x118
        };
    }
}
