#include "meta_ham/FitnessGoalMgr.h"
#include "HamProfile.h"
#include "game/PartyModeMgr.h"
#include "hamobj/HamGameData.h"
#include "macros.h"
#include "meta_ham/PassiveMessenger.h"
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
#include "utl/DataPointMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

FitnessGoalMgr::FitnessGoalMgr() {
    SetName("fitness_goal_mgr", ObjectDir::Main());
    mCurrProfileName = gNullStr;
    mCurrProfileOnlineIDs = gNullStr;
    mIsProccessingCmd = false;
    mCurrJob = nullptr;
    mCommands.clear();
    mUploadingProfile = nullptr;
}

FitnessGoalMgr::~FitnessGoalMgr() {}

void FitnessGoalMgr::Init() {
    MILO_ASSERT(!TheFitnessGoalMgr, 0x17);
    TheFitnessGoalMgr = new FitnessGoalMgr();
}

void FitnessGoalMgr::StartCmdGetFitnessGoalFromRC() {
    mCurrJob = new GetFitnessGoalJob(this, mCurrProfileOnlineIDs.c_str());
    TheRockCentral.ManageJob(mCurrJob);
}

bool FitnessGoalMgr::HasValidProfile() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        pProfile->UpdateOnlineID();
        if (pProfile->IsSignedIn()) {
            int padNum = pProfile->GetPadNum();
            if (ThePlatformMgr.IsSignedIntoLive(padNum) && TheRockCentral.IsOnline()) {
                mCurrProfileName = pProfile->GetName();
                QueueCmdChangeProfileOnlineID(pProfile->GetOnlineID()->ToString());
                return true;
            }
        }
    }
    mCurrProfileName = gNullStr;
    QueueCmdChangeProfileOnlineID(gNullStr);
    return false;
}

void FitnessGoalMgr::UploadNextProfile() {
    if (!mPendingProfiles.empty()) {
        mUploadingProfile = mPendingProfiles.front();
        mPendingProfiles.pop_front();
    }

    if (mUploadingProfile)
        QueueCmdUpdateFitnessGoalToRC(mUploadingProfile);
}

