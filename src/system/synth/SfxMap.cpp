#include "synth/Sfx.h"
#include "synth/Stream.h"
#include "utl/BinStream.h"

SfxMap::SfxMap(Hmx::Object *obj)
    : mSample(obj), mVolume(0), mPan(0), mTranspose(0), mFXCore(kFXCoreNone) {}

void SfxMap::Save(BinStream &bs) const {
    bs << mSample;
    bs << mVolume;
    bs << mPan;
    bs << mTranspose;
    bs << mFXCore;
    bs << mADSR;
}

void SfxMap::Load(BinStreamRev &d) {
    d >> mSample;
    if (d.rev > 2) {
        d >> mVolume;
        d >> mPan;
        d >> mTranspose;
        int fx;
        d >> fx;
        mFXCore = (FXCore)fx;
        if (d.rev >= 4) {
            d >> mADSR;
        }
    }
}

BinStream &operator<<(BinStream &bs, const SfxMap &s) {
    s.Save(bs);
    return bs;
}

BinStream &operator>>(BinStreamRev &d, SfxMap &s) {
    s.Load(d);
    return d;
}
