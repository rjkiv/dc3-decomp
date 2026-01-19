#include "net_ham/RockCentral.h"
#include "meta/ConnectionStatusPanel.h"
#include "meta_ham/ProfileMgr.h"
#include "net/DingoSvr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"
#include "xdk/XNET.h"

char *g_szMachineIdString;
const String RockCentral::kServerVer = "1";
RockCentral TheRockCentral;

namespace {
    void RockCentralTerminate() { TheRockCentral.Terminate(); }

    class DataPointJob : public RCJob {
    public:
        DataPointJob(DataPoint &pt, String &url) : RCJob(url.c_str(), nullptr) {
            SetDataPoint(pt);
        }
    };

    void SendDataPointNoReturn(DataPoint &dataPoint) {
        const char *type = dataPoint.Type();
        String str;
        if (type && strlen(type) != 0 && type[0] != '/') {
            if (type[strlen(type) - 1] == '/') {
                str = MakeString("dataminer/%s", dataPoint.Type());
            } else {
                str = MakeString("dataminer/%s/", dataPoint.Type());
            }
        } else {
            MILO_WARN(
                "SendDataPointNoReturn: dataPoint.mType must be in '<url>/' format!"
            );
            str = "dataminer/undefined/";
        }
        // so if login IS blocked...what do we do with this job?
        DataPointJob *job = new DataPointJob(dataPoint, str);
        if (!TheRockCentral.IsLoginBlocked()) {
            TheServer.ManageJob(job);
        }
    }
}

RockCentral::RockCentral()
    : mState(), unk7c(0), mMOTDJob(0), unk84(60000), mRockCentralTime(-1), unk8c(0),
      unk90(0), mMiscArt(0), mLoginBlocked(0), unkdd(0), mKinectShareConnection(0),
      unk124(0), unk128(0), unk12c(0) {}

RockCentral::~RockCentral() { RELEASE(mKinectShareConnection); }

