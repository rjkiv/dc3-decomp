#pragma once
#include "net/DingoJob.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"
#include <vector>

class DingoServer : public Hmx::Object {
public:
    enum AuthState {
        //   kUserDingoAuthUnauthenticated = 0x0000,
        //   kUserDingoAuthAuthenticating = 0x0001,
        //   kUserDingoAuthAuthenticated = 0x0002,
        //   kUserDingoAuthAuthFailedInvalidUserName = 0x0003,
        //   kUserDingoAuthAuthFailedInvalidPassword = 0x0004,
        //   kUserDingoAuthAuthFailedDuplicateUserName = 0x0005,
        //   kUserDingoAuthAuthFailed = 0x0006,
        kServerUnauthed = 0,
        kServerAuthenticating = 3,
        kServerAuthed = 4
    };
    DingoServer();
    virtual ~DingoServer() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void Terminate() {}
    virtual void CreateAccount() = 0;
    virtual bool Authenticate(int padnum) = 0;
    virtual void Logout();
    virtual void Disconnect() {}
    virtual void Poll() {} // 0x70
    virtual void ManageJob(DingoJob *job);
    virtual bool IsAuthenticated() { return mAuthState == kServerAuthed && unk74 != -1; }
    virtual bool IsAuthenticating() { return mAuthState == kServerAuthenticating; }
    virtual const char *GetPlatform() = 0;
    virtual const char *GetHostName() const { return mHostName.c_str(); }
    virtual unsigned int GetIPAddr() { return mIPAddr; }
    virtual unsigned int GetPort() const { return mPort; }

    // SSL vfunc positions are tentative
    virtual const char *GetSSLCertPath() { return ""; }
    virtual const char *GetSSLCertName() { return ""; }
    virtual unsigned short GetSSLVerifyPeer() { return 0; }
    virtual unsigned short GetSSLVerifyHost() { return 0; }
    virtual bool GetSSLEnable() const { return false; }

    virtual void SetLBView(unsigned int lb_id) {}
    virtual void SetLBScoreProperty(unsigned int prop_id) {}
    virtual void SetLoginName(const char *name) {}
    virtual void SetLoginPassword(const char *password) {}
    virtual const char *GetLoginName() { return nullptr; }
    virtual bool HasValidLoginCandidate() const { return true; } // 0xb8
    virtual bool IsValidLoginCandidate(int padnum) const { return true; }
    virtual void MakeSessionJobComplete(bool success) {}
    virtual void JoinSessionComplete(bool success) {}
    virtual void StartSessionComplete(bool success) {} // 0xc8
    virtual void WriteCareerLeaderboardComplete(bool success) {}
    virtual void LeaveSessionComplete(bool success) {}
    virtual void EndSessionComplete(bool success) {}
    virtual void DeleteSessionComplete(bool success) {}
    virtual void StartUploadCareerScore(u64 career_score) {}

    void DelayJob(DingoJob *job);
    void CancelDelayedCalls();
    void AddDelayedCalls();

private:
    bool SendAuthenticateMsg(const char *url, DataPoint &pt, Hmx::Object *callback);

protected:
    virtual void FillAuthParams(DataPoint &pt);
    virtual bool FillAuthParamsFromPadNum(DataPoint &pt, int padnum) { return false; }
    virtual void OnAuthSuccess() {}
    virtual void DoAdditionalLogin();

    DataNode OnMsg(const SigninChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const DingoJobCompleteMsg &);

    bool InitAndAddJob(DingoJob *job, bool immediate, bool delay);
    bool Authenticate(int padnum, const char *url);

    AuthState mAuthState; // 0x2c
    String mHostName; // 0x30
    unsigned int mIPAddr; // 0x38
    unsigned int mPort; // 0x3c
    String unk40;
    String mAuthUrl; // 0x48
    String unk50;
    String mLocale; // 0x58
    String mLanguage; // 0x60
    String mUserAgent; // 0x68
    int unk70; // 0x70 - padnum?
    int unk74; // 0x74
    bool unk78[4]; // 0x78
    OnlineID mOnlineId; // 0x80
    std::vector<String> mDisabledUrls; // 0x98
    std::vector<DingoJob *> mDelayedJobs; // 0xa4
};

extern DingoServer &TheServer;

enum ServerStatusResult {
    kServerStatusConnected = 0,
    kServerStatusInvalidUserName = 1,
    kServerStatusInvalidPassword = 2,
    kServerStatusDuplicateUserName = 3,
    kServerStatusDisconnected = 4,
};

DECLARE_MESSAGE(ServerStatusChangedMsg, "server_status_changed")
ServerStatusChangedMsg(ServerStatusResult r) : Message(Type(), r) {}
ServerStatusResult Result() const { return (ServerStatusResult)mData->Int(2); }
END_MESSAGE
