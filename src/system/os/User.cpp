#include "os/User.h"

#include "obj/Object.h"

void User::Reset() { mMachineID = -1; }
bool LocalUser::IsLocal() const { return true; }
LocalUser *LocalUser::GetLocalUser() { return this; }
const LocalUser *LocalUser::GetLocalUser() const { return this; }

User::User() : mOnlineID(new OnlineID()), mUserGuid(), mMachineID(-1) {
    mUserGuid.Generate();
}

void User::SyncSave(BinStream &bs, unsigned int ui) const {
    bs << mMachineID;
    bs << UserName();
    bs << *mOnlineID;
}

LocalUser::LocalUser() : mHasOnlineID(0) {}

RemoteUser *LocalUser::GetRemoteUser() {
    MILO_FAIL("Bad Conversion");
    return nullptr;
}

const RemoteUser *LocalUser::GetRemoteUser() const {
    MILO_FAIL("Bad Conversion");
    return nullptr;
}

BEGIN_HANDLERS(User)
    HANDLE_EXPR(is_local, IsLocal())
    HANDLE_EXPR(get_player_name, UserName())
    HANDLE_ACTION(reset, Reset())
    HANDLE_EXPR(comes_before, ComesBefore(_msg->Obj<User>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_HANDLERS(LocalUser)
    HANDLE_EXPR(get_pad_num, GetPadNum())
    HANDLE_EXPR(has_online_privilege, HasOnlinePrivilege())
    HANDLE_EXPR(is_signed_in_online, IsSignedInOnline())
END_HANDLERS

BEGIN_PROPSYNCS(User)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
