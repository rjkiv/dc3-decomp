#pragma once
#include <list>

class SkeletonRecoverer {
public:
    struct TrackingIDHistory {
        int unk0;
    };
    SkeletonRecoverer();
    virtual ~SkeletonRecoverer();

    bool WaitingToRecover();
    int GetTrackingIDWithRecovery(int, int);

protected:
    std::list<TrackingIDHistory> mIDHistory; // 0x4

private:
    bool IsSkeletonTracked(int) const;
};
