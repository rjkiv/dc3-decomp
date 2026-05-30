#pragma once
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
        };
    }
}