void FitnessGoalMgr::UpdateFitnessGoal(HamProfile *profile) {
    profile->UpdateOnlineID();
    if (profile->IsSignedIn()) {
        int padNum = profile->GetPadNum();
        if (ThePlatformMgr.IsSignedIntoLive(padNum) && profile->GetUnk360()) {
            if (!mUploadingProfile || mUploadingProfile == profile) {
                mUploadingProfile = profile;
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
    CmdSendFitnessGoalToRC *cmd = (CmdSendFitnessGoalToRC *)mCommands.front();
    mCurrJob = new SetFitnessGoalJob(this, cmd->profile);
    TheRockCentral.ManageJob(mCurrJob);
}

void FitnessGoalMgr::StartCmdUpdateFitnessGoalToRC() {
    CmdUpdateFitnessGoalToRC *cmd = (CmdUpdateFitnessGoalToRC *)mCommands.front();
    mCurrJob = new UpdateFitnessGoalJob(this, cmd->profile);
    TheRockCentral.ManageJob(mCurrJob);
}

void FitnessGoalMgr::StartCmdDeleteFitnessGoalFromRC() {
    CmdDeleteFitnessGoalFromRC *cmd = (CmdDeleteFitnessGoalFromRC *)mCommands.front();
    mCurrJob = new DeleteFitnessGoalJob(this, cmd->profile);
    TheRockCentral.ManageJob(mCurrJob);
}

void FitnessGoalMgr::HandleCmdChangeProfileOnlineID() {
    MILO_LOG("===== HandleCmdChangeProfileOnlineID\n");
    mCurrProfileOnlineIDs = ((CmdChangeProfileOnlineID *)mCommands.front())->onlineID;
    RELEASE(mCommands.front());
    mCommands.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdDeleteFitnessGoalFromRC() {
    mCurrJob = nullptr;
    RELEASE(mCommands.front());
    mCommands.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdUpdateFitnessGoalToRC() {
    MILO_LOG("===== HandleCmdUpdateFitnessGoalToRC\n");
    mCurrJob = nullptr;
    {
        DataNode fitness("fitness");
        DataNode updated("updated");
        ThePlatformMgr.SmartGlassSend(0, DataArrayPtr(updated, fitness));
    } // need this bc for some reason the DataArrayPtr releases here
    RELEASE(mCommands.front());
    mCommands.pop_front();
    if (mUploadingProfile) {
        mUploadingProfile->ClearFitnessGoalNeedUpload();
    }
    if (!mPendingProfiles.empty()) {
        UploadNextProfile();
    } else {
        mUploadingProfile = nullptr;
    }
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdSendFitnessGoalToRC() {
    MILO_LOG("===== HandleCmdSendFitnessGoalToRC\n");
    mCurrJob = nullptr;
    {
        DataNode fitness("fitness");
        DataNode updated("updated");
        ThePlatformMgr.SmartGlassSend(0, DataArrayPtr(updated, fitness));
    }
    RELEASE(mCommands.front());
    mCommands.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::HandleCmdGetFitnessGoalFromRC() {
    HamProfile *pActiveProfile = TheProfileMgr.GetActiveProfile(true);
    if (pActiveProfile && pActiveProfile->IsSignedIn()) {
        if (ThePlatformMgr.IsSignedIntoLive(pActiveProfile->GetPadNum())
            && TheRockCentral.IsOnline()) {
            if (!(mCurrProfileName != pActiveProfile->GetName())) { // idk it says it
                                                                    // needs the
                // != operator
                MILO_LOG("===== [SUCCESS] HandleCmdGetFitnessGoalFromRC\n");
                GetFitnessGoalJob *pJob = ((GetFitnessGoalJob *)mCurrJob);
                pJob->GetFitnessGoal(pActiveProfile);
                BroadcastSyncMsg("fitness_goal_synced");
                SendPassiveMsg("fitness_synced_with_rc");
                goto tag;
            }
        }
    }
    MILO_LOG("===== [FAIL] HandleCmdGetFitnessGoalFromRC\n");
    BroadcastSyncMsg("sync_failed");

tag: // stupid tag
    mCurrJob = nullptr;
    RELEASE(mCommands.front());
    mCommands.pop_front();
    ProcessNextCommand();
}

void FitnessGoalMgr::QueueCmdGetFitnessGoalFromRC() {
    CmdGetFitnessGoalFromRC *cmd = new CmdGetFitnessGoalFromRC();
    mCommands.push_back(cmd);
    if (!mIsProccessingCmd) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdSendFitnessGoalToRC(HamProfile *profile) {
    CmdSendFitnessGoalToRC *cmd = new CmdSendFitnessGoalToRC(profile);
    mCommands.push_back(cmd);
    if (!mIsProccessingCmd) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdUpdateFitnessGoalToRC(HamProfile *profile) {
    CmdUpdateFitnessGoalToRC *cmd = new CmdUpdateFitnessGoalToRC(profile);
    mCommands.push_back(cmd);
    if (!mIsProccessingCmd) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdDeleteFitnessGoalFromRC(HamProfile *profile) {
    CmdDeleteFitnessGoalFromRC *cmd = new CmdDeleteFitnessGoalFromRC(profile);
    mCommands.push_back(cmd);
    if (!mIsProccessingCmd) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::QueueCmdChangeProfileOnlineID(String str) {
    mCommands.push_back(new CmdChangeProfileOnlineID(str));
    if (!mIsProccessingCmd) {
        ProcessNextCommand();
    }
}

void FitnessGoalMgr::OnSendFitnessGoalToRC(HamProfile *profile) {
    if (!profile) {
        return;
    }
    QueueCmdSendFitnessGoalToRC(profile);
}

void FitnessGoalMgr::DeleteFitnessGoalFromRC(HamProfile *profile) {
    if (!profile) {
        return;
    }
    QueueCmdDeleteFitnessGoalFromRC(profile);
}

DataNode FitnessGoalMgr::OnMsg(const SmartGlassMsg &msg) {
    MILO_LOG("SmartGlass: I should update fitness goal from RC\n");
    SendDataPoint("smartglass/fitness");
    QueueCmdGetFitnessGoalFromRC();
    return 1;
}

void FitnessGoalMgr::OnSmartGlassListen(int i) {
    if (i != 0) {
        ThePlatformMgr.AddSink(this, "smart_glass_msg");
    } else {
        ThePlatformMgr.RemoveSink(this, "smart_glass_msg");
    }
}

bool FitnessGoalMgr::IsProfileChanged() {
    HamProfile *pActiveProfile = TheProfileMgr.GetActiveProfile(true);
    return mCurrProfileName != (pActiveProfile ? pActiveProfile->GetName() : gNullStr);
}

void FitnessGoalMgr::AddPendingProfile(HamProfile *profile) {
    MILO_ASSERT(profile, 0x1dc);
    bool check = false;
    FOREACH (it, mPendingProfiles) {
        if (*it == profile) {
            check = true;
            break;
        }
    }
    if (!check) {
        mPendingProfiles.push_back(profile);
    }
}

void FitnessGoalMgr::ProcessNextCommand() {
    if (mCommands.size() == 0) {
        mIsProccessingCmd = false;
    } else {
        mIsProccessingCmd = true;
        switch (mCommands.front()->GetType()) {
        case 0:
            HandleCmdChangeProfileOnlineID();
            break;
        case 1:
            StartCmdGetFitnessGoalFromRC();
            break;
        case 2:
            StartCmdSendFitnessGoalToRC();
            break;
        case 3:
            StartCmdDeleteFitnessGoalFromRC();
            break;
        case 4:
            StartCmdUpdateFitnessGoalToRC();
            break;
        }
    }
}

void FitnessGoalMgr::SendPassiveMsg(Symbol s) {
    static Symbol p1("p1");
    static Symbol p2("p2");
    static Symbol none("none");

    Symbol player = none;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x45);
        if (playerData->GetPlayerName() == mCurrProfileName) {
            player = i == 0 ? p1 : p2;
            break;
        }
    }

    ThePassiveMessenger->TriggerGenericMsg(
        s, player, kPassiveMessageGeneral, gNullStr, -1
    );
}

DataNode FitnessGoalMgr::OnMsg(const RCJobCompleteMsg &msg) {
    if (!msg.Success()) {
        MILO_ASSERT(!mCommands.empty(), 0x163);
        switch (mCommands.front()->GetType()) {
        case 1:
            MILO_LOG(
                "[FitnessGoalMgr::OnMsg] Fitness Goal net API ==kCmdGetFitnessGoalFromRC== failed.\n"
            );
            break;
        case 2:
            MILO_LOG(
                "[FitnessGoalMgr::OnMsg] Fitness Goal net API ==kCmdSendFitnessGoalToRC== failed.\n"
            );
            break;
        case 3:
            MILO_LOG(
                "[FitnessGoalMgr::OnMsg] Fitness Goal net API ==kCmdDeleteFitnessGoalFromRC== failed.\n"
            );
            break;
        case 4:
            MILO_LOG(
                "[FitnessGoalMgr::OnMsg] Fitness Goal net API ==kCmdUpdateFitnessGoalToRC== failed.\n"
            );
            break;
        }
        mCurrJob = nullptr;
        BroadcastSyncMsg("sync_failed");
        RELEASE(mCommands.front());
        mCommands.pop_front();
        ProcessNextCommand();
        return 1;
    } else {
        if (msg.Job() == mCurrJob) {
            switch (mCommands.front()->GetType()) {
            case 1:
                HandleCmdGetFitnessGoalFromRC();
                break;
            case 2:
                HandleCmdSendFitnessGoalToRC();
                break;
            case 3:
                HandleCmdDeleteFitnessGoalFromRC();
                break;
            case 4:
                HandleCmdUpdateFitnessGoalToRC();
                break;
            }
        }
        return 1;
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
