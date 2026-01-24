#include "meta_ham/AppMiniLeaderboardDisplay.h"
#include "flow/Flow.h"
#include "hamobj/HamList.h"
#include "hamobj/MiniLeaderboardDisplay.h"
#include "meta/SongMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/ProfileMgr.h"
#include "net/DingoSvr.h"
#include "net_ham/LeaderboardJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "rndobj/Dir.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListWidget.h"
#include "utl/Symbol.h"

AppMiniLeaderboardDisplay::AppMiniLeaderboardDisplay()
    : unk60(0), mLeaderboardList(0), mSongID(0), unk6c(0) {}

AppMiniLeaderboardDisplay::~AppMiniLeaderboardDisplay() {
    TheRockCentral.CancelOutstandingCalls(this);
}

BEGIN_HANDLERS(AppMiniLeaderboardDisplay)
    HANDLE_EXPR(update_leaderboard, UpdateLeaderboard(_msg->Sym(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(ServerStatusChangedMsg)
    HANDLE_SUPERCLASS(MiniLeaderboardDisplay)
END_HANDLERS

void AppMiniLeaderboardDisplay::Poll() {
    UIComponent::Poll();
    if (mSongID != 0) {
        if (unk60 == 0) {
            float uiSeconds = TheTaskMgr.UISeconds();
            if (unk6c > uiSeconds)
                unk6c = uiSeconds;
            if (1.0f <= uiSeconds - unk6c) {
                UpdateLeaderboardOnline(mSongID);
            }
        } else if (unk60 == 5 && ThePlatformMgr.IsConnected()) {
            Symbol name = TheSongMgr.GetShortNameFromSongID(mSongID);
            UpdateLeaderboard(name);
        }
    }
}

void AppMiniLeaderboardDisplay::Enter() {
    UIComponent::Enter();
    TheServer.AddSink(this);
    if (unk60 != 0) {
        unk60 = 0;
        Flow *f = mResourceDir->Find<Flow>("pending.flow");
        f->Activate();
    }
}

void AppMiniLeaderboardDisplay::Exit() {
    UIComponent::Exit();
    TheServer.RemoveSink(this);
    TheRockCentral.CancelOutstandingCalls(this);
    mSongID = 0;
    unk6c = 0;
}

void AppMiniLeaderboardDisplay::DrawShowing() {
    MILO_ASSERT(mResourceDir, 0x5C);
    mResourceDir->SetWorldXfm(WorldXfm());
    mResourceDir->Draw();
}

int AppMiniLeaderboardDisplay::NumData() const { return mLBRows.size(); }

UIListWidgetState
AppMiniLeaderboardDisplay::ElementStateOverride(int, int data, UIListWidgetState s) const {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        profile->UpdateOnlineID();
        bool bHasOnlineID = profile->IsSignedIn();
        MILO_ASSERT(bHasOnlineID, 500);
        XUID xuid = profile->GetOnlineID()->GetXUID();
        if (mLBRows[data].unk20 == xuid) {
            return kUIListWidgetHighlight;
        }
    }
    return kUIListWidgetActive;
}

void AppMiniLeaderboardDisplay::UpdateData(GetMiniLeaderboardJob *job) {
    mLBRows.clear();
    job->GetRows(&mLBRows);
    UpdateSelfInRows();
    MILO_ASSERT(mLeaderboardList, 0x15f);
    mLeaderboardList->Refresh(false);
}

void AppMiniLeaderboardDisplay::ClearData() {
    mLBRows.clear();
    MILO_ASSERT(mLeaderboardList, 0xfa);
    mLeaderboardList->Refresh(false);
}

DataNode AppMiniLeaderboardDisplay::OnMsg(const ServerStatusChangedMsg &) {
    if (mSongID != 0) {
        Symbol name = TheSongMgr.GetShortNameFromSongID(mSongID);
        UpdateLeaderboard(name);
    }
    return DATA_UNHANDLED;
}

DataNode AppMiniLeaderboardDisplay::OnMsg(const RCJobCompleteMsg &msg) {
    if (msg.Success()) {
        GetMiniLeaderboardJob *job = dynamic_cast<GetMiniLeaderboardJob *>(msg.Job());
        if (job && job->SongID() == mSongID) {
            UpdateData(job);
        }
        if (unk60 != 1) {
            unk60 = 1;
            mResourceDir->Find<Flow>("ready.flow")->Activate();
        }
    } else if (unk60 != 3) {
        unk60 = 3;
        mResourceDir->Find<Flow>("connection_error.flow")->Activate();
    }
    return 1;
}

void AppMiniLeaderboardDisplay::Update() {
    MiniLeaderboardDisplay::Update();
    MILO_ASSERT(mResourceDir, 0x16a);
    static Symbol leaderboard("leaderboard");
    HamList *pLeaderboardList = mResourceDir->Find<HamList>("leaderboard.lst");
    mLeaderboardList = pLeaderboardList;
    mLeaderboardList->SetProvider(this);
}

void AppMiniLeaderboardDisplay::UpdateLeaderboardOnline(int i1) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile && profile->IsSignedIn() && ThePlatformMgr.IsConnected()) {
        TheRockCentral.ManageJob(new GetMiniLeaderboardJob(this, profile, i1));
        if (unk60 != 2) {
            unk60 = 2;
            mResourceDir->Find<Flow>("pending.flow")->Activate();
        }
    } else if (!ThePlatformMgr.IsConnected()) {
        if (unk60 != 5) {
            unk60 = 5;
            mResourceDir->Find<Flow>("no_profile.flow")->Activate();
        } else if (unk60 != 4) {
            unk60 = 4;
            mResourceDir->Find<Flow>("no_profile.flow")->Activate();
        }
    }
}

bool AppMiniLeaderboardDisplay::UpdateLeaderboard(Symbol s) { // has one small discrepancy
    if (!TheProfileMgr.HasActiveProfile(true)) {
        if (unk60 != 4) {
            unk60 = 4;
            Flow *f = mResourceDir->Find<Flow>("no_profile.flow", true);
            f->Activate();
            return true;
        }
    } else {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(profile, 0xb1);
        profile->UpdateOnlineID();
        if (profile->IsSignedIn()) {
            if (!ThePlatformMgr.IsConnected()) { // mismatch right here?
                if (unk60 != 5) {
                    unk60 = 5;
                    Flow *f = mResourceDir->Find<Flow>("no_profile.flow", true);
                    f->Activate();
                    return true;
                }
            } else {
                mSongID = TheSongMgr.GetSongIDFromShortName(s, false);
                ClearData();
                TheRockCentral.CancelOutstandingCalls(this);
                if (mSongID == 0) {
                    return true;
                }
                if (unk60 != 0) {
                    unk60 = 0;
                    Flow *f = mResourceDir->Find<Flow>("pending.flow", true);
                    f->Activate();
                }
                unk6c = TheTaskMgr.UISeconds();
            }
        }
    }
    return true;
}
