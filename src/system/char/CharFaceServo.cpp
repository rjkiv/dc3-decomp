#include "char/CharFaceServo.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"

CharFaceServo::CharFaceServo()
    : mClips(this), mBaseClip(this), mBlinkClipLeft(this), mBlinkClipLeft2(this),
      mBlinkClipRight(this), mBlinkClipRight2(this), mBlinkWeightLeft(0),
      mBlinkWeightRight(0), mNeedScaleDown(0), mProceduralBlinkWeight(0),
      mAppliedProceduralBlink(0) {}

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

BEGIN_LOADS(CharFaceServo)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    ObjPtr<ObjectDir> oDirPtr(this);
    bs >> oDirPtr;
    Symbol sym;
    if (d.rev > 3)
        bs >> sym;
    else if (oDirPtr) {
        sym = oDirPtr->Type();
        if (sym.Null()) {
            for (ObjDirItr<CharClip> it(oDirPtr, true); it != nullptr; ++it) {
                sym = it->Type();
                break;
            }
        }
    }
    if (d.rev != 0)
        bs >> mBlinkClipLeftName;
    if (d.rev > 1)
        bs >> mBlinkClipRightName;
    if (d.rev > 2) {
        bs >> mBlinkClipLeftName2;
        bs >> mBlinkClipRightName2;
    }
    SetClips(oDirPtr);
    SetClipType(sym);
END_LOADS

void CharFaceServo::Enter() {
    RndPollable::Enter();
    mNeedScaleDown = true;
    mProceduralBlinkWeight = 0;
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
        mNeedScaleDown = true;
    }
}

void CharFaceServo::TryScaleDown() {
    if (mNeedScaleDown) {
        mNeedScaleDown = false;
        if (mBaseClip && !mClipType.Null()) {
            mBaseClip->ScaleDown(*this, 0.0f);
        }
        mBlinkWeightRight = 0.0f;
        mBlinkWeightLeft = 0.0f;
    }
}

void CharFaceServo::ApplyProceduralWeights() {
    if (mProceduralBlinkWeight > 0.0f && !mAppliedProceduralBlink) {
        TryScaleDown();
        if (mBlinkClipLeft) {
            mBlinkClipLeft->ScaleAdd(
                *this,
                Interp(0.0f, 1.0f - mBlinkWeightLeft, mProceduralBlinkWeight),
                mBlinkClipLeft->StartBeat(),
                0.0f
            );
        }
        if (mBlinkClipRight && mBlinkClipRight != mBlinkClipLeft) {
            mBlinkClipRight->ScaleAdd(
                *this,
                Interp(0.0f, 1.0f - mBlinkWeightRight, mProceduralBlinkWeight),
                mBlinkClipRight->StartBeat(),
                0.0f
            );
        }
        mAppliedProceduralBlink = true;
    }
}

void CharFaceServo::ScaleAdd(CharClip *clip, float weight, float f2, float f3) {
    if (!clip->Relative()) {
        MILO_NOTIFY_ONCE(
            "%s playing non-relative clip %s, cut it out!", PathName(this), PathName(clip)
        );
    } else {
        MILO_ASSERT(weight >= 0, 0x88);
        TryScaleDown();
        if (clip == mBlinkClipLeft || clip == mBlinkClipLeft2) {
            mBlinkWeightLeft += weight;
            mBlinkWeightLeft = Clamp(0.0f, 1.0f, mBlinkWeightLeft);
        } else if (clip == mBlinkClipRight || clip == mBlinkClipRight2) {
            mBlinkWeightRight += weight;
            mBlinkWeightRight = Clamp(0.0f, 1.0f, mBlinkWeightRight);
        }
        clip->ScaleAdd(*this, weight, f2, f3);
    }
}

void CharFaceServo::Poll() {
    START_AUTO_TIMER("faceservo");
    if (mBaseClip) {
        TryScaleDown();
        ScaleAddIdentity();
        mBaseClip->RotateBy(*this, mBaseClip->StartBeat());
        PoseMeshes();
    }
    mNeedScaleDown = true;
    mAppliedProceduralBlink = false;
}
