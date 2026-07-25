#pragma once
#include "types.h"
#include <list>

class SkeletonRecoverer {
public:
    struct TrackingIDHistory {
        int unk0;
        float unk4;
        float unk8;
        float unkc;
        float unk10;
        float unk14;
    };
    SkeletonRecoverer();
    virtual ~SkeletonRecoverer();

    bool WaitingToRecover();
    int GetTrackingIDWithRecovery(int, int);
    void Poll();

protected:
    std::list<TrackingIDHistory> mIDHistory; // 0x4

private:
    bool IsSkeletonTracked(int) const;
};
