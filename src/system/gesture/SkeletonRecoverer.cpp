#include "gesture/SkeletonRecoverer.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "obj/Task.h"
#include "utl/Std.h"

SkeletonRecoverer::SkeletonRecoverer() {}

SkeletonRecoverer::~SkeletonRecoverer() {}

bool SkeletonRecoverer::IsSkeletonTracked(int id) const {
    for (int i = 0; i < 6; i++) {
        if (TheGestureMgr->GetSkeleton(i).TrackingID() == id) {
            if (TheGestureMgr->GetSkeleton(i).IsTracked())
                return true;
        }
    }
    return false;
}

bool SkeletonRecoverer::WaitingToRecover() {
    FOREACH (it, mIDHistory) {
        if (0 < it->unk14) {
            return true;
        }
    }
    return false;
}
