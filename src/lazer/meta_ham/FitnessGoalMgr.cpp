#include "meta_ham/FitnessGoalMgr.h"
#include "HamProfile.h"
#include "game/PartyModeMgr.h"
#include "macros.h"
#include "meta_ham/PlaylistSortMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/FitnessGoalJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

FitnessGoalMgr::FitnessGoalMgr() {
    SetName("fitness_goal_mgr", ObjectDir::Main());
    unk2c = gNullStr;
    unk34 = gNullStr;
    unk44 = false;
    unk48 = nullptr;
    unk3c.clear();
    unk4c = nullptr;
}

FitnessGoalMgr::~FitnessGoalMgr() {}

void FitnessGoalMgr::Init() {
    MILO_ASSERT(!TheFitnessGoalMgr, 0x17);
    TheFitnessGoalMgr = new FitnessGoalMgr();
}

void FitnessGoalMgr::StartCmdGetFitnessGoalFromRC() {
    unk48 = new GetFitnessGoalJob(this, unk34.c_str());
    TheRockCentral.ManageJob(unk48);
}

bool FitnessGoalMgr::HasValidProfile() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        pProfile->UpdateOnlineID();
        if (pProfile->IsSignedIn()) {
            int padNum = pProfile->GetPadNum();
            if (ThePlatformMgr.IsSignedIntoLive(padNum) && TheRockCentral.IsOnline()) {
                unk2c = pProfile->GetName();
                QueueCmdChangeProfileOnlineID(pProfile->GetOnlineID()->ToString());
                return true;
            }
        }
    }
    unk2c = gNullStr;
    QueueCmdChangeProfileOnlineID(gNullStr);
    return false;
}

void FitnessGoalMgr::UploadNextProfile() {
    unk50.clear();
    if (unk4c)
        QueueCmdUpdateFitnessGoalToRC(unk4c);
}

void FitnessGoalMgr::UpdateFitnessGoal(HamProfile *profile) {
    profile->UpdateOnlineID();
    if (profile->IsSignedIn()) {
        int padNum = profile->GetPadNum();
        if (ThePlatformMgr.IsSignedIntoLive(padNum) && profile->GetUnk360()) {
            if (!unk4c || unk4c == profile) {
                unk4c = profile;
                UploadNextProfile();
            } else {
                AddPendingProfile(profile);
            }
        }
    }
}

void FitnessGoalMgr::BroadcastSyncMsg(Symbol s) {
    Symbol sym = s;
    MILO_LOG("[FitnessGoalMgr::BroadcastSyncMsg] Broadcasting msg (%s).\n", sym);
    Message msg(sym);
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void FitnessGoalMgr::StartCmdSendFitnessGoalToRC() {
    QueueableCommand *cmd = unk3c.front();
    unk48 = new SetFitnessGoalJob(this, cmd->unk4.profile);
    TheRockCentral.ManageJob(unk48);
}

void FitnessGoalMgr::StartCmdUpdateFitnessGoalToRC() {
    QueueableCommand *cmd = unk3c.front();
    unk48 = new UpdateFitnessGoalJob(this, cmd->unk4.profile);
    TheRockCentral.ManageJob(unk48);
}

void FitnessGoalMgr::StartCmdDeleteFitnessGoalFromRC() {
    QueueableCommand *cmd = unk3c.front();
    unk48 = new DeleteFitnessGoalJob(this, cmd->unk4.profile);
    TheRockCentral.ManageJob(unk48);
}

void FitnessGoalMgr::HandleCmdChangeProfileOnlineID() {
    MILO_LOG("===== HandleCmdChangeProfileOnlineID\n");
    unk34 = unk3c.front()->unk4.onlineID;
    RELEASE(unk3c.front());
    unk3c.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdDeleteFitnessGoalFromRC() {
    unk48 = nullptr;
    RELEASE(unk3c.front());
    unk3c.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdUpdateFitnessGoalToRC() {
    MILO_LOG("===== HandleCmdUpdateFitnessGoalToRC\n");
    unk48 = nullptr;
    DataNode fitness("fitness");
    DataNode updated("updated");
    ThePlatformMgr.SmartGlassSend(0, DataArrayPtr(fitness, updated));
    RELEASE(unk3c.front());
    unk3c.pop_front();
    if (unk4c) {
        unk4c->ClearFitnessGoalNeedUpload();
    }
    if (unk50.empty()) {
        unk4c = nullptr;
    } else {
        UploadNextProfile();
    }
    ProcessNextCommand();
}

void FitnessGoalMgr::QueueCmdGetFitnessGoalFromRC() {
    CmdGetFitnessGoalFromRC *cmd = new CmdGetFitnessGoalFromRC();
    unk3c.push_back(cmd);
    if (!unk44) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdSendFitnessGoalToRC(HamProfile *profile) {
    CmdSendFitnessGoalToRC *cmd = new CmdSendFitnessGoalToRC(profile);
    unk3c.push_back(cmd);
    if (!unk44) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdUpdateFitnessGoalToRC(HamProfile *profile) {
    CmdUpdateFitnessGoalToRC *cmd = new CmdUpdateFitnessGoalToRC(profile);
    unk3c.push_back(cmd);
    if (!unk44) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdDeleteFitnessGoalFromRC(HamProfile *profile) {
    CmdDeleteFitnessGoalFromRC *cmd = new CmdDeleteFitnessGoalFromRC(profile);
    unk3c.push_back(cmd);
    if (!unk44) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdChangeProfileOnlineID(String str) {
    CmdChangeProfileOnlineID *cmd = new CmdChangeProfileOnlineID(str);
    unk3c.push_back(cmd);
    if (!unk44) {
        ProcessNextCommand();
    }
}

BEGIN_HANDLERS(FitnessGoalMgr)
    HANDLE_EXPR(has_valid_profile, HasValidProfile())
    HANDLE_EXPR(is_profile_changed, IsProfileChanged())
    HANDLE_ACTION(get_fitness_goal_from_rc, QueueCmdGetFitnessGoalFromRC())
    HANDLE_ACTION(smart_glass_listen, OnSmartGlassListen(_msg->Int(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(SmartGlassMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
