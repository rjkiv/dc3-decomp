#pragma once
#include "math/Vec.h"
#include "types.h"
#include <list>

class SkeletonRecoverer {
public:
    struct TrackingIDHistory {
        int mTrackingID; // 0x0
        Vector3 mPos; // 0x4
        float mAge; // 0x14
    };
    SkeletonRecoverer();
    virtual ~SkeletonRecoverer();

    bool WaitingToRecover();
    int GetTrackingIDWithRecovery(int, int);
    void Poll();

protected:
    std::list<TrackingIDHistory> mHistory; // 0x4

private:
    bool IsSkeletonTracked(int) const;
};
