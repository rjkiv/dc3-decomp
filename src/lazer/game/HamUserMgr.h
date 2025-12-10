#pragma once
#include "game/HamUser.h"
#include "obj/Data.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "stl/_vector.h"
#include "utl/HxGuid.h"

class HamUserMgr : public UserMgr {
public:
    // Hmx::Object
    virtual ~HamUserMgr();
    virtual DataNode Handle(DataArray *, bool);

    // UserMgr
    virtual void GetUsers(std::vector<User *> &) const;
    virtual User *GetUser(UserGuid const &, bool) const;
    virtual LocalUser *GetLocalUser(UserGuid const &, bool) const;
    virtual RemoteUser *GetRemoteUser(UserGuid const &, bool) const;

    std::vector<HamUser *> unk30;

    HamUserMgr(int);
    HamUser *GetActiveUser() const;
    HamUser *GetHamUser(UserGuid const &, bool) const;
    HamUser *GetUserFromPad(int) const;
    DataNode ForEachUser(DataArray const *);
};

extern HamUserMgr *TheHamUserMgr;

void HamUserMgrInit(bool);
void HamUserMgrTerminate();
