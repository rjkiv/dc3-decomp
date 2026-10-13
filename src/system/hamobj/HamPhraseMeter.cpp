#include "hamobj/HamPhraseMeter.h"
#include "ScoreUtl.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Dir.h"
#include "utl/BinStream.h"

HamPhraseMeter::HamPhraseMeter()
    : mAnim(this), mRatingFrac(0), mRating("move_bad"), mDesiredFPB(480),
      mFirstPerfectFrame(1920), unk220(0), mPlayerIndex(0) {}

BEGIN_HANDLERS(HamPhraseMeter)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_PROPSYNCS(HamPhraseMeter)
    SYNC_PROP(anim, mAnim)
    SYNC_PROP_MODIFY(rating_frac, mRatingFrac, SetRatingFrac(mRatingFrac, kHugeFloat))
    SYNC_PROP_MODIFY(
        rating, mRating, SetRatingFrac(RatingToRatingFrac(mRating), kHugeFloat)
    )
    SYNC_PROP(desired_fpb, mDesiredFPB)
    SYNC_PROP(first_perfect_frame, mFirstPerfectFrame)
    SYNC_PROP(player_index, mPlayerIndex)
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(HamPhraseMeter)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(RndDir)
    bs << mAnim;
    bs << mRatingFrac;
    bs << mRating;
    bs << mDesiredFPB;
    bs << mFirstPerfectFrame;
    if (IsProxy()) {
        bs << mPlayerIndex;
    }
END_SAVES

BEGIN_COPYS(HamPhraseMeter)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY(HamPhraseMeter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAnim)
        COPY_MEMBER(mRatingFrac)
        COPY_MEMBER(mRating)
        COPY_MEMBER(mDesiredFPB)
        COPY_MEMBER(mFirstPerfectFrame)
        COPY_MEMBER(mPlayerIndex)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

void HamPhraseMeter::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    RndDir::PreLoad(d);
    PUSH_REVS(d);
}

void HamPhraseMeter::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    RndDir::PostLoad(d);
    mAnim.Load(d, false, this);
    d >> mRatingFrac;
    d >> mRating;
    d >> mDesiredFPB;
    d >> mFirstPerfectFrame;
    if (d.rev > 2 && IsProxy()) {
        d >> mPlayerIndex;
    }
}

void HamPhraseMeter::Poll() {
    RndDir::Poll();
    if (mAnim) {
        float frame = mAnim->GetFrame();
        float delta = TheTaskMgr.DeltaBeat() * unk224;
        if (frame < unk220) {
            frame = Min(delta + frame, unk220);
        } else if (frame > unk220) {
            frame = Max(frame - delta, unk220);
        }
        mAnim->SetFrame(frame, 1);
    }
}

void HamPhraseMeter::Enter() {
    RndDir::Enter();
    SetRatingFrac(0, -1);
    unk224 = mDesiredFPB;
}

void HamPhraseMeter::SetRatingFrac(float f1, float f2) {
    if (mAnim) {
        if (f1 == 1) {
            unk220 = mAnim->EndFrame();
        } else {
            unk220 = (mFirstPerfectFrame - mAnim->StartFrame()) * f1;
        }
        if (f2 <= 0) {
            mAnim->SetFrame(unk220, 1);
        } else {
            float div = std::fabs(unk220 - mAnim->GetFrame());
            unk224 = Max<float>(mDesiredFPB, div / f2);
        }
    }
}
