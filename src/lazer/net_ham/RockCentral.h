#pragma once
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoSvr.h"
#include "net_ham/KinectShare.h"
#include "net_ham/MotdJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Timer.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "utl/HxGuid.h"
#include "utl/Str.h"

DECLARE_MESSAGE(UserLoginMsg, "user_login")
END_MESSAGE

class RockCentral : public Hmx::Object {
public:
    RockCentral();
    virtual ~RockCentral();
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetLoginName(const char *);
    virtual void SetLoginPassword(const char *);
    virtual void Login();
    virtual void CreateAccount();
    virtual unsigned int GetPrincipalID() const { return 0; }

    void Init();
    void Poll();
    void Terminate();
    void GetCommunityMsg(int, String &) const;
    int GetCommunityMsgCount() const;
    bool HasDlcMsg();
    void GetDlcMsg(String &) const;
    bool HasUtilityMsg();
    void GetUtilityMsg(String &) const;
    void ForceLogout();
    bool IsOnline();
    void ManageJob(RCJob *);
    void CancelOutstandingCalls(Hmx::Object *);
    void SetMiscArtBitMap(RndBitmap &);
    void DeleteMiscArt();
    int GetMotdFreq() const;

    DataNode OnMsg(const ServerStatusChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const TmsDownloadedMsg &);
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const UserLoginMsg &);

    bool IsLoginBlocked() const { return mLoginBlocked; }
    String GetDlcImage() { return mDlcImage; }
    String GetUtilityImage() { return mUtilityImage; }
    String GetUtilitySound() { return mUtilitySound; }
    String GetMiscImage() { return mMiscImage; }
    RndTex *GetUtilityArt() { return mUtilityArt; }
    int GetLastNewSong() const { return mLastNewSong; }
    unsigned int GetChallengeInterval() const { return mChallengeInterval; }
    void EnterControllerMode() { mNumEnteringControllerMode++; }
    void ExitControllerMode() { mNumExitingControllerMode++; }
    bool HasXpMult() const { return mHasXpMult; }

private:
    static const String kServerVer;
    static bool sCheckSomething;

protected:
    virtual void OnJobFinished(RCJob *);

private:
    std::vector<RCJob *> mJobs; // 0x2c
    std::vector<RCJob *> mNewJobs; // 0x38
    enum {
        kOffline = 0x0000,
        kConnecting = 0x0001,
        kOnline = 0x0002,
        kDisconnecting = 0x0003,
        kFailed = 0x0004
    } mState; // 0x44
    Timer mTime; // 0x48
    float mRetryTime; // 0x78
    float mUploadControllerStatsTime; // 0x7c
    GetMotdJob *mMotdJob; // 0x80
    unsigned int mChallengeInterval; // 0x84
    int mLastNewSong; // 0x88
    bool mHasXpMult; // 0x8c
    int mMotdFreq; // 0x90
    std::vector<String> mCommunityMsgs; // 0x94
    String mDlcMsg; // 0xa0
    String mDlcImage; // 0xa8
    String mDlcSound; // 0xb0
    String mUtilityMsg; // 0xb8
    String mUtilityImage; // 0xc0
    String mUtilitySound; // 0xc8
    String mMiscImage; // 0xd0 - not in PDB
    RndTex *mUtilityArt; // 0xd8
    bool mLoginBlocked; // 0xdc
    bool mHasLoggedIn; // 0xdd
    HxGuid mGameSessionGuid; // 0xe0
    XNADDR mXnaddr; // 0xf0
    ULONGLONG mMachineId; // 0x118
    KinectShareConnection *mKinectShare; // 0x120
    Hmx::Object *mKinectShareCallback;
    int mNumEnteringControllerMode; // 0x128
    int mNumExitingControllerMode; // 0x12c
};

extern RockCentral TheRockCentral;

class RockCentralOpCompleteMsg : public Message, public Hmx::Object {
public:
    RockCentralOpCompleteMsg();
    RockCentralOpCompleteMsg(bool b, int i, DataNode n) : Message(Type(), b, i, n) {}
    RockCentralOpCompleteMsg(DataArray *da) : Message(da) {}
    static Symbol Type() {
        static Symbol t("rock_central_op_complete_msg");
        return t;
    }
    bool Success() const { return mData->Int(2); }
    int Arg1() const { return mData->Int(3); }
    DataNode Arg2() const { return mData->Node(4); }
};
