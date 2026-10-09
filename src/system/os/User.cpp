#include "os/User.h"
#include "Joypad.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"

#pragma region User

User::User() : mOnlineID(new OnlineID()), mUserGuid(), mMachineID(-1) {
    mUserGuid.Generate();
}

BEGIN_HANDLERS(User)
    HANDLE_EXPR(is_local, IsLocal())
    HANDLE_EXPR(get_player_name, UserName())
    HANDLE_ACTION(reset, Reset())
    HANDLE_EXPR(comes_before, ComesBefore(_msg->Obj<User>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(User)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void User::Reset() { mMachineID = -1; }

void User::SyncSave(BinStream &dest, unsigned int dirtyMask) const {
    dest << mMachineID;
    dest << UserName();
    dest << *mOnlineID;
}

#pragma endregion
#pragma region LocalUser

LocalUser::LocalUser() : mHasOnlineID(0) {}

BEGIN_HANDLERS(LocalUser)
    HANDLE_EXPR(get_pad_num, GetPadNum())
    HANDLE_EXPR(has_online_privilege, HasOnlinePrivilege())
    HANDLE_EXPR(is_signed_in_online, IsSignedInOnline())
END_HANDLERS

int LocalUser::GetPadNum() const { return JoypadGetUsersPadNum(this); }

bool LocalUser::IsJoypadConnected() const {
    static DataNode &n = DataVariable("fake_controllers");
    if (n.Int()) {
        return true;
    } else {
        return JoypadIsConnectedPadNum(GetPadNum());
    }
}

bool LocalUser::HasOnlinePrivilege() const {
    return ThePlatformMgr.UserHasOnlinePrivilege(this);
}
bool LocalUser::IsGuest() const { return ThePlatformMgr.IsUserAGuest(this); }
bool LocalUser::IsSignedIn() const { return ThePlatformMgr.IsUserSignedIn(this); }
bool LocalUser::IsSignedInOnline() const {
    return ThePlatformMgr.IsUserSignedIntoLive(this);
}
bool LocalUser::CanSaveData() const { return IsSignedIn() && !IsGuest(); }
const char *LocalUser::UserName() const { return ThePlatformMgr.GetName(GetPadNum()); }
bool LocalUser::IsLocal() const { return true; }
LocalUser *LocalUser::GetLocalUser() { return this; }
const LocalUser *LocalUser::GetLocalUser() const { return this; }

RemoteUser *LocalUser::GetRemoteUser() {
    MILO_FAIL("Bad Conversion");
    return nullptr;
}

const RemoteUser *LocalUser::GetRemoteUser() const {
    MILO_FAIL("Bad Conversion");
    return nullptr;
}
