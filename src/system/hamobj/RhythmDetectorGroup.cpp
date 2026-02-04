#include "RhythmDetectorGroup.h"
#include "hamobj/RhythmDetector.h"

RhythmDetectorGroup::RhythmDetectorGroup()
    : mDetectors(this, (EraseMode)0), mRating(0.0), mFreshness(0.0), mSkeletonIndex(-1),
      mDebugGraph(nullptr) {}

RhythmDetectorGroup::~RhythmDetectorGroup() {}

BEGIN_HANDLERS(RhythmDetectorGroup)
    HANDLE_ACTION(add_debug_graphs, AddDebugGraphs())
    HANDLE_ACTION(remove_debug_graphs, RemoveDebugGraphs())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RhythmDetectorGroup)
    SYNC_PROP(detectors, mDetectors)
    SYNC_PROP(rating, mRating)
    SYNC_PROP(freshness, mFreshness)
    SYNC_PROP(skeleton_index, mSkeletonIndex)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RhythmDetectorGroup)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(RndPollable)
    bs << mDetectors;
    bs << mSkeletonIndex;
END_SAVES

BEGIN_COPYS(RhythmDetectorGroup)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RhythmDetectorGroup)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDetectors)
        COPY_MEMBER(mRating)
        COPY_MEMBER(mFreshness)
        COPY_MEMBER(mSkeletonIndex)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RhythmDetectorGroup)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(RndPollable)
    if (d.rev >= 1) {
        d >> mDetectors;
    }
    if (d.rev >= 2) {
        d >> mSkeletonIndex;
        if (mSkeletonIndex > -1) {
            SetSkeletonIndex(mSkeletonIndex);
        }
    }
END_LOADS

void RhythmDetectorGroup::Poll() {
    if (!TheLoadMgr.EditMode()) {
        mRating = 0;
        mFreshness = 0;
        FOREACH (it, mDetectors) {
            RhythmDetector *cur = *it;
            mRating = Max(mRating, cur->Groove());
            mFreshness = Max(mFreshness, cur->Freshness());
        }
        if (mDebugGraph) {
            mDebugGraph->AddData(mRating, false);
            mDebugGraph->Draw();
        }
    }
}

void RhythmDetectorGroup::SetSkeletonIndex(int skelIdx) {
    FOREACH (it, mDetectors) {
        (*it)->SetSkeletonID(skelIdx);
    }
}

void RhythmDetectorGroup::RemoveDebugGraphs() {
    RELEASE(mDebugGraph);
    FOREACH (it, mDetectors) {
        (*it)->RemoveDebugGraphs();
    }
}

void RhythmDetectorGroup::AddDebugGraphs() {
    float f6 = 1.0f / (float)(mDetectors.size() + 1);
    float f7 = f6 * 0.9f;
    delete mDebugGraph;
    mDebugGraph = new DebugGraph(
        0.1f,
        0.0f,
        0.8f,
        0.9f,
        Hmx::Color(0.4, 0.4, 0.4, 0.8),
        Hmx::Color(0.4, 0.4, 0.4, 0.8),
        null,
        0.0,
        2.0,
        ""
    );
    mDebugGraph->SetUnk44(1);
    float f10 = f6;
    FOREACH (it, mDetectors) {
        RhythmDetector *cur = *it;
        cur->RemoveDebugGraphs();
        cur->AddDebugGraph(0.1f, f10, 0.8f, 0.1f / 0.8f, Hmx::Color(1, 0, 1, 0));
        f10 += f6;
    }
}
