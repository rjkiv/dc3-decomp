#pragma once
#include "synth_xbox/Synapse_dsp.h"
#include "xdk/XAUDIO2.h"

namespace DSP {

    struct VoiceParams {
        bool enabled; // 0x0
        float targetNote; // 0x4
        float gain; // 0x8
        float transposition; // 0xc
        float amount; // 0x10
        float proximityEffect; // 0x14
        float proximityFocus; // 0x18
    };

    // size 0x5c
    struct SynapseAPOParams {
        SynapseAPOParams() {
            for (int i = 0; i < 3; i++) {
                mNoteProps[i].enabled = false;
                mNoteProps[i].targetNote = 220;
                mNoteProps[i].gain = 0;
                mNoteProps[i].transposition = 0;
            }
            attackSmoothing = 20;
            releaseSmoothing = 40;
        }

        VoiceParams mNoteProps[3]; // 0x0
        /** "attack time ms for correction". Ranges from 10 to 500. */
        float attackSmoothing; // 0x54
        /** "release time ms for correction". Ranges from 10 to 500. */
        float releaseSmoothing; // 0x58
    };

    class SynapseAPO : public ATG::CSampleXAPOBase<SynapseAPO, SynapseAPOParams> {
    public:
        SynapseAPO();
        virtual ~SynapseAPO();

        virtual void
        DoProcess(const SynapseAPOParams &, float *__restrict, unsigned int, unsigned int);

        void SetSamplingRate(float);

    private:
        virtual void OnSetParameters(const SynapseAPOParams &);

        Synapse::Synapse *mSynapse; // 0x168
        SynapseAPOParams mParams; // 0x16c
    };

}
