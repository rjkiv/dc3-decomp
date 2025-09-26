#include "char/CharFaceServo.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"

CharFaceServo::CharFaceServo()
    : mClips(this), mBaseClip(this), mBlinkClipLeft(this), mBlinkClipLeft2(this),
      mBlinkClipRight(this), mBlinkClipRight2(this), mBlinkWeightLeft(0),
      mBlinkWeightRight(0), unk110(0), unk114(0), unk118(0) {}

CharFaceServo::~CharFaceServo() {}

BEGIN_HANDLERS(CharFaceServo)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharFaceServo)
    SYNC_PROP_SET(clips, mClips.Ptr(), SetClips(_val.Obj<ObjectDir>()))
    SYNC_PROP_SET(clip_type, mClipType, SetClipType(_val.Sym()))
    SYNC_PROP(blink_clip_left, mBlinkClipLeftName)
    SYNC_PROP(blink_clip_left2, mBlinkClipLeftName2)
    SYNC_PROP(blink_clip_right, mBlinkClipRightName)
    SYNC_PROP(blink_clip_right2, mBlinkClipRightName2)
    SYNC_SUPERCLASS(CharBonesMeshes)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharFaceServo)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mClips;
    bs << mClipType;
    bs << mBlinkClipLeftName;
    bs << mBlinkClipRightName;
    bs << mBlinkClipLeftName2;
    bs << mBlinkClipRightName2;
END_SAVES

BEGIN_COPYS(CharFaceServo)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharFaceServo)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBlinkWeightLeft)
        COPY_MEMBER(mBlinkWeightRight)
        COPY_MEMBER(mBlinkClipLeftName)
        COPY_MEMBER(mBlinkClipRightName)
        COPY_MEMBER(mBlinkClipLeftName2)
        COPY_MEMBER(mBlinkClipRightName2)
        SetClips(c->mClips);
        SetClipType(c->mClipType);
    END_COPYING_MEMBERS
END_COPYS

void CharFaceServo::Enter() {
    RndPollable::Enter();
    unk110 = true;
    unk114 = 0;
}

float CharFaceServo::BlinkWeightLeft() const { return mBlinkWeightLeft; }

void CharFaceServo::SetClips(ObjectDir *clipDir) {
    mClips = clipDir;
    if (mClips) {
        mBaseClip = mClips->Find<CharClip>("Base", false);
        mBlinkClipLeft = mClips->Find<CharClip>(mBlinkClipLeftName.Str(), false);
        mBlinkClipLeft2 = mClips->Find<CharClip>(mBlinkClipLeftName2.Str(), false);
        mBlinkClipRight = mClips->Find<CharClip>(mBlinkClipRightName.Str(), false);
        mBlinkClipRight2 = mClips->Find<CharClip>(mBlinkClipRightName2.Str(), false);
    }
}

void CharFaceServo::SetBlinkClipLeft(Symbol name) {
    mBlinkClipLeftName = name;
    if (mClips)
        SetClips(mClips);
}

void CharFaceServo::SetBlinkClipRight(Symbol name) {
    mBlinkClipRightName = name;
    if (mClips)
        SetClips(mClips);
}

void CharFaceServo::SetClipType(Symbol type) {
    if (type != mClipType) {
        mClipType = type;
        ClearBones();
        CharBoneDir::StuffBones(*this, mClipType);
        unk110 = true;
    }
}
