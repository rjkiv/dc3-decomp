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
            std::vector<float> unk0;
            std::vector<float> unkc;
            unsigned int unk18;
            unsigned int unk1c;
            unsigned int unk20;
            unsigned int unk24;
            scoped_ptr<PitchDetector> mPitchDetector; // 0x28
            float unk2c;
            float unk30;
            float unk34;
            float unk38;
            float unk3c;
            scoped_ptr<PeakDetector> mPeakDetector; // 0x40
            std::vector<std::vector<float> > unk44; // 0x44
            std::vector<float *> unk50; // 0x50
            std::vector<PitchCorrectedVoice> mVoices; // 0x5c
            scoped_ptr<GranularSynth> mGranularSynth; // 0x68
            float mSampleRate; // 0x6c
            scoped_ptr<Biquad> mLowPassBiquad; // 0x70
            scoped_ptr<Biquad> mHighPassBiquad; // 0x74
            float unk78;
            float unk7c;
        };
    }
}
