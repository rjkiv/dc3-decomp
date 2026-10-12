#pragma once

namespace DSP {
    namespace Synapse {
        class PitchCorrectedVoice {
        public:
            PitchCorrectedVoice();
            ~PitchCorrectedVoice();
            void SetAmount(float);
            void SetProximityEffect(float);
            void SetProximityFocus(float);
            void SetReleaseSmoothing(float);
            void SetTransposition(float);
            float GetCorrection();
            void SetAttackSmoothing(float);

            void SetTargetNote(float note) { m_fTargetNoteHz = note; }

        private:
            float m_fDetectedNoteHz; // 0x0
            float m_fTargetNoteHz; // 0x4
            float m_fSmoothedCorrection; // 0x8
            float m_fBaseSmoothing; // 0xc
            float m_fAttackSmoothing; // 0x10
            float m_fReleaseSmoothing; // 0x14
            float m_fTransposition; // 0x18
            float m_fAmount; // 0x1c
            float m_fProximityEffect; // 0x20
            float m_fProximityFocus; // 0x24
            float m_fTonality; // 0x28
            float m_fEnergy; // 0x2c
            float m_fPrevEnergy; // 0x30
            float m_fErrorMagnitude; // 0x34
        };
    }
}
