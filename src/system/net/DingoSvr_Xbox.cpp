#include "net/DingoSvr_Xbox.h"
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoJob.h"
#include "net/DingoSvr.h"
#include "net/SessionJobs_Xbox.h"
#include "net/XLSPConnection.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "xdk/XAPILIB.h"

DingoSvrXbox gDingoSvrXbox;
DingoServer &TheServer = gDingoSvrXbox;

DingoSvrXbox::DingoSvrXbox()
    : mState(kServerXboxObtainingServiceID), mXUID(0), mDingoServiceID(0), mJobMgr(this),
      mLeaderboardSubmitState(kServerXBLSubmitState_Idle), mLeaderboardXuid(0),
      mLeaderboardScore(0), mSessionHandle(0), mTimeBetweenReconn(0), mLeaderboardID(-1),
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
    mTimeBetweenReconn = cfg->FindFloat("ms_between_reconn_dingo");
}

bool DingoSvrXbox::Authenticate(int i1) {
    if (mState != kServerXboxXLSPConnected) {
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
    mProfileName.erase();
}

void DingoSvrXbox::Disconnect() {
    mState = kServerXboxObtainingServiceID;
    mXlspConnection.Disconnect();
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
    if (b1 && mSessionHandle) {
        mJobMgr.QueueJob(new AddLocalPlayerJob(mSessionHandle, mPadLoggedIn, false));
        mLeaderboardSubmitState = kServerXBLSubmitState_JoiningSession;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::JoinSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new StartSessionJob(mSessionHandle));
        mLeaderboardSubmitState = kServerXBLSubmitState_StartingSession;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::StartSessionComplete(bool b1) {
    MILO_ASSERT(mLeaderboardID != -1, 0x207);
    MILO_ASSERT(mLeaderboardScorePropID != -1, 0x208);
    if (b1) {
        mJobMgr.QueueJob(new WriteCareerLeaderboardJob(
            mSessionHandle,
            mLeaderboardID,
            mLeaderboardScorePropID,
            mLeaderboardXuid,
            mLeaderboardScore
        ));
        mLeaderboardSubmitState = kServerXBLSubmitState_SubmittingScore;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::WriteCareerLeaderboardComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new EndSessionJob(mSessionHandle));
        mLeaderboardSubmitState = kServerXBLSubmitState_EndingSession;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::LeaveSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new DeleteSessionJob(mSessionHandle));
        mLeaderboardSubmitState = kServerXBLSubmitState_DeletingSession;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::EndSessionComplete(bool b1) {
    if (b1) {
        mJobMgr.QueueJob(new RemoveLocalPlayerJob(mSessionHandle, mPadLoggedIn));
        mLeaderboardSubmitState = kServerXBLSubmitState_UnjoiningSession;
    } else {
        mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
    }
}

void DingoSvrXbox::DeleteSessionComplete(bool b1) {
    mLeaderboardSubmitState = kServerXBLSubmitState_Idle;
}

void DingoSvrXbox::StartUploadCareerScore(u64 u) {
    if (mLeaderboardSubmitState == kServerXBLSubmitState_Idle) {
        mLeaderboardScore = u;
        mLeaderboardXuid = mXUID;
        CreateSession();
    }
}

void DingoSvrXbox::FillAuthParams(DataPoint &pt) {
    DingoServer::FillAuthParams(pt);
    if (HasValidLoginCandidate()) {
        char buf[32];
        mPadLoggingIn = GetValidLoginCandidate(buf, mXUID);
        mProfileName = buf;
    }
    static Symbol username("username");
    pt.AddPair(username, mProfileName.c_str());
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
    } else if (ThePlatformMgr.IsSignedIntoLive(padnum)
               && !ThePlatformMgr.IsPadAGuest(padnum)) {
        String str70;
        XUID xuid;
        HRESULT i3 = XUserGetXUID(padnum, &xuid);
        char name[32];
        HRESULT i4 = XUserGetName(padnum, name, 0x1E);
        bool ret;
        if (i3 != 0 || i4 != 0) {
            return false;
        } else {
            str70 = name;
            if (mPadLoggedIn == -1) {
                mPadLoggingIn = padnum;
                mProfileName = name;
                mXUID = xuid;
            }
            static Symbol username("username");
            pt.AddPair(username, str70.c_str());
            static Symbol platform_uid("platform_uid");
            String str80;
            MILO_ASSERT(xuid, 0x101);
            str80 << xuid;
            pt.AddPair(platform_uid, str80.c_str());
            return true;
        }
    }
    return false;
}

void DingoSvrXbox::OnAuthSuccess() {
    mPlayerLoginState[mPadLoggingIn] = true;
    mOnlineID.SetXUID(mXUID);
    mOnlineID.SetPlayerName(ThePlatformMgr.GetName(mPadLoggingIn));
    int old = mPadLoggingIn;
    mPadLoggingIn = -1;
    mPadLoggedIn = old;
}

void DingoSvrXbox::CreateSession() {
    XUserSetContext(mPadLoggedIn, 0x800A, 1);
    mJobMgr.QueueJob(new MakeSessionJob(&mSessionHandle, 0x706, mPadLoggedIn));
    mLeaderboardSubmitState = kServerXBLSubmitState_CreatingSession;
}

int DingoSvrXbox::GetValidLoginCandidate(char *name, u64 &xuid) const {
    for (int i = 0; i < 4; i++) {
        if (mPlayerLoginState[i]) {
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

void DingoSvrXbox::Poll() {
    if (!ThePlatformMgr.IsConnected()) {
        return;
    }
    mJobMgr.Poll();

    switch (mState) {
    case kServerXboxObtainingServiceID: {
        if (ThePlatformMgr.GetServiceID("dingo", mDingoServiceID)) {
            if (mXLSPFilter.empty()) {
                MILO_NOTIFY("DingoSvrXbox: Empty XLSP filter string.");
                break;
            } else if (mDingoServiceID == 0) {
                MILO_NOTIFY("DingoSvrXbox: Invalid Dingo service ID.");
                break;
            } else {
                mXlspConnection.Connect(mXLSPFilter.c_str(), mDingoServiceID);
                mState = kServerXboxConnectingToXLSP;
            }
        }
        break;
    }
    case kServerXboxConnectingToXLSP:
        if (mXlspConnection.GetState() == 3) {
            mState = kServerXboxXLSPConnected;
            mIPAddr = mXlspConnection.GetServiceIP();
            mHostName.erase();
        }
        break;
    case kServerXboxXLSPConnected:
        break;
    default:
        MILO_FAIL("DingoSvrXbox: State %d unhandled.", mState);
        break;
    }

    mXlspConnection.Poll();
    if (mXlspConnection.GetState() == 4
        && mXlspConnection.GetTimerSplitMs() >= mTimeBetweenReconn) {
        Disconnect();
    }
}
