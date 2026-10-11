#include "gesture/SkeletonRecoverer.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/Ham.h"
#include "obj/Task.h"
#include "utl/Std.h"

SkeletonRecoverer::SkeletonRecoverer() {}

SkeletonRecoverer::~SkeletonRecoverer() {}

bool SkeletonRecoverer::IsSkeletonTracked(int id) const {
    for (int i = 0; i < NUM_SKELETONS; i++) {
        if (TheGestureMgr->GetSkeleton(i).TrackingID() == id) {
            if (TheGestureMgr->GetSkeleton(i).IsTracked())
                return true;
        }
    }
    return false;
}

bool SkeletonRecoverer::WaitingToRecover() {
    FOREACH (it, mHistory) {
        if (0 < it->mAge) {
            return true;
        }
    }
    return false;
}
