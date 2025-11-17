#include "char/CharClipDriver.h"
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "math/Rand.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/Symbol.h"

CharClipDriver::CharClipDriver(
    Hmx::Object *owner,
    CharClip *clip,
    int mask,
    float blendwidth,
    CharClipDriver *next,
    float f2,
    float f3,
    bool multclips
)
    : mPlayFlags(clip->PlayFlags()), mBlendWidth(blendwidth), mTimeScale(1.0f), mDBeat(0),
      mAdvanceBeat(0), mClip(owner, clip), mNext(next), mNextEvent(-1),
      mPlayMultipleClips(multclips) {
    if (mask & 0xF0U)
        CharClip::SetDefaultLoopFlag(mPlayFlags, mask & 0xF0U);
    if (mask & 0xFU)
        CharClip::SetDefaultBlendFlag(mPlayFlags, mask & 0xFU);
    if (mask & 0xF600U)
        CharClip::SetDefaultBeatAlignModeFlag(mPlayFlags, mask & 0xF600U);
    while (mNext && mNext->mBlendFrac == 0) {
        mNext = mNext->Exit(false);
    }
    if (f2 != kHugeFloat) {
        mBeat = f2;
        mRampIn = f3;
        mBlendFrac = 0;
    } else {
        if (mNext && (mPlayFlags & 0xF) == 2) {
            mNext = mNext->Exit(true);
        }
        if (mNext) {
            const CharGraphNode *gNode =
                mNext->mClip->FindNode(mClip, mNext->mBeat, mPlayFlags, mBlendWidth);
            if (gNode) {
                mBeat = gNode->nextBeat;
                float cur = gNode->curBeat;
                float nextBeat = mNext->mBeat;
                mRampIn = cur - nextBeat;
                mBlendFrac = 0;
                goto next;
            }
        }
        mBeat = clip->StartBeat();
        mRampIn = 0;
        mBlendFrac = 1;
    }
next:
    if (mPlayMultipleClips || (mPlayFlags & 0xF) == 8) {
        mBlendFrac = 1e-06f;
    }
    if (mBlendFrac == 1) {
        if (mClip->Range() > 0) {
            float f7 = RandomFloat(0, mClip->Range());
            float f10 = mClip->EndBeat() + mClip->StartBeat();
            f10 /= 2.0f;
            float f8 = mClip->StartBeat();
            mBeat = ModRange(f8, f10, mBeat + f7);
        }
    }
    mWeight = 0;
}

CharClipDriver::CharClipDriver(Hmx::Object *o, const CharClipDriver &driver)
    : mClip(o, driver.mClip) {
    mPlayFlags = driver.mPlayFlags;
    mBlendWidth = driver.mBlendWidth;
    mTimeScale = driver.mTimeScale;
    mRampIn = driver.mRampIn;
    mBeat = driver.mBeat;
    mDBeat = driver.mDBeat;
    mBlendFrac = driver.mBlendFrac;
    mAdvanceBeat = driver.mAdvanceBeat;
    mWeight = driver.mWeight;
    mNextEvent = driver.mNextEvent;
    mEventData = driver.mEventData;
    if (driver.mNext)
        mNext = new CharClipDriver(o, *driver.mNext);
    else
        mNext = nullptr;
}

void CharClipDriver::ScaleAdd(CharBones &bones, float f) {}

void CharClipDriver::RotateTo(CharBones &bones, float f) {}

int CharClipDriver::NumBeatEvents() const {
    if (mClip)
        return mClip->NumBeatEvents();
    return 0;
}

void CharClipDriver::DeleteStack() {
    if (mNext)
        mNext->DeleteStack();
    delete this;
}

CharClipDriver *CharClipDriver::Exit(bool b) {
    static Symbol exit("exit");
    if (b && mNext) {
        mNext = mNext->Exit(b);
    }
    CharClipDriver *ret = mNext;
    ExecuteEvent(exit);
    RndAnimatable *syncAnim = mClip->SyncAnim();
    if (syncAnim)
        syncAnim->EndAnim();
    delete this;
    return ret;
}

CharClipDriver *CharClipDriver::DeleteRef(ObjRef *ref, bool &b) {
    if (&mClip == ref) {
        b = true;
        return Exit(false);
    } else if (mNext) {
        mNext = mNext->DeleteRef(ref, b);
    }
    return this;
}
