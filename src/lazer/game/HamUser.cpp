#include "lazer/game/HamUser.h"
#include "meta_ham/SkeletonIdentifier.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/User.h"
#include "xdk/XAPILIB.h"

HamUser::HamUser(int i) : unk4(0), unk8(i) {}

HamUser::~HamUser() {}

BEGIN_HANDLERS(HamUser)
    HANDLE_EXPR(get_user_name, UserName())
    HANDLE_EXPR(can_save_data, CanSaveData())
    HANDLE_EXPR(can_get_achievements, CanGetAchievements())
    HANDLE_EXPR(has_as_friend, HasAsFriend(_msg->Obj<HamUser>(2)))
    HANDLE_SUPERCLASS(LocalUser)
    HANDLE_SUPERCLASS(User)
END_HANDLERS

BEGIN_PROPSYNCS(HamUser)
    SYNC_SUPERCLASS(User)
END_PROPSYNCS

int HamUser::GetPadNum() const {
    if (TheSkeletonIdentifier->GetPlayerPadNum(unk8) < 4) {
        return TheSkeletonIdentifier->GetPlayerPadNum(unk8);
    } else {
        return -1;
    }
}

bool HamUser::CanSaveData() const { return GetPadNum() >= 0; }

HamUser *HamUser::NewHamUser(int i) { return new HamUser(i); }

bool HamUser::HasAsFriend(HamUser *user) const {
    if (user) {
        XUID xuid = user->GetOnlineID()->GetXUID();
        BOOL result;
        if (XUserAreUsersFriends(GetPadNum(), &xuid, 1, &result, nullptr)
            == ERROR_SUCCESS) {
            return result;
        }
    }
    return false;
}
