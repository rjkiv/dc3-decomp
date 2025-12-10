#include "lazer/game/HamUserMgr.h"
#include "game/HamUser.h"
#include "macros.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "stl/_vector.h"
#include "utl/HxGuid.h"
#include "utl/Std.h"

HamUserMgr *TheHamUserMgr;

HamUserMgr::HamUserMgr(int size) {
    unk30.reserve(size);
    for (int i = 0; i < size; i++) {
        HamUser *h = HamUser::NewHamUser(i);
        unk30.push_back(h);
    }
}

HamUserMgr::~HamUserMgr() {}

HamUser *HamUserMgr::GetActiveUser() const {
    MILO_NOTIFY(
        "Are you sure you should be calling GetActiveUser()??  You probably want GetActiveProfile()!"
    );
    // need TheProfileMgr to finish this func
    return nullptr;
}

HamUser *HamUserMgr::GetUserFromPad(int i) const {
    FOREACH (it, unk30) {
        HamUser *user = *it;
        MILO_ASSERT(user, 0xa4);
        if (user->GetPadNum() == i)
            return user;
    }
    return nullptr;
}

RemoteUser *HamUserMgr::GetRemoteUser(UserGuid const &, bool) const {
    MILO_FAIL("Called HamUserMgr::GetRemoteUser");
    return nullptr;
}

User *HamUserMgr::GetUser(UserGuid const &ug, bool b) const {
    HamUser *user = GetHamUser(ug, b);
    if (user)
        return user;
}

LocalUser *HamUserMgr::GetLocalUser(UserGuid const &ug, bool b) const {
    HamUser *user = GetHamUser(ug, b);
    if (user)
        return user;
}

void HamUserMgr::GetUsers(std::vector<User *> &users) const {
    users.clear();
    for (int i = 0; i < unk30.size(); i++) {
        users.push_back(unk30[i]);
    }
}

BEGIN_HANDLERS(HamUserMgr)
    HANDLE_EXPR(foreach_user, ForEachUser(_msg))
    HANDLE_EXPR(get_active_user, GetActiveUser())
    HANDLE_SUPERCLASS(UserMgr)
END_HANDLERS

void HamUserMgrInit(bool b) {
    MILO_ASSERT(!TheHamUserMgr, 0x20);
    TheHamUserMgr = new HamUserMgr(8);
    TheHamUserMgr->SetBool(b);
    SetTheUserMgr(TheHamUserMgr);
}

void HamUserMgrTerminate() {
    MILO_ASSERT(TheHamUserMgr, 0x1a);
    RELEASE(TheHamUserMgr);
}
