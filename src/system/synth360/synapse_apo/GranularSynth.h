#pragma once
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x44
        class GranularSynth {
        public:
            // size 0x40
            struct Granule {
                bool bActive; // 0x0
                float fInStart; // 0x4
                float fLength; // 0x8
                float fGain; // 0xc
                double fOutStart; // 0x10
                double fNextStart; // 0x18
                unsigned int iVoice; // 0x20
                unsigned int iCrossfade; // 0x24
                unsigned int uCrossfadeLength; // 0x28
                unsigned int iOffset; // 0x2c
                unsigned int sPasteLength; // 0x30
                int sEndSamples; // 0x34
                int sDeactivateSamples; // 0x38
                float fCurIn; // 0x3c
            };
            // size 0x18
            struct Voice {
                float fFormantScaling; // 0x0
                float fGain; // 0x4
                float fPitchShiftRatio; // 0x8
                bool bEnabled; // 0xc
                double fOutNextStart; // 0x10
            };

            GranularSynth(
                const std::vector<float> &, unsigned int, unsigned int, unsigned int
            );
            ~GranularSynth();

            void SetVoiceEnabled(unsigned int idx, bool enabled) {
                if (enabled && !m_voices[idx].bEnabled) {
                    if ((float)m_iCurOut - m_voices[idx].fOutNextStart
                        > m_sMaxPitchLag * 3) {
                        m_voices[idx].fOutNextStart = m_iCurOut;
                    }
                }
                m_voices[idx].bEnabled = enabled;
            }

            void SetVoiceGain(unsigned int idx, float gain) {
                m_voices[idx].fGain = gain;
            }

        private:
            const std::vector<float> &m_delayLine; // 0x0
            float m_fPitchLag; // 0x4
            float m_fNewGranuleStart; // 0x8
            float m_fTonality; // 0xc
            unsigned int m_sMinPitchLag; // 0x10
            unsigned int m_sMaxPitchLag; // 0x14
            unsigned int m_iCurOut; // 0x18
            unsigned int m_iBufferOffset; // 0x1c
            std::vector<Granule> m_granules; // 0x20
            std::vector<Voice> m_voices; // 0x2c
            std::vector<std::vector<float> > m_fCrossfades; // 0x38
        };
    }
}
