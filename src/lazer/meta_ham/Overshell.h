#pragma once
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "obj/Object.h"

enum OvershellSlotState {
    kJoinNoSkeleton = 0x0000,
    kJoinPending = 0x0001,
    kJoinSelDiff = 0x0002,
    kJoinPlaying = 0x0003
};

class OvershellSlot : public Hmx::Object {
public:
    OvershellSlot(HamPlayerData &);
    virtual ~OvershellSlot() {}
    virtual DataNode Handle(DataArray *, bool);

    void SetState(OvershellSlotState);
    void Init(int player) { mSlot = player; }
    void SetPlaying(bool);
    void Poll(const Skeleton *const (&)[6]);

private:
    const HamPlayerData &mPlayerData; // 0x2c
    OvershellSlotState mState; // 0x30
    int mSlot; // 0x34
    Skeleton *mSkeleton; // 0x38
    int mTrackingId; // 0x3c
};

class Overshell : public Hmx::Object {
public:
    Overshell();
    virtual ~Overshell();
    virtual DataNode Handle(DataArray *, bool);

    void Init();
    void ResolveSkeletons();
    void Poll(const Skeleton *const (&)[6]);

private:
    OvershellSlot *mSlots[2]; // 0x2c
};
