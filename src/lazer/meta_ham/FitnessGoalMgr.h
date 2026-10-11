#pragma once
#include "game/PartyModeMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Playlist.h"
#include "net_ham/FitnessGoalJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

struct QueueableCommand {
public:
    virtual int GetType() = 0;
    ~QueueableCommand() {}
};

struct CmdGetFitnessGoalFromRC : public QueueableCommand {
public:
    virtual int GetType() { return 1; }
    CmdGetFitnessGoalFromRC() {}
};

struct CmdSendFitnessGoalToRC : public QueueableCommand {
public:
    virtual int GetType() { return 2; }
    CmdSendFitnessGoalToRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

struct CmdUpdateFitnessGoalToRC : public QueueableCommand {
public:
    virtual int GetType() { return 4; }
    CmdUpdateFitnessGoalToRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

struct CmdDeleteFitnessGoalFromRC : public QueueableCommand {
public:
    virtual int GetType() { return 3; }
    CmdDeleteFitnessGoalFromRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

class CmdChangeProfileOnlineID : public QueueableCommand {
public:
    virtual int GetType() { return 0; }
    CmdChangeProfileOnlineID(String id) { onlineID = id; }
    String onlineID;
};

class FitnessGoalMgr : public Hmx::Object {
public:
    virtual ~FitnessGoalMgr();
    virtual DataNode Handle(DataArray *, bool);

    FitnessGoalMgr();
    void DeleteFitnessGoalFromRC(HamProfile *);
    void OnSendFitnessGoalToRC(HamProfile *);
    void UpdateFitnessGoal(HamProfile *);

    static void Init();

private:
    bool IsProfileChanged();
    void SendPassiveMsg(Symbol);
    void StartCmdGetFitnessGoalFromRC();
    void BroadcastSyncMsg(Symbol);
    void StartCmdSendFitnessGoalToRC();
    void StartCmdUpdateFitnessGoalToRC();
    void StartCmdDeleteFitnessGoalFromRC();
    void HandleCmdChangeProfileOnlineID();
    void AddPendingProfile(HamProfile *);
    void ProcessNextCommand();
    void QueueCmdChangeProfileOnlineID(String);
    void QueueCmdGetFitnessGoalFromRC();
    void HandleCmdGetFitnessGoalFromRC();
    void QueueCmdSendFitnessGoalToRC(HamProfile *);
    void HandleCmdSendFitnessGoalToRC();
    void QueueCmdUpdateFitnessGoalToRC(HamProfile *);
    void QueueCmdDeleteFitnessGoalFromRC(HamProfile *);
    void HandleCmdDeleteFitnessGoalFromRC();
    void UploadNextProfile();
    bool HasValidProfile();
    void HandleCmdUpdateFitnessGoalToRC();
    void OnSmartGlassListen(int);
    DataNode OnMsg(RCJobCompleteMsg const &);
    DataNode OnMsg(SmartGlassMsg const &);

    String mCurrProfileName; // 0x2c
    String mCurrProfileOnlineIDs; // 0x34
    std::list<QueueableCommand *> mCommands; // 0x3c
    bool mIsProccessingCmd; // 0x44
    RCJob *mCurrJob; // 0x48
    HamProfile *mUploadingProfile; // 0x4c
    std::list<HamProfile *> mPendingProfiles; // 0x50
};

extern FitnessGoalMgr *TheFitnessGoalMgr;
