#include "hamobj/HamIconMan.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/SongUtl.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"

HamIconMan::HamIconMan()
    : mTexture(this), mStartBeat(0), mEndBeat(0), mOffset(-0.75),
      mDifficulty(kDifficultyExpert), mMoveName(""), mCharClip(nullptr), mBPMOverride(0),
      mPlayIntroTransition(1) {
    SetRate(k1_fpb);
}

HamIconMan::~HamIconMan() {}

BEGIN_HANDLERS(HamIconMan)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamIconMan)
    SYNC_PROP(texture, mTexture)
    SYNC_PROP(start_beat, mStartBeat)
    SYNC_PROP(end_beat, mEndBeat)
    SYNC_PROP(move_name, mMoveName)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(char_clip, mCharClip)
    SYNC_PROP(bpm_override, mBPMOverride)
    SYNC_PROP(play_intro_transition, mPlayIntroTransition)
    SYNC_PROP_SET(difficulty, mDifficulty, mDifficulty = (Difficulty)_val.Int())
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamIconMan)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mTexture;
    bs << mStartBeat;
    bs << mEndBeat;
    bs << mOffset;
    bs << mDifficulty;
END_SAVES

BEGIN_COPYS(HamIconMan)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(HamIconMan)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTexture)
        COPY_MEMBER(mStartBeat)
        COPY_MEMBER(mEndBeat)
        COPY_MEMBER(mOffset)
        COPY_MEMBER(mDifficulty)
        COPY_MEMBER(mMoveName)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(HamIconMan)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mTexture;
    d >> mStartBeat >> mEndBeat >> mOffset;
    if (d.rev > 0) {
        int diff;
        d >> diff;
        mDifficulty = (Difficulty)diff;
    }
END_LOADS

void HamIconMan::DrawShowing() {
    if (TheHamDirector) {
        if (mCharClip) {
            float beat;
            if (mBPMOverride > 0) {
                beat = TheTaskMgr.UISeconds() * (mBPMOverride * 0.016666668f);
            } else {
                beat = FrameToBeat(TheHamDirector->SongAnim(0)->GetFrame());
            }
            beat = (float)fmod(beat, 4.0f) + 1.0f;
            if (mPlayIntroTransition && beat > 4.5f) {
                beat -= 4.0f;
            }
            TheHamDirector->PoseIconMan(
                mCharClip, mCharClip->StartBeat() + beat, mTexture, true, nullptr, 0, 0
            );
        } else {
            float clamp = Clamp(0.0f, mEndBeat - mStartBeat, GetFrame());
            float beat = mStartBeat + clamp;
            if (mMoveName != "") {
                TheHamDirector->DrawIconMan(
                    mMoveName, mMoveName, mMoveName, clamp, mOffset, mTexture
                );
            } else {
                TheHamDirector->DrawIconMan(
                    mDifficulty, beat, mStartBeat, mEndBeat - mStartBeat, mOffset, mTexture
                );
            }
        }
    }
}
