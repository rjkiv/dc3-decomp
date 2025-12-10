#pragma once
#include "os/User.h"

class HamUser : virtual public LocalUser {
public:
    virtual ~HamUser();
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual class DataNode Handle(DataArray *, bool);
    virtual void Reset();
    virtual int GetPadNum() const;
    virtual bool CanSaveData() const;

    static HamUser *NewHamUser(int);

    bool HasAsFriend(HamUser *) const;

    int unk4;
    int unk8;

protected:
    HamUser(int);
};
