#include "gesture/SkeletonRecoverer.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/Ham.h"
#include "math/Vec.inl"
#include "obj/Task.h"
#include "utl/Std.h"
#include <float.h>

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

SkeletonRecoverer::TrackingIDHistory *SkeletonRecoverer::GetHistory(int trackingID) {
    FOREACH (it, mHistory) {
        if (it->mTrackingID == trackingID) {
            return &*it;
        }
    }
    return nullptr;
}

int SkeletonRecoverer::GetTrackingIDWithRecovery(int trackingID, int ignoreTrackingID) {
    Skeleton *skeleton = TheGestureMgr->GetSkeletonByTrackingID(trackingID);
    if (skeleton && skeleton->IsTracked()) {
        return trackingID;
    } else {
        TrackingIDHistory *history = GetHistory(trackingID);
        if (!history) {
            return 0;
        } else {
            int idx = -1;
            float f9 = FLT_MAX;
            for (int i = 0; i < NUM_SKELETONS; i++) {
                Skeleton &curSkel = TheGestureMgr->GetSkeleton(i);
                if (curSkel.TrackingState() != kSkeletonNotTracked
                    && curSkel.TrackingID() != ignoreTrackingID) {
                    Vector3 tmp;
                    Subtract(curSkel.Center(), history->mPos, tmp);
                    float dist = LengthSquared(tmp);
                    // float dist = DistanceSquared(curSkel.Center(), history->mPos);
                    if (dist < f9) {
                        f9 = dist;
                        idx = i;
                    }
                }
            }
            float dist = GestureMgr::GetMaxRecoveryDistance();
            if (idx != -1 && f9 <= dist * dist
                && history->mAge > GestureMgr::GetMinRecoveryTime()) {
                return TheGestureMgr->GetSkeleton(idx).TrackingID();
            } else {
                return history->mTrackingID;
            }
        }
    }
}

void SkeletonRecoverer::Poll() {
    float delta = TheTaskMgr.DeltaUISeconds();
    for (int i = 0; i < NUM_SKELETONS; i++) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(i);
        if (skeleton.IsTracked()) {
            TrackingIDHistory *history = GetHistory(skeleton.TrackingID());
            if (history) {
                history->mAge = 0;
                history->mPos = skeleton.Center();
            } else {
                TrackingIDHistory insertHistory;
                insertHistory.mAge = 0;
                insertHistory.mTrackingID = skeleton.TrackingID();
                insertHistory.mPos = skeleton.Center();
                mHistory.push_front(insertHistory);
            }
        }
    }
    for (auto it = mHistory.begin(); it != mHistory.end();) {
        TrackingIDHistory &cur = *it;
        if (cur.mAge > GestureMgr::GetMaxRecoveryTime()) {
            it = mHistory.erase(it);
        } else {
            ++it;
            if (!IsSkeletonTracked(cur.mTrackingID)) {
                cur.mAge += delta;
            }
        }
    }
}
