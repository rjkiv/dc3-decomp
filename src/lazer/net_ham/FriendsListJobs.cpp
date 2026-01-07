#include "net_ham/FriendsListJobs.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include <cstddef>

UpdateFriendsListJob::UpdateFriendsListJob(Hmx::Object *callback, HamProfile *profile)
    : RCJob("friends/updatefriends/", callback) {
    MILO_ASSERT(callback == NULL, 0x18);
    mProfile = profile;
    unkb4 = profile->GetPadNum();
    mFriendsListJobState = kFriendsListState_0;
}

void UpdateFriendsListJob::EnumerateFriends() {
    mFriendsListJobState = kEnumeratingFriends;
    ThePlatformMgr.EnumerateFriends(unkb4, unkbc, this);
}

DataNode UpdateFriendsListJob::OnMsg(RCJobCompleteMsg const &msg) {
    MILO_ASSERT(mFriendsListJobState == kUpdatingFriends, 0x7d);
    if (msg.Success() && mProfile->HasValidSaveData()) {
        mProfile->SetUploadFriendsToken(unkb8);
    }
    mFriendsListJobState = kFriendsListState_3;
    return 1;
}

BEGIN_HANDLERS(UpdateFriendsListJob)
    HANDLE_MESSAGE(PlatformMgrOpCompleteMsg)
    HANDLE_MESSAGE(RCJobCompleteMsg)
END_HANDLERS
