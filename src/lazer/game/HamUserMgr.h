#pragma once
#include "game/HamUser.h"
#include "obj/Data.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "utl/HxGuid.h"

class HamUserMgr : public UserMgr {
public:
    HamUserMgr(int);
    // Hmx::Object
    virtual ~HamUserMgr();
    virtual DataNode Handle(DataArray *, bool);

    // UserMgr
    virtual void GetUsers(std::vector<User *> &) const;
    virtual User *GetUser(const UserGuid &, bool) const;
    virtual LocalUser *GetLocalUser(const UserGuid &, bool) const;
    virtual RemoteUser *GetRemoteUser(const UserGuid &, bool) const;

    HamUser *GetActiveUser() const;
    HamUser *GetHamUser(const UserGuid &, bool) const;
    HamUser *GetUserFromPad(int) const;
    DataNode ForEachUser(DataArray const *);

private:
    std::vector<HamUser *> mUsers; // 0x30
};

extern HamUserMgr *TheHamUserMgr;

void HamUserMgrInit(bool);
void HamUserMgrTerminate();
