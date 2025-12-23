#pragma once
#include "net/DingoSvr.h"
#include "net/XLSPConnection.h"
#include "utl/JobMgr.h"
#include "utl/Str.h"

class DingoSvrXbox : public DingoServer {
public:
    DingoSvrXbox();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void CreateAccount() {}
    virtual bool Authenticate(int);
    virtual void Logout();
    virtual void Disconnect();
    virtual void Poll();
    virtual const char *GetPlatform() { return "xbl"; }
    virtual void SetLBView(unsigned int view) { mLeaderboardID = view; }
    virtual void SetLBScoreProperty(unsigned int prop) { mLeaderboardScorePropID = prop; }
    virtual bool HasValidLoginCandidate() const;
    virtual bool IsValidLoginCandidate(int) const;
    virtual void MakeSessionJobComplete(bool);
    virtual void JoinSessionComplete(bool);
    virtual void StartSessionComplete(bool);
    virtual void WriteCareerLeaderboardComplete(bool);
    virtual void LeaveSessionComplete(bool);
    virtual void EndSessionComplete(bool);
    virtual void DeleteSessionComplete(bool);
    virtual void StartUploadCareerScore(u64);

private:
    int GetValidLoginCandidate(char *, u64 &) const;
    void CreateSession();

protected:
    virtual void FillAuthParams(DataPoint &);
    virtual bool FillAuthParamsFromPadNum(DataPoint &, int);
    virtual void OnAuthSuccess();

    int unkb0;
    int unkb4;
    XUID mXUID; // 0xb8
    String mUserName; // 0xc0
    XLSPConnection unkc8;
    String mXLSPFilter; // 0x140
    int unk148;
    JobMgr mJobMgr; // 0x14c
    int unk15c; // 0x15c - state? last job type queued?
    u64 unk160;
    u64 unk168;
    HANDLE unk170;
    float mMsBetweenReconnDingo; // 0x174
    unsigned int mLeaderboardID; // 0x178
    unsigned int mLeaderboardScorePropID; // 0x17c
};

extern DingoSvrXbox gDingoSvrXbox;
