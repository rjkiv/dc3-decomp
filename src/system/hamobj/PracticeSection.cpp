#include "hamobj/PracticeSection.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/MoveDir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/Std.h"

PracticeSection::PracticeSection() : mDifficulty(kDifficultyEasy), mTestStepSequence(0) {}

PracticeSection::~PracticeSection() { DeleteAll(mSeqs); }

BEGIN_HANDLERS(PracticeSection)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(PracticeStep)
    SYNC_PROP_SET(type, o.mType, o.SetType(_val.Sym()))
    SYNC_PROP(start, o.mStart)
    SYNC_PROP(end, o.mEnd)
    SYNC_PROP(boundary, o.mBoundary)
    SYNC_PROP(name_override, o.mNameOverride)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(PracticeSection)
    SYNC_PROP(display_name, mDisplayName)
    SYNC_PROP_SET(difficulty, mDifficulty, mDifficulty = (Difficulty)_val.Int())
    SYNC_PROP(steps, mSteps)
    SYNC_PROP_MODIFY(
        test_step_sequence,
        mTestStepSequence,
        MinEq<int>(mTestStepSequence, mSeqs.size() - 1)
    )
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const PracticeStep &step) {
    bs << step.mType;
    bs << step.mStart;
    bs << step.mEnd;
    bs << step.mBoundary;
    bs << step.mNameOverride;
    return bs;
}

BEGIN_SAVES(PracticeSection)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mDisplayName;
    bs << mDifficulty;
    bs << mSteps;
    bs << mSeqs.size();
    for (int i = 0; i < mSeqs.size(); i++) {
        mSeqs[i]->Save(bs);
    }
    bs << mTestStepSequence;
END_SAVES

BEGIN_COPYS(PracticeSection)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY(PracticeSection)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDisplayName)
        COPY_MEMBER(mDifficulty)
        mSteps.clear();
        mSteps.insert(mSteps.begin(), c->mSteps.begin(), c->mSteps.end());
        COPY_MEMBER(mTestStepSequence)
        DeleteAll(mSeqs);
        for (int i = 0; i < c->mSeqs.size(); i++) {
            DancerSequence *curSeq = Hmx::Object::New<DancerSequence>();
            curSeq->Copy(c->mSeqs[i], ty);
            mSeqs.push_back(curSeq);
        }
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &bs, PracticeStep &step) {
    bs >> step.mType;
    bs >> step.mStart;
    bs >> step.mEnd;
    if (bs.rev > 0) {
        bs >> step.mBoundary;
    }
    if (bs.rev > 2) {
        bs >> step.mNameOverride;
    }
    if (step.mType != "learn") {
        step.mBoundary = true;
    }
    return bs;
}

BEGIN_LOADS(PracticeSection)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 1) {
        LOAD_SUPERCLASS(RndAnimatable)
    }
    bs >> mDisplayName;
    int diff;
    bs >> diff;
    mDifficulty = (Difficulty)diff;
    d >> mSteps;
    DeleteAll(mSeqs);
    if (d.rev > 1) {
        int numSeqs;
        bs >> numSeqs;
        for (int i = 0; i < numSeqs; i++) {
            DancerSequence *curSeq = Hmx::Object::New<DancerSequence>();
            curSeq->Load(bs);
            mSeqs.push_back(curSeq);
        }
        bs >> mTestStepSequence;
    }
END_LOADS

void PracticeSection::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    DancerSequence *seq;
    if (mTestStepSequence >= 0 && mTestStepSequence < mSeqs.size()) {
        seq = mSeqs[mTestStepSequence];
    } else {
        seq = nullptr;
    }
    if (seq) {
        seq->SetFrame(frame, blend);
        MoveDir *dir = dynamic_cast<MoveDir *>(Dir());
        if (dir) {
            dir->SetDancerSequence(seq);
        }
    }
}

float PracticeSection::StartFrame() {
    DancerSequence *seq;
    if (mTestStepSequence >= 0 && mTestStepSequence < mSeqs.size()) {
        seq = mSeqs[mTestStepSequence];
    } else {
        seq = nullptr;
    }
    if (seq) {
        return seq->StartFrame();
    } else
        return 0;
}

float PracticeSection::EndFrame() {
    DancerSequence *seq;
    if (mTestStepSequence >= 0 && mTestStepSequence < mSeqs.size()) {
        seq = mSeqs[mTestStepSequence];
    } else {
        seq = nullptr;
    }
    if (seq) {
        return seq->EndFrame();
    } else
        return 0;
}

const std::vector<PracticeStep> &PracticeSection::Steps() const { return mSteps; }
void PracticeSection::ClearSteps() { ClearAndShrink(mSteps); }
void PracticeSection::AddStep(PracticeStep step) { mSteps.push_back(step); }

DancerSequence *PracticeSection::SequenceForDetection(Symbol start, Symbol end) {
    int idx = 0;
    for (std::vector<PracticeStep>::iterator it = mSteps.begin(); it != mSteps.end();
         ++it, ++idx) {
        if (!it->mStart.Null() && !it->mEnd.Null()) {
            if (it->mStart == start && it->mEnd == end) {
                if (idx < mSeqs.size()) {
                    return mSeqs[idx];
                } else
                    return nullptr;
            }
        }
    }
    return nullptr;
}
