#pragma once

namespace DSP {
    namespace Synapse {
        class PitchCorrectedVoice {
        public:
            PitchCorrectedVoice();
            void SetAmount(float);
            void SetProximityEffect(float);
            void SetProximityFocus(float);
            void SetReleaseSmoothing(float);
            void SetTransposition(float);
            float GetCorrection();
            void SetAttackSmoothing(float);

        private:
            float unk0;
            float unk4;
            float unk8;
            float unkc;
            float unk10;
            float mReleaseSmoothing; // 0x14
            float mTransposition; // 0x18
            float mAmount; // 0x1c
            float mProximityEffect; // 0x20
            float mProximityFocus; // 0x24
            float unk28;
            float unk2c;
            float unk30;
            float unk34;
        };
    }
}
