#pragma once
#include "synth360/synapse_apo/Biquad.h"
#include "synth360/synapse_apo/GranularSynth.h"
#include "synth360/synapse_apo/PeakDetector.h"
#include "synth360/synapse_apo/PitchDetector.h"
#include "synth360/synapse_apo/common_vector.h"
#include "synth360/synapse_apo/PitchCorrectedVoice.h"
#include <vector>

namespace DSP {
    namespace Synapse {
        // size 0x80
        class Synapse {
        public:
            Synapse(float);
            ~Synapse();

            void SetVoiceEnabled(unsigned int, bool);
            void SetVoiceGain(unsigned int, float);
            void SetVoiceTargetNote(unsigned int, float);
            void SetVoiceTransposition(unsigned int, float);
            void SetVoiceAmount(unsigned int, float);
            void SetVoiceProximityEffect(unsigned int, float);
            void SetVoiceProximityFocus(unsigned int, float);
            void SetAttackSmoothing(float);
            void SetReleaseSmoothing(float);
            void ProcessInPlace(unsigned int, float *);

        private:
            std::vector<float> m_fInputDelay; // 0x0
            std::vector<float> m_fDnInputDelay; // 0xc
            unsigned int m_iInputDelayOffset; // 0x18
            unsigned int m_sMinPitchLag; // 0x1c
            unsigned int m_sMaxPitchLag; // 0x20
            unsigned int m_sPitchDetectionStride; // 0x24
            scoped_ptr<PitchDetector> m_pPitchDetector; // 0x28
            float m_fPitchLag; // 0x2c
            float m_fTonality; // 0x30
            float m_fEnergy; // 0x34
            float m_fTonalityThreshold; // 0x38
            float m_fGain; // 0x3c
            scoped_ptr<PeakDetector> m_pPeakDetector; // 0x40
            std::vector<std::vector<float> > m_fTempOutputs; // 0x44
            std::vector<float *> m_pTempOutputs; // 0x50
            std::vector<PitchCorrectedVoice> m_voices; // 0x5c
            scoped_ptr<GranularSynth> m_pGranularSynth; // 0x68
            float m_fSamplingRate; // 0x6c
            scoped_ptr<Biquad> m_pPreFilterLowpass; // 0x70
            scoped_ptr<Biquad> m_pPreFilterHighpass; // 0x74
            float m_fAvgSignal; // 0x78
            float m_kAvgSignalSmoothing; // 0x7c
        };
    }
}
