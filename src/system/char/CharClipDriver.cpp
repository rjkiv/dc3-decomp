#include "char/CharClipDriver.h"
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "math/Easing.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "utl/Symbol.h"

CharClipDriver::CharClipDriver(
    Hmx::Object *owner,
    CharClip *clip,
    int playFlags,
    float blendwidth,
    CharClipDriver *next,
    float startBeat,
    float deltaStart,
    bool playMultiple
)
    : mPlayFlags(clip->PlayFlags()), mBlendWidth(blendwidth), mTimeScale(1.0f), mDBeat(0),
      mAdvanceBeat(0), mClip(owner, clip), mNext(next), mNextEvent(-1),
      mPlayMultipleClips(playMultiple) {
    if (playFlags & 0xF0U)
        CharClip::SetDefaultLoopFlag(mPlayFlags, playFlags & 0xF0U);
    if (playFlags & 0xFU)
        CharClip::SetDefaultBlendFlag(mPlayFlags, playFlags & 0xFU);
    if (playFlags & 0xF600)
        CharClip::SetDefaultBeatAlignModeFlag(mPlayFlags, playFlags & 0xF600U);
    while (mNext && mNext->mBlendFrac == 0) {
        mNext = mNext->Exit(false);
    }
    if (startBeat != kHugeFloat) {
        mBeat = startBeat;
        mRampIn = deltaStart;
        mBlendFrac = 0;
    } else {
        if (mNext && (mPlayFlags & 0xF) == 2) {
            mNext = mNext->Exit(true);
        }
        const CharGraphNode *gNode;
        if (mNext
            && (gNode =
                    mNext->mClip->FindNode(mClip, mNext->mBeat, mPlayFlags, mBlendWidth),
                gNode)) {
            mBeat = gNode->nextBeat;
            mRampIn = gNode->curBeat - mNext->mBeat;
            mBlendFrac = 0;
        } else {
            mBeat = clip->StartBeat();
            mRampIn = 0;
            mBlendFrac = 1;
        }
    }
    if ((mPlayFlags & 0xF) == 2) {
        mPlayFlags |= 0x80;
    }
    if (mPlayMultipleClips || (mPlayFlags & 0xF) == 8) {
        mBlendFrac = 1e-06f;
    }
    if (mBlendFrac == 1) {
        if (mClip->Range() > 0) {
            float rand = RandomFloat(0, mClip->Range());
            mBeat = ModRange(
                mClip->StartBeat(),
                (mClip->EndBeat() + mClip->StartBeat()) / 2,
                mBeat + rand
            );
        }
    }
    mWeight = 0;
}

CharClipDriver::CharClipDriver(Hmx::Object *owner, const CharClipDriver &driver)
    : mClip(owner, driver.mClip) {
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
        mNext = new CharClipDriver(owner, *driver.mNext);
    else
        mNext = nullptr;
}

void CharClipDriver::ScaleAdd(CharBones &bones, float weight) {
    if (weight != 0) {
        mWeight = EaseSigmoid(mBlendFrac, 0, 0) * weight;
        bones.ScaleAdd(mClip, mWeight, mBeat, mDBeat);

        if (mPlayMultipleClips) {
            if (mNext) {
                mNext->ScaleAdd(bones, weight);
            }
        } else if (mNext) {
            mNext->ScaleAdd(bones, weight - mWeight);
        }
    }
}

void CharClipDriver::RotateTo(CharBones &bones, float weight) {
    if (weight != 0) {
        mWeight = EaseSigmoid(mBlendFrac, 0, 0) * weight;
        mClip->RotateTo(bones, mWeight, mBeat);
        if (mNext) {
            mNext->RotateTo(bones, weight - mWeight);
        }
    }
}

int CharClipDriver::NumBeatEvents() const { return mClip ? mClip->NumBeatEvents() : 0; }

void CharClipDriver::DeleteStack() {
    if (mNext) {
        mNext->DeleteStack();
    }
    delete this;
}

