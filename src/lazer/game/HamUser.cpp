#include "lazer/game/HamUser.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/User.h"

HamUser::HamUser(int i) : unk4(0), unk8(i) {}

HamUser::~HamUser() {}

BEGIN_PROPSYNCS(HamUser)
    SYNC_SUPERCLASS(User)
END_PROPSYNCS

HamUser *HamUser::NewHamUser(int i) { return new HamUser(i); }

void HamUser::Reset() { User::Reset(); }

BEGIN_HANDLERS(HamUser)
    HANDLE_EXPR(get_user_name, UserName())
    HANDLE_ACTION(can_save_data, CanSaveData())
    HANDLE_ACTION(can_get_achievements, CanSaveData())
    HANDLE_ACTION(has_as_friend, HasAsFriend(_msg->Obj<HamUser>(2)))
    HANDLE_SUPERCLASS(LocalUser)
    HANDLE_SUPERCLASS(User)
END_HANDLERS
