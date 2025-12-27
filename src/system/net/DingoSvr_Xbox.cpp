#include "net/DingoSvr_Xbox.h"
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoJob.h"
#include "net/DingoSvr.h"
#include "net/SessionJobs_Xbox.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "xdk/xapilibi/xbox.h"

DingoSvrXbox gDingoSvrXbox;
DingoServer &TheServer = gDingoSvrXbox;

DingoSvrXbox::DingoSvrXbox()
    : unkb0(0), mXUID(0), unk148(0), mJobMgr(this), unk15c(0), unk160(0), unk168(0),
      unk170(0), mMsBetweenReconnDingo(0), mLeaderboardID(-1),
      mLeaderboardScorePropID(-1) {}

BEGIN_HANDLERS(DingoSvrXbox)
    HANDLE_MESSAGE(DingoJobCompleteMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_SUPERCLASS(DingoServer)
END_HANDLERS

void DingoSvrXbox::Init() {
    DingoServer::Init();
    DataArray *cfg = SystemConfig("net", "dingo");
    mPort = cfg->FindInt("port");
    mHostName = cfg->FindStr("hostname");
    mUserAgent = cfg->FindStr("user_agent");
    mXLSPFilter = cfg->FindStr("xlsp_filter");
    mMsBetweenReconnDingo = cfg->FindFloat("ms_between_reconn_dingo");
}

bool DingoSvrXbox::Authenticate(int i1) {
    if (unkb0 != 2) {
        SendDebugDataPoint(
            "no_xlsp_connection",
            "location",
            "DingoSvrXbox::Authenticate",
            "severity",
            "warn",
            "project",
            "sync"
        );
        Export(ServerStatusChangedMsg((ServerStatusResult)4), false);
        return false;
    } else {
        return DingoServer::Authenticate(i1, "auth/authenticate/");
    }
}

void DingoSvrXbox::Logout() {
    DingoServer::Logout();
    mXUID = 0;
    mUserName.erase();
}

void DingoSvrXbox::Disconnect() {
    unkb0 = 0;
    unkc8.Disconnect();
}

bool DingoSvrXbox::HasValidLoginCandidate() const {
    u64 xuid;
    char buf[32];
    return GetValidLoginCandidate(buf, xuid) != -1;
}

bool DingoSvrXbox::IsValidLoginCandidate(int pad) const {
    if (!ThePlatformMgr.IsSignedIntoLive(pad)) {
        return false;
    } else {
        return !ThePlatformMgr.IsPadAGuest(pad);
    }
}

void DingoSvrXbox::MakeSessionJobComplete(bool b1) {
    if (b1 && unk170) {
        mJobMgr.QueueJob(new AddLocalPlayerJob(unk170, unk74, false));
        unk15c = 2;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::JoinSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new StartSessionJob(unk170));
        unk15c = 3;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::StartSessionComplete(bool b1) {
    MILO_ASSERT(mLeaderboardID != -1, 0x207);
    MILO_ASSERT(mLeaderboardScorePropID != -1, 0x208);
    if (b1) {
        mJobMgr.QueueJob(new WriteCareerLeaderboardJob(
            unk170, mLeaderboardID, mLeaderboardScorePropID, unk160, unk168
        ));
        unk15c = 4;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::WriteCareerLeaderboardComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new EndSessionJob(unk170));
        unk15c = 5;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::LeaveSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new DeleteSessionJob(unk170));
        unk15c = 7;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::EndSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new RemoveLocalPlayerJob(unk170, unk74));
        unk15c = 6;
    } else {
        unk15c = 0;
    }
}

void DingoSvrXbox::DeleteSessionComplete(bool b1) { unk15c = 0; }

void DingoSvrXbox::StartUploadCareerScore(u64 u) {
    if (unk15c == 0) {
        unk168 = u;
        unk160 = mXUID;
        CreateSession();
    }
}

void DingoSvrXbox::FillAuthParams(DataPoint &pt) {
    DingoServer::FillAuthParams(pt);
    if (HasValidLoginCandidate()) {
        char buf[32];
        unk70 = GetValidLoginCandidate(buf, mXUID);
        mUserName = buf;
    }
    static Symbol username("username");
    pt.AddPair(username, mUserName.c_str());
    static Symbol platform_uid("platform_uid");
    String str70;
    if (mXUID == 0) {
        MILO_NOTIFY("Sending a zero XUID to the server!\n");
    }
    str70 << mXUID;
    pt.AddPair(platform_uid, str70.c_str());
}

bool DingoSvrXbox::FillAuthParamsFromPadNum(DataPoint &pt, int padnum) {
    DingoServer::FillAuthParams(pt);
    if (padnum < 0) {
        MILO_NOTIFY("Bad auth attempt with padnum = %d.", padnum);
        return false;
    } else {
        if (ThePlatformMgr.IsSignedIntoLive(padnum)
            && !ThePlatformMgr.IsPadAGuest(padnum)) {
            String str70;
            XUID xuid;
            HRESULT i3 = XUserGetXUID(padnum, &xuid);
            char name[32];
            HRESULT i4 = XUserGetName(padnum, name, 0x1E);
            bool ret;
            if (i3 == 0 && i4 == 0) {
                str70 = name;
                if (unk74 == -1) {
                    unk70 = padnum;
                    mUserName = name;
                    mXUID = xuid;
                }
                static Symbol username("username");
                pt.AddPair(username, str70.c_str());
                static Symbol platform_uid("platform_uid");
                String str80;
                MILO_ASSERT(xuid, 0x101);
                str80 << xuid;
                pt.AddPair(platform_uid, str80.c_str());
                ret = true;
            } else {
                ret = false;
            }
            return ret;
        }
    }
    return false;
}

void DingoSvrXbox::OnAuthSuccess() {
    unk78[unk70] = true;
    mOnlineId.SetXUID(mXUID);
    mOnlineId.SetPlayerName(ThePlatformMgr.GetName(unk70));
    int old = unk70;
    unk70 = -1;
    unk74 = old;
}

void DingoSvrXbox::CreateSession() {
    XUserSetContext(unk74, 0x800A, 1);
    mJobMgr.QueueJob(new MakeSessionJob(&unk170, 0x706, unk74));
    unk15c = 1;
}

int DingoSvrXbox::GetValidLoginCandidate(char *name, u64 &xuid) const {
    for (int i = 0; i < 4; i++) {
        if (unk78[i]) {
            return -1;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (ThePlatformMgr.IsSignedIntoLive(i) && !ThePlatformMgr.IsPadAGuest(i)) {
            HRESULT xRes = XUserGetXUID(i, &xuid);
            HRESULT nameRes = XUserGetName(i, name, 0x1E);
            if (xRes == 0 && nameRes == 0) {
                return i;
            }
        }
    }
    return -1;
}
