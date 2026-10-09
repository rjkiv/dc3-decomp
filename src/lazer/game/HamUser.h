#pragma once
#include "os/User.h"

class HamUser : public virtual LocalUser {
public:
    // Hmx::Object
    virtual ~HamUser();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // LocalUser
    virtual void Reset() { User::Reset(); }
    virtual int GetPadNum() const;
    virtual bool CanSaveData() const;

    static HamUser *NewHamUser(int);

    bool HasAsFriend(HamUser *) const;
    bool CanGetAchievements() const { return CanSaveData(); }

protected:
    HamUser(int);

    int mSkeletonIndex; // 0x4
    int mEnrollmentIndex; // 0x8
};
