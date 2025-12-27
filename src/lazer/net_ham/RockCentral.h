#pragma once
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoSvr.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Timer.h"
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

    DataNode OnMsg(const ServerStatusChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const TmsDownloadedMsg &);
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const UserLoginMsg &);

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
    int unk80;
    int unk84;
    int unk88;
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
    int unkd8;
    bool mLoginBlocked; // 0xdc
    bool unkdd;
    HxGuid unke0;
    int unkf0, unkf4, unkf8, unkfc;
    int unk100, unk104, unk108, unk10c;
    int unk110, unk114, unk118, unk11c;
    int unk120; // 0x120 - KinectShareConnection*
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
    DataNode Arg2() const;
};
