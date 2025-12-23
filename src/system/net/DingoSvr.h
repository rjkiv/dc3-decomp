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
        kServerAuthed = 4
    };
    DingoServer();
    virtual ~DingoServer() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void Terminate() {}
    virtual void CreateAccount() = 0;
    virtual bool Authenticate(int) = 0;
    virtual void Logout();
    virtual void Disconnect() {}
    virtual void Poll() {} // 0x70
    virtual void ManageJob(DingoJob *);
    virtual bool IsAuthenticated() { return mAuthState == kServerAuthed && unk74 != -1; }
    virtual bool IsAuthenticating() { return mAuthState == 3; }
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

    virtual void SetLBView(unsigned int) {}
    virtual void SetLBScoreProperty(unsigned int) {}
    virtual void SetLoginName(const char *) {}
    virtual void SetLoginPassword(const char *) {}
    virtual const char *GetLoginName() { return nullptr; }
    virtual bool HasValidLoginCandidate() const { return true; } // 0xb8
    virtual bool IsValidLoginCandidate(int) const { return true; }
    virtual void MakeSessionJobComplete(bool) {}
    virtual void JoinSessionComplete(bool) {}
    virtual void StartSessionComplete(bool) {} // 0xc8
    virtual void WriteCareerLeaderboardComplete(bool) {}
    virtual void LeaveSessionComplete(bool) {}
    virtual void EndSessionComplete(bool) {}
    virtual void DeleteSessionComplete(bool) {}
    virtual void StartUploadCareerScore(u64) {}

    void DelayJob(DingoJob *);
    void CancelDelayedCalls();
    void AddDelayedCalls();

private:
    bool SendAuthenticateMsg(char const *, DataPoint &, Hmx::Object *);

protected:
    virtual void FillAuthParams(DataPoint &);
    virtual bool FillAuthParamsFromPadNum(DataPoint &, int) { return false; }
    virtual void OnAuthSuccess() {}
    virtual void DoAdditionalLogin();

    DataNode OnMsg(const SigninChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const DingoJobCompleteMsg &);

    bool InitAndAddJob(DingoJob *, bool, bool);
    bool Authenticate(int, char const *);

    int mAuthState; // 0x2c
    String mHostName; // 0x30
    unsigned int mIPAddr; // 0x38
    unsigned int mPort; // 0x3c
    String unk40;
    String mAuthUrl; // 0x48
    String unk50;
    String mRegion; // 0x58
    String mLanguage; // 0x60
    String mUserAgent; // 0x68
    int unk70; // 0x70 - padnum?
    int unk74;
    bool unk78[4];
    OnlineID unk80;
    std::vector<String> unk98; // 0x98 - urls?
    std::vector<DingoJob *> mDelayedJobs; // 0xa4
};

extern DingoServer &TheServer;

enum ServerStatusResult {
};

DECLARE_MESSAGE(ServerStatusChangedMsg, "server_status_changed")
ServerStatusChangedMsg(ServerStatusResult r) : Message(Type(), r) {}
END_MESSAGE