BEGIN_HANDLERS(RockCentral)
    HANDLE_MESSAGE(ServerStatusChangedMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_MESSAGE(TmsDownloadedMsg)
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(UserLoginMsg)
    HANDLE_EXPR(state, mState)
    HANDLE_ACTION(force_logout, ForceLogout())
    HANDLE_EXPR(is_online, IsOnline())
    HANDLE_EXPR(toggle_block_login, mLoginBlocked = !mLoginBlocked)
    HANDLE_EXPR(is_login_blocked, mLoginBlocked)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void RockCentral::ForceLogout() {
    if (mState == 2 || mState == 1) {
        mState = (State)3;
        TheServer.Logout();
    }
}

bool RockCentral::IsOnline() {
    if (mLoginBlocked) {
        return false;
    } else {
        return mState == 2;
    }
}

void RockCentral::SetLoginName(const char *name) { TheServer.SetLoginName(name); }
void RockCentral::SetLoginPassword(const char *password) {
    TheServer.SetLoginPassword(password);
}

void RockCentral::Login() {
    mState = (State)1;
    unkdd = false;
    if (!TheServer.Authenticate(0)) { // should be TheServer->unk74
        Export(ServerStatusChangedMsg((ServerStatusResult)4), false);
    }
}

void RockCentral::CreateAccount() {
    MILO_ASSERT(mState == kFailed, 0x124);
    TheServer.CreateAccount();
}

void RockCentral::OnJobFinished(RCJob *job) {
    MILO_ASSERT(job->IsFinished(), 0x24A);
    delete job;
}

void RockCentral::Init() {
    SetName("rock_central", ObjectDir::Main());
    TheServer.AddSink(this);
    static Symbol connection_status_changed("connection_status_changed");
    ThePlatformMgr.AddSink(this, ConnectionStatusChangedMsg::Type());
    ThePlatformMgr.AddSink(this, TmsDownloadedMsg::Type());
    TheDebug.AddExitCallback(RockCentralTerminate);
    TheDataPointMgr.SetDataPointRecorder(SendDataPointNoReturn);
    unke0.Generate();
    unk48.Start();
    unk78 = unk48.Ms();
    unk7c = unk48.Ms() + 600000.0f;
    mDLCMsg = gNullStr;
    mUtilityMsg = gNullStr;
    mCommunityMsgs.clear();
}

void RockCentral::Terminate() {
    TheServer.RemoveSink(this);
    ThePlatformMgr.RemoveSink(this, ConnectionStatusChangedMsg::Type());
    ThePlatformMgr.RemoveSink(this, TmsDownloadedMsg::Type());
}

void RockCentral::GetCommunityMsg(int index, String &str) const {
    MILO_ASSERT_RANGE(index, 0, mCommunityMsgs.size(), 0x1AE);
    str = mCommunityMsgs[index];
}

int RockCentral::GetCommunityMsgCount() const { return mCommunityMsgs.size(); }
bool RockCentral::HasDlcMsg() { return !(mDLCMsg == gNullStr); }
void RockCentral::GetDlcMsg(String &str) const { str = mDLCMsg; }
bool RockCentral::HasUtilityMsg() { return !(mUtilityMsg == gNullStr); }
void RockCentral::GetUtilityMsg(String &str) const { str = mUtilityMsg; }
DataNode RockCentral::OnMsg(const UserLoginMsg &) { return 1; }

void RockCentral::ManageJob(RCJob *job) {
    if (!mLoginBlocked) {
        TheServer.ManageJob(job);
    }
}

void RockCentral::SetMiscArtBitMap(RndBitmap &bmap) {
    DeleteMiscArt();
    mMiscArt = Hmx::Object::New<RndTex>();
    mMiscArt->SetBitmap(bmap, nullptr, false, RndTex::kRegular);
}

void RockCentral::DeleteMiscArt() {
    if (mMiscArt) {
        RELEASE(mMiscArt);
    }
}

void RockCentral::CancelOutstandingCalls(Hmx::Object *obj) {
    for (auto it = unk2c.begin(); it != unk2c.end();) {
        RCJob *cur = *it;
        if (cur->GetCallback() == obj) {
            unk2c.erase(it);
            cur->Cancel(false);
            OnJobFinished(cur);
            it = unk2c.begin();
        } else {
            ++it;
        }
    }
}

DataNode RockCentral::OnMsg(const ConnectionStatusChangedMsg &msg) {
    if (msg.Connected() && (mState == 4 || mState == 0)) {
        mState = (State)0;
        unk78 = unk48.Ms();
    } else if (!msg.Connected() && (mState == 2 || mState == 1)) {
        mState = (State)3;
        TheServer.Logout();
    }
    return 1;
}

DataNode RockCentral::OnMsg(const TmsDownloadedMsg &msg) {
    if (ThePlatformMgr.IsConnected() && (mState == 4 || mState == 0)) {
        mState = (State)0;
        unk78 = unk48.Ms();
    }
    return 1;
}

DataNode RockCentral::OnMsg(const RCJobCompleteMsg &msg) {
    if (msg.Success()) {
        mMOTDJob->GetMotdData(
            unk84,
            mRockCentralTime,
            unk8c,
            unk90,
            mCommunityMsgs,
            mDLCMsg,
            mUtilityMsg,
            unkb0,
            unkb8,
            unkc0,
            unkc8,
            unkd0
        );

        // void GetMotdData(
        //     unsigned int &challengeInterval,
        //     int &lastNewSongDt,
        //     bool &motdXPFlag,
        //     int &motdFreq,
        //     std::vector<String> &toasts,
        //     String &motd,
        //     String &motdImage,
        //     String &motdSound,
        //     String &motdAux,
        //     String &motdImageAux,
        //     String &motdSoundAux,
        //     String &motdMiscImage
        // );

        TheProfileMgr.CheckForServerCrewUnlock();
        static Symbol motd_loaded("motd_loaded");
        static Message msg(motd_loaded);
        UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("main_panel");
        if (panel->GetState() == UIPanel::kUp) {
            panel->HandleType(msg);
        }
    }
    return 1;
}

DataNode RockCentral::OnMsg(const ServerStatusChangedMsg &msg) {
    if (msg.Result() == kServerStatusConnected && mState != 2) {
        unkdd = true;
        mState = (State)2;
        XNetGetTitleXnAddr(&mXNetAddr);
        XNetXnAddrToMachineId(&mXNetAddr, &mMachineID);
        Hx_snprintf(g_szMachineIdString, 20, "%llu", mMachineID);
    } else if (msg.Result() != kServerStatusConnected) {
        if (mState == 3) {
            mState = (State)0;
            unk78 = unk48.Ms() + 8000.0f;
        } else if (msg.Result() == 1) {
            mState = (State)4;
            CreateAccount();
            unk78 = unk48.Ms();
        } else {
            mState = (State)4;
            unk78 = unk48.Ms() + 40000.0f;
        }
    }
    Hmx::Object::Handle(msg, false);
    return 1;
}