CharClipDriver *CharClipDriver::Exit(bool stack) {
    static Symbol exit("exit");
    if (stack && mNext) {
        mNext = mNext->Exit(stack);
    }
    CharClipDriver *ret = mNext;
    ExecuteEvent(exit);
    RndAnimatable *syncAnim = mClip->SyncAnim();
    if (syncAnim) {
        syncAnim->EndAnim();
    }
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

float CharClipDriver::AlignToBeat(float beat) {
    float flag = (mPlayFlags >> 0xC) & 0xF;
    float ret = 0;
    if (flag != 0 && mTimeScale == 1 && (mPlayFlags & 0xF0) != 0x20) {
        ret = Mod(beat - mBeat, flag);
        if (ret > flag / 2) {
            ret -= flag;
            if (ret + mBeat < mClip->StartBeat()) {
                ret += flag;
            }
        }
    }
    return ret;
}

void CharClipDriver::PlayEvents(float oldBeat) {
    if (mNextEvent == -1) {
        RndAnimatable *anim = mClip->SyncAnim();
        if (anim) {
            anim->StartAnim();
        }
        static Symbol enter("enter");
        ExecuteEvent(enter);
        mNextEvent = 0;
    }
    for (; mNextEvent < mClip->BeatEvents().size(); mNextEvent++) {
        CharClip *clip = mClip;
        if (clip->BeatEvents()[mNextEvent].beat <= mBeat) {
            ExecuteEvent(clip->BeatEvents()[mNextEvent].event);
        } else {
            return;
        }
    }
}

void CharClipDriver::ExecuteEvent(Symbol handler) {
    if (!handler.Null()) {
        static Symbol clip_event("clip_event");
        Hmx::Object *clipOwnerDir = mClip.RefOwner()->Dir();
        static Message h(clip_event, 0, 0, 0);
        h[0] = handler;
        h[1] = mClip.Ptr();
        clipOwnerDir->Export(h, true);
    }
}

void CharClipDriver::SetBeatOffset(float offset, TaskUnits units, Symbol beatEvent) {
    if (offset != 0 && mClip) {
        mBeat = mClip->StartBeat();
        if (!beatEvent.Null()) {
            int i = 0;
            for (; i < mClip->BeatEvents().size(); i++) {
                if (mClip->BeatEvents()[i].event == beatEvent) {
                    mBeat = mClip->BeatEvents()[i].beat;
                    break;
                }
            }
            if (i == mClip->BeatEvents().size()) {
                MILO_NOTIFY("%s could not find event %s", PathName(mClip), beatEvent);
            }
        }
        if (units != kTaskBeats) {
            offset = mClip->DeltaSecondsToDeltaBeat(offset, mBeat);
        }
        mBeat += offset;
    }
}

float CharClipDriver::Evaluate(float beat, float f2, float f3) {
    float mult = 0;
    if (mNext) {
        mult = mNext->Evaluate(beat, f2, f3);
    }
    if ((mPlayFlags & 0xF0) == 0x20) {
        if (mBeat > mClip->EndBeat()) {
            float len = mClip->LengthBeats();
            mBeat = len > 0 ? mClip->StartBeat() + fmodf(mClip->EndBeat() - mBeat, len)
                            : mClip->StartBeat();
            mBeat += AlignToBeat(beat);
            mNextEvent = 0;
        } else if (mBeat < mClip->StartBeat()) {
            float len = mClip->LengthBeats();
            mBeat = len > 0 ? mClip->EndBeat() - fmodf(mClip->StartBeat() - mBeat, len)
                            : mClip->StartBeat();
            mBeat += AlignToBeat(beat);
            mNextEvent = mClip->NumBeatEvents();
        }
    }
    float sigmoid = EaseSigmoid(mBlendFrac, 0, 0);
    return (1 - sigmoid) * mult + sigmoid;
}

CharClipDriver *CharClipDriver::PreEvaluate(float beat, float dbeat, float dt) {
    MILO_ASSERT(mBlendFrac >= 0, 0xAB);
    if (mBlendWidth < 0) {
        MILO_NOTIFY("CharClipDriver: blend width < 0 with clip %s", mClip->Name());
        mBlendWidth = 0;
    }
    if (mNext) {
        mNext = mNext->PreEvaluate(beat, dbeat, dt);
    }
    bool flag9 = (mPlayFlags >> 9) & 1;
    bool flag10 = (mPlayFlags >> 10) & 1;
    float f12;
    if (!mPlayMultipleClips && mNext) {
        f12 = mNext->mAdvanceBeat;
    } else if (flag9) {
        f12 = dt;
    } else {
        f12 = dbeat;
    }
    if (mRampIn > 0 || f12 > 0) {
        mRampIn -= f12;
    }
    if (mRampIn >= 0) {
        mDBeat = 0;
        mBlendFrac = 0;
    } else {
        float oldBeat = mBeat;
        float f8 = dbeat;
        if (mPlayFlags & 0x80) {
            mDBeat = 0;
            mPlayFlags &= 0xFFFFFF7F;
        } else if (!flag10) {
            if (flag9) {
                dbeat = mClip->DeltaSecondsToDeltaBeat(dt, mBeat);
            }
            mDBeat = mTimeScale * dbeat;
        }
        mBeat += mDBeat;
        float align = AlignToBeat(beat);
        mBeat += align;
        mAdvanceBeat += align;
        PlayEvents(oldBeat);
        RndAnimatable *anim = mClip->SyncAnim();
        if (anim) {
            anim->SetFrame(mClip->BeatToFrame(mBeat), 1);
        }
        if (mBlendFrac < 1) {
            if (mBlendWidth > 0) {
                if (!flag10) {
                    f8 = mDBeat;
                }
                float f9 = f8 / mBlendWidth;
                if (f9 > 0) {
                    mBlendFrac += f9;
                }
            } else {
                mBlendFrac = 1;
            }
            MinEq(mBlendFrac, 1.0f);
        }
    }

    if (!mPlayMultipleClips) {
        if (mNext && mBlendFrac == 1) {
            mNext = mNext->Exit(true);
        }
        return this;
    } else if (mBeat > mClip->EndBeat()) {
        return Exit(false);
    } else {
        return this;
    }
}
