#include "char/CharIKMidi.h"
#include "char/Char.h"
#include "math/Easing.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "obj/Msg.h"
#include "obj/Task.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"

CharIKMidi::CharIKMidi()
    : mBone(this), mCurSpot(this), mNewSpot(this), mSpotChanged(0), mAnimBlender(this),
      mMaxAnimBlend(1), mAnimFracPerBeat(0), mAnimFrac(0) {
    Enter();
}

CharIKMidi::~CharIKMidi() {}

BEGIN_HANDLERS(CharIKMidi)
    HANDLE_ACTION(
        new_spot,
        NewSpot(Dir()->Find<RndTransformable>(_msg->Str(2), true), _msg->Float(3))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKMidi)
    SYNC_PROP(bone, mBone)
    SYNC_PROP(anim_blend_weightable, mAnimBlender)
    SYNC_PROP(anim_blend_max, mMaxAnimBlend)
    SYNC_PROP_SET(cur_spot, mCurSpot.Ptr(), NewSpot(_val.Obj<RndTransformable>(), 0))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKMidi)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBone;
    bs << mAnimBlender;
    bs << mMaxAnimBlend;
END_SAVES

BEGIN_COPYS(CharIKMidi)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharIKMidi)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBone)
        COPY_MEMBER(mAnimBlender)
        COPY_MEMBER(mMaxAnimBlend)
    END_COPYING_MEMBERS
END_COPYS

void CharIKMidi::Enter() {
    mCurSpot = nullptr;
    mNewSpot = nullptr;
    mSpotChanged = false;
    mFrac = 0;
    mFracPerBeat = 0;
    mLocalXfm.Reset();
    mOldLocalXfm.Reset();
    RndPollable::Enter();
}

void CharIKMidi::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mBone);
    changedBy.push_back(mBone);
    changedBy.push_back(mCurSpot);
}

void DoDebugDraws(CharIKMidi *mid, float f) {
    for (ObjDirItr<Hmx::Object> it(ObjectDir::Main(), false); it != nullptr; ++it) {
        MsgSinks *sinks = it->Sinks();
        if (sinks && sinks->HasSink(mid)) {
            static Message msg("debug_draw", 2.0f, 2.0f);
            msg[0] = f;
            msg[1] = TheTaskMgr.Beat();
            it->Handle(msg, false);
            return;
        }
    }
}

void CharIKMidi::NewSpot(RndTransformable *t, float f) {
    float f18 = f;
    if (mNewSpot != t) {
        if (!mNewSpot && f18 <= 0)
            mFracPerBeat = kHugeFloat;
        else {
            MaxEq(f18, 0.1f);
            mFracPerBeat = 1.0f / f18;
            if (f18 > 0.2) {
                mAnimFracPerBeat = mMaxAnimBlend / f18;
            }
        }
        mFrac = 0;
        mAnimFrac = 0;
        mSpotChanged = true;
        mNewSpot = t;
    }
}

void CharIKMidi::Poll() {
    if (mBone) {
        if (mSpotChanged) {
            mSpotChanged = false;
            if (!mCurSpot) {
                mCurSpot = mBone;
                mOldLocalXfm.Reset();
            }
            if (mNewSpot) {
                Transform tf48;
                FastInvert(mNewSpot->WorldXfm(), tf48);
                Multiply(mCurSpot->WorldXfm(), tf48, tf48);
                Multiply(mOldLocalXfm, tf48, mLocalXfm);
            }
            mCurSpot = mNewSpot;
        }
        if (mCurSpot) {
            mFrac += mFracPerBeat * TheTaskMgr.DeltaBeat();
            if (IsNaN(mFrac))
                mFrac = 0;
            ClampEq<float>(mFrac, 0, 1);
            float sigmoid = EaseSigmoid(mFrac, 0, 0);
            if (mAnimFracPerBeat > 0 && mAnimBlender) {
                if (mFrac < 0.5) {
                    mAnimFrac += mAnimFracPerBeat * TheTaskMgr.DeltaBeat();
                } else {
                    mAnimFrac -= mAnimFracPerBeat * TheTaskMgr.DeltaBeat();
                }
                ClampEq<float>(mAnimFrac, 0, mMaxAnimBlend);
                mAnimBlender->SetWeight(mAnimFrac);
            }
            Scale(mLocalXfm.v, 1.0f - sigmoid, mOldLocalXfm.v);
            Hmx::Quat q88(mLocalXfm.m);
            IdentityInterp(q88, sigmoid, q88);
            MakeRotMatrix(q88, mOldLocalXfm.m);
            Transform tf78;
            Multiply(mOldLocalXfm, mCurSpot->WorldXfm(), tf78);
            mBone->SetWorldXfm(tf78);
        }
    }
}

void CharIKMidi::Highlight() {
    if (gCharHighlightY == -1.0f) {
        CharDeferHighlight(this);
    } else {
        Hmx::Color white(1, 1, 1);
        Vector2 v2(5.0f, gCharHighlightY);
        TheRnd.DrawString(MakeString("%s:", PathName(this)), v2, white, true);
        v2.y += 16.0f;
        TheRnd.DrawString(
            MakeString("frac %.3f new:%s", mFrac, mCurSpot ? mCurSpot->Name() : "NULL"),
            v2,
            white,
            true
        );
        v2.y += 16.0f;
        DoDebugDraws(this, (v2.y + 24.0f) - 12.0f);
        gCharHighlightY += 112.0f;
    }
}
