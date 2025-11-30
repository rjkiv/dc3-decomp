#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"

/** "An equalizer effect." */
class FxSendEQ : public FxSend {
public:
    virtual ~FxSendEQ();
    OBJ_CLASSNAME(FxSendEQ);
    OBJ_SET_TYPE(FxSendEQ);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0xF);
    NEW_OBJ(FxSendEQ)

protected:
    FxSendEQ();

    /** "High frequency cutoff, in Hz". Ranges from 0 to 24000. */
    float mHighFreqCutoff; // 0x60
    /** "High frequency gain, in dB". Ranges from -42 to 42. */
    float mHighFreqGain; // 0x64
    /** "Mid frequency cutoff, in Hz". Ranges from 0 to 24000. */
    float mMidFreqCutoff; // 0x68
    /** "Mid frequency bandwidth, in Hz". Ranges from 0 to 24000. */
    float mMidFreqBandwidth; // 0x6c
    /** "Mid frequency gain, in dB". Ranges from -42 to 42. */
    float mMidFreqGain; // 0x70
    /** "Low frequency cutoff, in Hz". Ranges from 0 to 24000. */
    float mLowFreqCutoff; // 0x74
    /** "Low frequency gain, in dB". Ranges from -42 to 42. */
    float mLowFreqGain; // 0x78
    /** "Low pass filter cutoff, in Hz". Ranges from 20 to 20000. */
    float mLowPassCutoff; // 0x7c
    /** "Low pass filter resonance, in dB". Ranges from -25 to 25. */
    float mLowPassReso; // 0x80
    /** "High pass filter cutoff, in Hz". Ranges from 20 to 20000. */
    float mHighPassCutoff; // 0x84
    /** "High pass filter resonance, in dB". Ranges from -25 to 25. */
    float mHighPassReso; // 0x88
    /** "Enable or disable Linkwitz-Riley mode" */
    bool mLRMode; // 0x8c
    /** "Transition time for gain changes, in ms". Ranges from 25 to 5000. */
    float mTransitionTime; // 0x90
};
