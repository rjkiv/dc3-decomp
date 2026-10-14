#include "synth/SampleZone.h"
#include "obj/Object.h"
#include "synth/Stream.h"
#include "utl/BinStream.h"

SampleZone::SampleZone(Hmx::Object *owner)
    : mSample(owner), mVolume(0.0f), mPan(0.0f), mCenterNote(0x24), mMinNote(0),
      mMaxNote(0x7f), mMinVelo(0), mMaxVelo(0x7f), mFX(kFXCoreNone) {}

void SampleZone::Save(BinStream &bs) const {
    bs << mSample;
    bs << mVolume;
    bs << mPan;
    bs << mCenterNote;
    bs << mMinNote;
    bs << mMaxNote;
    bs << mFX;
    bs << mADSR;
    bs << mMinVelo;
    bs << mMaxVelo;
}

void SampleZone::Load(BinStreamRev &d) {
    d >> mSample;
    d >> mVolume;
    d >> mPan;
    d >> mCenterNote;
    d >> mMinNote;
    d >> mMaxNote;
    int fx;
    d >> fx;
    mFX = (FXCore)fx;
    d >> mADSR;
    if (d.rev >= 2) {
        d >> mMinVelo;
        d >> mMaxVelo;
    }
}

bool SampleZone::Includes(unsigned char note, unsigned char vel) {
    return mMinNote <= note && note <= mMaxNote && mMinVelo <= vel && vel <= mMaxVelo;
}

BinStream &operator<<(BinStream &bs, const SampleZone &sz) {
    sz.Save(bs);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, SampleZone &sz) {
    sz.Load(d);
    return d;
}
