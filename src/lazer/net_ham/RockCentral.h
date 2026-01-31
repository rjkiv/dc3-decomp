#pragma once
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoSvr.h"
#include "net_ham/KinectShare.h"
#include "net_ham/MotdJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "utl/HxGuid.h"
#include "utl/Str.h"

DECLARE_MESSAGE(TmsDownloadedMsg, "tms_downloaded")
END_MESSAGE

DECLARE_MESSAGE(UserLoginMsg, "user_login")
END_MESSAGE

class RockCentral : public Hmx::Object {
public:
    enum State {
        kFailed = 4
    };
    RockCentral();
    virtual ~RockCentral();
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetLoginName(const char *);
    virtual void SetLoginPassword(const char *);
    virtual void Login();
    virtual void CreateAccount();
    virtual unsigned int GetPrincipalID() const { return 0; }

    void Init();
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

    DataNode OnMsg(const ServerStatusChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const TmsDownloadedMsg &);
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const UserLoginMsg &);

    bool IsLoginBlocked() const { return mLoginBlocked; }
    String GetDLCImage() { return mUtilityMsg; }
    String GetUtilityImage() { return unkc0; }
    String GetUtilitySound() { return unkc8; }
    String GetMiscImage() { return unkd0; }
    int GetRockCentralTime() { return mRockCentralTime; }
    void SetRockCentralTime(int i) { mRockCentralTime = i; }
    int GetUnk12c() const { return unk12c; }
    void SetUnk12c(int i) { unk12c = i; }
    int GetUnk128() const { return unk128; }
    void SetUnk128(int i) { unk128 = i; }
    bool GetUnk8c() const { return unk8c; }

private:
    static const String kServerVer;

protected:
    virtual void OnJobFinished(RCJob *);

    std::vector<RCJob *> unk2c;
    std::vector<RCJob *> unk38;
    State mState; // 0x44
    Timer unk48;
    float unk78;
    float unk7c;
    GetMotdJob *mMOTDJob; // 0x80
    unsigned int unk84;
    int mRockCentralTime; // 0x88
    bool unk8c;
    int unk90;
    std::vector<String> mCommunityMsgs; // 0x94
    String mDLCMsg; // 0xa0
    String mUtilityMsg; // 0xa8
    String unkb0;
    String unkb8;
    String unkc0;
    String unkc8;
    String unkd0;
    RndTex *mMiscArt; // 0xd8
    bool mLoginBlocked; // 0xdc
    bool unkdd;
    HxGuid unke0;
    XNADDR mXNetAddr; // 0xf0
    ULONGLONG mMachineID; // 0x118
    KinectShareConnection *mKinectShareConnection; // 0x120
    int unk124;
    int unk128;
    int unk12c;
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
