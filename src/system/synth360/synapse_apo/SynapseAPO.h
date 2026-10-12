#pragma once
#include "synth360/synapse_apo/Synapse_dsp.h"
#include "xdk/XAUDIO2.h"

namespace DSP {

    struct SynapseAPOVoiceParams {
        bool bEnabled; // 0x0
        float fNoteHz; // 0x4
        float fGain; // 0x8
        float fTransposition; // 0xc
        float fAmount; // 0x10
        float fProximityEffect; // 0x14
        float fProximityFocus; // 0x18
    };

    // size 0x5c
    struct SynapseAPOParams {
        SynapseAPOParams() {
            for (int i = 0; i < 3; i++) {
                mNoteProps[i].bEnabled = false;
                mNoteProps[i].fNoteHz = 220;
                mNoteProps[i].fGain = 0;
                mNoteProps[i].fTransposition = 0;
            }
            fAttackSmoothing = 20;
            fReleaseSmoothing = 40;
        }

        SynapseAPOVoiceParams mNoteProps[3]; // 0x0
        /** "attack time ms for correction". Ranges from 10 to 500. */
        float fAttackSmoothing; // 0x54
        /** "release time ms for correction". Ranges from 10 to 500. */
        float fReleaseSmoothing; // 0x58
    };

    DEFINE_CLSID(SynapseAPO, 03004D97, D165, 4CC0, AB, DD, 6A, 98, F0, 4E, 6E, B7);

    class SynapseAPO : public ATG::CSampleXAPOBase<SynapseAPO, SynapseAPOParams> {
    public:
        SynapseAPO();
        virtual ~SynapseAPO();

        virtual void
        DoProcess(const SynapseAPOParams &, float *__restrict, unsigned int, unsigned int);

        void SetSamplingRate(float);

    private:
        virtual void OnSetParameters(const SynapseAPOParams &);

        Synapse::Synapse *m_pSynapse; // 0x168
        SynapseAPOParams m_params; // 0x16c
    };

}
