#pragma once
#include "hamobj/RhythmBattlePlayer.h"
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Friend.h"
#include "stl/_vector.h"

DECLARE_MESSAGE(PlatformMgrOpCompleteMsg, "platform_mgr_op_complete_msg") // idk
END_MESSAGE

enum FriendsListJobState {
    kFriendsListState_0,
    kEnumeratingFriends,
    kUpdatingFriends,
    kFriendsListState_3
};

class UpdateFriendsListJob : public RCJob {
public:
    virtual DataNode Handle(DataArray *, bool);

    UpdateFriendsListJob(Hmx::Object *, HamProfile *);
    void EnumerateFriends();

protected:
    HamProfile *mProfile; // 0xb0
    int unkb4;
    int unkb8;
    std::vector<Friend *> unkbc;
    FriendsListJobState mFriendsListJobState; // 0xc8

private:
    void GetFriendsListToken();
    DataNode OnMsg(RCJobCompleteMsg const &);
    DataNode OnMsg(PlatformMgrOpCompleteMsg const &);
};
