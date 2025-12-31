#include "meta_ham/AppMiniLeaderboardDisplay.h"
#include "flow/Flow.h"
#include "hamobj/HamList.h"
#include "hamobj/MiniLeaderboardDisplay.h"
#include "meta/SongMgr.h"
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
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListWidget.h"
#include "utl/Symbol.h"

AppMiniLeaderboardDisplay::AppMiniLeaderboardDisplay()
    : unk60(0), mLeaderboardList(), unk68(0), unk6c(0) {}

AppMiniLeaderboardDisplay::~AppMiniLeaderboardDisplay() {
    TheRockCentral.CancelOutstandingCalls(this);
}

int AppMiniLeaderboardDisplay::NumData() const { return unk70.size(); }

void AppMiniLeaderboardDisplay::Exit() {
    UIComponent::Exit();
    TheServer.RemoveSink(this);
    TheRockCentral.CancelOutstandingCalls(this);
    unk68 = 0;
    unk6c = 0;
}

void AppMiniLeaderboardDisplay::Poll() {
    UIComponent::Poll();
    if (unk68 != 0) {
        if (unk60 == 0) {
            float uiSeconds = TheTaskMgr.UISeconds();
            if (unk6c > uiSeconds)
                unk6c = uiSeconds;
            if (1.0f <= uiSeconds - unk6c) {
                UpdateLeaderboardOnline(unk68);
            }
        } else if (unk60 == 5 && ThePlatformMgr.IsConnected()) {
            Symbol name = TheSongMgr.GetShortNameFromSongID(unk68);
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

void AppMiniLeaderboardDisplay::UpdateData(GetMiniLeaderboardJob *job) {
    unk70.clear();
    job->GetRows(&unk70);
    UpdateSelfInRows();
    MILO_ASSERT(mLeaderboardList, 0x15f);
    mLeaderboardList->Refresh(false);
}

void AppMiniLeaderboardDisplay::ClearData() {
    unk70.clear();
    MILO_ASSERT(mLeaderboardList, 0xfa);
    mLeaderboardList->Refresh(false);
}

DataNode AppMiniLeaderboardDisplay::OnMsg(ServerStatusChangedMsg const &) {
    if (unk68 != 0) {
        Symbol name = TheSongMgr.GetShortNameFromSongID(unk68);
        UpdateLeaderboard(name);
    }
    return 6;
}

void AppMiniLeaderboardDisplay::Update() {
    Init(); // unsure if correct call, doesnt give me much info
    MILO_ASSERT(mResourceDir, 0x16a);
    static Symbol leaderboard("leaderboard");
    HamList *pLeaderboardList = mResourceDir->Find<HamList>("leaderboard.lst");
    mLeaderboardList = pLeaderboardList;
    mLeaderboardList->SetProvider(this);
}

BEGIN_HANDLERS(AppMiniLeaderboardDisplay)
    HANDLE_EXPR(update_leaderboard, UpdateLeaderboard(_msg->Sym(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(ServerStatusChangedMsg)
    HANDLE_SUPERCLASS(MiniLeaderboardDisplay)
END_HANDLERS
