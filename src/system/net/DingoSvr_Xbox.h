#pragma once
#include "net/DingoSvr.h"
#include "net/XLSPConnection.h"
#include "utl/JobMgr.h"
#include "utl/Str.h"

enum XboxAuthStateT {
    kServerXboxObtainingServiceID = 0x0000,
    kServerXboxConnectingToXLSP = 0x0001,
    kServerXboxXLSPConnected = 0x0002,
};

enum XBLSubmitStateT {
    kServerXBLSubmitState_Idle = 0x0000,
    kServerXBLSubmitState_CreatingSession = 0x0001,
    kServerXBLSubmitState_JoiningSession = 0x0002,
    kServerXBLSubmitState_StartingSession = 0x0003,
    kServerXBLSubmitState_SubmittingScore = 0x0004,
    kServerXBLSubmitState_EndingSession = 0x0005,
    kServerXBLSubmitState_UnjoiningSession = 0x0006,
    kServerXBLSubmitState_DeletingSession = 0x0007,
};

class DingoSvrXbox : public DingoServer {
public:
    DingoSvrXbox();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void CreateAccount() {}
    virtual bool Authenticate(int padnum);
    virtual void Logout();
    virtual void Disconnect();
    virtual void Poll();
    virtual const char *GetPlatform() { return "xbl"; }
    virtual void SetLBView(unsigned int lb_id) { mLeaderboardID = lb_id; }
    virtual void SetLBScoreProperty(unsigned int prop_id) {
        mLeaderboardScorePropID = prop_id;
    }
    virtual bool HasValidLoginCandidate() const;
    virtual bool IsValidLoginCandidate(int padnum) const;
    virtual void MakeSessionJobComplete(bool success);
    virtual void JoinSessionComplete(bool success);
    virtual void StartSessionComplete(bool success);
    virtual void WriteCareerLeaderboardComplete(bool success);
    virtual void LeaveSessionComplete(bool success);
    virtual void EndSessionComplete(bool success);
    virtual void DeleteSessionComplete(bool success);
    virtual void StartUploadCareerScore(u64 career_score);

protected:
    virtual void FillAuthParams(DataPoint &pt);
    virtual bool FillAuthParamsFromPadNum(DataPoint &pt, int padnum);
    virtual void OnAuthSuccess();

private:
    int GetValidLoginCandidate(char *, u64 &) const;
    void CreateSession();

    XboxAuthStateT mState; // 0xb0
    XUID mXUID; // 0xb8
    String mProfileName; // 0xc0
    XLSPConnection mXlspConnection; // 0xc8
    String mXLSPFilter; // 0x140
    unsigned int mDingoServiceID; // 0x148
    JobMgr mJobMgr; // 0x14c
    XBLSubmitStateT mLeaderboardSubmitState; // 0x15c
    XUID mLeaderboardXuid; // 0x160
    u64 mLeaderboardScore; // 0x168
    HANDLE mSessionHandle; // 0x170
    float mTimeBetweenReconn; // 0x174
    unsigned int mLeaderboardID; // 0x178
    unsigned int mLeaderboardScorePropID; // 0x17c
};

extern DingoSvrXbox gDingoSvrXbox;
