#pragma once
#include "obj/Object.h"
#include "synth/ADSR.h"
#include "synth/Stream.h"
#include "synth/SynthSample.h"
#include "utl/BinStream.h"

class SampleZone {
public: // this isn't in RB2, making everything public based RB3 retail and overall vibes
    SampleZone(Hmx::Object *);
    void Save(BinStream &) const;
    void Load(BinStreamRev &);
    bool Includes(unsigned char, unsigned char);

    /** "Which sample to play" */
    ObjPtr<SynthSample> mSample; // 0x0
    /** "Volume in dB (0 is full volume, -96 is silence)" */
    float mVolume; // 0x14
    /** "Surround pan, between -4 and 4" */
    float mPan; // 0x18
    /** "note at which sample pays without pitch change" */
    int mCenterNote; // 0x1c
    /** "Lowest zone note" */
    int mMinNote; // 0x20
    /** "Highest zone note" */
    int mMaxNote; // 0x24
    /** "Lowest zone velocity" */
    int mMinVel; // 0x28
    /** "Highest zone velocity" */
    int mMaxVel; // 0x2c
    /** "Which core's digital FX should be used in playing this sample" */
    FXCore mFXCore; // 0x30
    ADSRImpl mADSR; // 0x34
};

BinStream &operator<<(BinStream &, const SampleZone &);
BinStreamRev &operator>>(BinStreamRev &, SampleZone &);
