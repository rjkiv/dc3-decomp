#pragma once
#include "obj/Object.h"
#include "os/User.h"

class UserMgr : public Hmx::Object {
protected:
    bool unk2c;

public:
    UserMgr();
    virtual ~UserMgr() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual void GetUsers(std::vector<User *> &) const = 0;
    virtual User *GetUser(const UserGuid &, bool) const = 0;
    virtual LocalUser *GetLocalUser(const UserGuid &, bool) const { return nullptr; }
    virtual RemoteUser *GetRemoteUser(const UserGuid &, bool) const { return nullptr; }

    void SetBool(bool b) { unk2c = b; } // change later

    void GetLocalUsers(std::vector<LocalUser *> &) const;
    // void GetRemoteUsers(std::vector<RemoteUser *> &) const; // goes unused in DC3
    LocalUser *GetLocalUserFromPadNum(int) const;
};

void SetTheUserMgr(UserMgr *);
extern UserMgr *TheUserMgr;
