#include "synth/MoggClipMap.h"
#include "utl/BinStream.h"

void MoggClipMap::mySave(BinStream &bs) const {
    bs << mMoggClip;
    bs << mVolume;
    bs << mPan;
    bs << mPanWidth;
    bs << mIsStereo;
}

MoggClipMap::MoggClipMap(Hmx::Object *obj)
    : mMoggClip(obj), mPan(0.0f), mPanWidth(0.0f), mVolume(0.0f), mIsStereo(false) {}

MoggClipMap::MoggClipMap(const MoggClipMap &mogg)
    : mMoggClip(mogg.mMoggClip), mPan(mogg.mPan), mPanWidth(mogg.mPanWidth),
      mVolume(mogg.mVolume), mIsStereo(mogg.mIsStereo) {}

MoggClipMap &MoggClipMap::operator=(const MoggClipMap &mogg) {
    mMoggClip = mogg.mMoggClip;
    mPan = mogg.mPan;
    mPanWidth = mogg.mPanWidth;
    mVolume = mogg.mVolume;
    mIsStereo = mogg.mIsStereo;
    return *this;
}

BinStream &operator<<(BinStream &bs, const MoggClipMap &mogg) {
    mogg.mySave(bs);
    return bs;
}

void MoggClipMap::myLoad(BinStreamRev &bs) {
    bs >> mMoggClip;
    if (bs.rev >= 11) {
        bs >> mVolume;
        bs >> mPan;
        bs >> mPanWidth;
        bs >> mIsStereo;
    }
}

BinStream &operator>>(BinStreamRev &bs, MoggClipMap &mogg) {
    mogg.myLoad(bs);
    return bs.stream;
}
