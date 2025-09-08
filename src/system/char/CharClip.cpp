#include "char/CharClip.h"
#include "obj/Object.h"

CharClip::CharClip()
    : mTransitions(this), mFramesPerSec(30), mFlags(0), mPlayFlags(0), mRange(0),
      mRelative(this), mDirty(true), mOldVer(-1), mDoNotCompress(false), mSyncAnim(this),
      unk198(0) {
    mBeatTrack.resize(1);
    mBeatTrack[0].frame = 0.0f;
    mBeatTrack[0].value = 0.0f;
}

void CharClip::Init() { REGISTER_OBJ_FACTORY(CharClip); }
