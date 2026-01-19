#pragma once
#include "game/PartyModeMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Playlist.h"
#include "net_ham/FitnessGoalJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

struct QueueableCommand {
public:
    virtual void pure1() = 0;
    ~QueueableCommand() {}

    union {
        int i;
        HamProfile *profile;
        Playlist *playlist;
        CustomPlaylist *customPlaylist;
        const char *onlineID;
    } unk4;
};

struct CmdGetFitnessGoalFromRC : public QueueableCommand {
public:
    virtual void pure1() {}
    CmdGetFitnessGoalFromRC() {}
};

struct CmdSendFitnessGoalToRC : public QueueableCommand {
public:
    virtual void pure1() {}
    CmdSendFitnessGoalToRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

struct CmdUpdateFitnessGoalToRC : public QueueableCommand {
public:
    virtual void pure1() {}
    CmdUpdateFitnessGoalToRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

struct CmdDeleteFitnessGoalFromRC : public QueueableCommand {
public:
    virtual void pure1() {}
    CmdDeleteFitnessGoalFromRC(HamProfile *p) : profile(p) {}
    HamProfile *profile;
};

struct CmdChangeProfileOnlineID : public QueueableCommand {
public:
    virtual void pure1() {}
    CmdChangeProfileOnlineID(String s) : str(s) {}
    String str;
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

protected:
    String unk2c;
    String unk34;
    std::list<QueueableCommand *> unk3c;
    bool unk44;
    RCJob *unk48;
    HamProfile *unk4c;
    std::list<HamProfile *> unk50;

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
};

extern FitnessGoalMgr *TheFitnessGoalMgr;
