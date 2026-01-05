#include "RhythmDetectorGroup.h"

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

BEGIN_LOADS(RhythmDetectorGroup)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(RndPollable)
    bs >> mDetectors;
    bs >> mSkeletonIndex;
    if (mSkeletonIndex > -1) {
        SetSkeletonIndex(mSkeletonIndex);
    }
END_LOADS

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

RhythmDetectorGroup::~RhythmDetectorGroup() {}

RhythmDetectorGroup::RhythmDetectorGroup() : mDetectors(this, static_cast<EraseMode>(0), kObjListNoNull), mRating(0.0), mFreshness(0.0), mSkeletonIndex(-1), mDebugGraph(nullptr) {}

void RhythmDetectorGroup::Poll() {
    if (TheLoadMgr.EditMode() == false) {
        mRating = 0.0;
        mFreshness = 0.0;
        FOREACH(it, mDetectors) {
            float groove = (*it)->Groove();
            float temp = mRating;
            if (temp - groove < 0.0) {
                temp = groove;
            }
            mRating = temp;
            float freshness = (*it)->Freshness();
            float temp2 = mFreshness;
            if (temp2 - freshness < 0.0) {
                temp2 = freshness;
            }
            mFreshness = temp2;
        }
        if (mDebugGraph) {
            mDebugGraph->AddData(mRating, mFreshness);
            mDebugGraph->Draw();
        }
    }
}

void RhythmDetectorGroup::SetSkeletonIndex(int skelIdx) {
    FOREACH(it, mDetectors) {
        (*it)->SetSkeletonID(skelIdx);
    }
}

void RhythmDetectorGroup::RemoveDebugGraphs() {
    delete mDebugGraph;
    mDebugGraph = nullptr;
    FOREACH(it, mDetectors) {
        (*it)->RemoveDebugGraphs();
    }
}

void RhythmDetectorGroup::AddDebugGraphs() {
    delete mDebugGraph;
    mDebugGraph = new DebugGraph(0.1f, 0.0f, 0.8f, 0.9f, Hmx::Color(0.4, 0.4, 0.4, 0.8), Hmx::Color(0.4, 0.4, 0.4, 0.8), null, 0.0, 2.0, "");
    FOREACH(it, mDetectors) {
        (*it)->RemoveDebugGraphs();
        (*it)->AddDebugGraph(0.1f, 0.1f, 0.8f, 0.1f / 0.8f, Hmx::Color(0.4, 0.4, 0.4, 0.8));
    }
}