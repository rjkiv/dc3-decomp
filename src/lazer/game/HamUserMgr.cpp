#include "lazer/game/HamUserMgr.h"
#include "game/HamUser.h"
#include "macros.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "utl/HxGuid.h"
#include "utl/Std.h"
#include "meta_ham/ProfileMgr.h"

HamUserMgr *TheHamUserMgr;

HamUserMgr::HamUserMgr(int size) {
    mUsers.reserve(size);
    for (int i = 0; i < size; i++) {
        HamUser *h = HamUser::NewHamUser(i);
        mUsers.push_back(h);
    }
}

HamUserMgr::~HamUserMgr() {
    TheHamUserMgr = nullptr;
    for (int i = 0; i < 8; i++) {
        delete mUsers[i];
    }
}

BEGIN_HANDLERS(HamUserMgr)
    HANDLE_EXPR(foreach_user, ForEachUser(_msg))
    HANDLE_EXPR(get_active_user, GetActiveUser())
    HANDLE_SUPERCLASS(UserMgr)
END_HANDLERS

void HamUserMgr::GetUsers(std::vector<User *> &users) const {
    users.clear();
    for (int i = 0; i < mUsers.size(); i++) {
        users.push_back(mUsers[i]);
    }
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

RemoteUser *HamUserMgr::GetRemoteUser(UserGuid const &, bool) const {
    MILO_FAIL("Called HamUserMgr::GetRemoteUser");
    return nullptr;
}

HamUser *HamUserMgr::GetActiveUser() const {
    MILO_NOTIFY(
        "Are you sure you should be calling GetActiveUser()??  You probably want GetActiveProfile()!"
    );
    HamUser *user = nullptr;
    HamProfile *h = TheProfileMgr.GetActiveProfile(true);
    if (h)
        user = h->GetHamUser();
    return user;
}

HamUser *HamUserMgr::GetUserFromPad(int pad) const {
    FOREACH (it, mUsers) {
        HamUser *user = *it;
        MILO_ASSERT(user, 0xa4);
        if (user->GetPadNum() == pad)
            return user;
    }
    return nullptr;
}

HamUser *HamUserMgr::GetHamUser(const UserGuid &guid, bool fail) const {
    for (int i = 0; i < 8; i++) {
        if (mUsers[i]->GetUserGuid() == guid) {
            return mUsers[i];
        }
    }
    if (fail) {
        MILO_FAIL("No HamUser exists with guid %s\n", guid.ToString());
    }
    return nullptr;
}

DataNode HamUserMgr::ForEachUser(const DataArray *a) {
    DataNode *var = a->Var(2);
    DataNode n = *var;
    for (int i = 0; i < 8; i++) {
        HamUser *user = mUsers[i];
        MILO_ASSERT(user, 0xBB);
        *var = user;
        for (int j = 3; j < a->Size(); j++) {
            a->Command(j)->Execute();
        }
    }
    *var = n;
    return 0;
}

void HamUserMgrInit(bool b) {
    MILO_ASSERT(!TheHamUserMgr, 0x20);
    TheHamUserMgr = new HamUserMgr(8);
    TheHamUserMgr->SetBool(b);
    SetTheUserMgr(TheHamUserMgr);
    TheDebug.AddExitCallback(HamUserMgrTerminate);
}

void HamUserMgrTerminate() {
    MILO_ASSERT(TheHamUserMgr, 0x1a);
    RELEASE(TheHamUserMgr);
}
