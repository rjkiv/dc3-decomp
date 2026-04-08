#include "net_ham/FriendsListJobs.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "net/WebSvcReq.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"
#include <cstddef>
#include <cstring>

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

DataNode UpdateFriendsListJob::OnMsg(PlatformMgrOpCompleteMsg const &msg) {
    MILO_ASSERT(mFriendsListJobState == kEnumeratingFriends, 0x28);
    GetFriendsListToken();
    int uploadToken = 0;
    if (mProfile) {
        uploadToken = mProfile->GetUploadFriendsToken();
    }

    if (msg.Success() && mProfile && mProfile->HasValidSaveData()
        && unkb8 != uploadToken) {
        mFriendsListJobState = kUpdatingFriends;
        DataPoint dataP;
        String friendInfo;
        String friendName;
        int friendSize = unkbc.size();
        static Symbol friends("friends");
        char namebuf[8];
        char buf[24];
        for (int i = 0; i < friendSize - 1; i++) {
            friendName = unkbc[i]->mName.c_str();
            XUID xuid = unkbc[i]->unk18;
            friendInfo += MakeString("%llu,", xuid);
            Hx_snprintf(namebuf, 8, "name%03d", i);
            dataP.AddPair(namebuf, friendName);
            Hx_snprintf(namebuf, 8, "guid%03d", i);
            Hx_snprintf(buf, 24, "%lld", xuid);
            dataP.AddPair(namebuf, buf);
        }
        if (unkbc.size() != 0) {
            friendName = unkbc[friendSize - 1]->mName.c_str();
            XUID xuid = unkbc[friendSize - 1]->unk18;
            friendInfo += MakeString("%llu", xuid);
        }
        dataP.AddPair(friends, friendInfo);
        SetDataPoint(dataP);
        mCallback = this;
        TheRockCentral.ManageJob(this);
    } else {
        mFriendsListJobState = kFriendsListState_3;
        WebSvcRequest::Cancel(false);
        TheRockCentral.ManageJob(this);
    }

    return 1;
}

BEGIN_HANDLERS(UpdateFriendsListJob)
    HANDLE_MESSAGE(PlatformMgrOpCompleteMsg)
    HANDLE_MESSAGE(RCJobCompleteMsg)
END_HANDLERS
