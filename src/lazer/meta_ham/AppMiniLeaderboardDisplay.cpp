#include "meta_ham/AppMiniLeaderboardDisplay.h"
#include "flow/Flow.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamList.h"
#include "hamobj/MiniLeaderboardDisplay.h"
#include "meta/SongMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongStatusMgr.h"
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
#include "ui/UIListLabel.h"
#include "ui/UIListWidget.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"

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
    } else {
        if (!ThePlatformMgr.IsConnected()) {
            if (unk60 != 5) {
                unk60 = 5;
                mResourceDir->Find<Flow>("no_profile.flow")->Activate();
            }
        } else {
            if (unk60 != 4) {
                unk60 = 4;
                mResourceDir->Find<Flow>("no_profile.flow")->Activate();
            }
        }
    }
}

bool AppMiniLeaderboardDisplay::UpdateLeaderboard(Symbol s) {
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
        if (!profile->IsSignedIn()) {
            if (unk60 != 4) {
                unk60 = 4;
                Flow *f = mResourceDir->Find<Flow>("no_profile.flow", true);
                f->Activate();
                return true;
            }
        } else if (!ThePlatformMgr.IsConnected()) {
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
    return true;
}

void AppMiniLeaderboardDisplay::UpdateSelfInRows() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        pProfile->UpdateOnlineID();
        bool bHasOnlineID = pProfile->IsSignedIn();
        MILO_ASSERT(bHasOnlineID, 0x107);
        XUID xuid = pProfile->GetOnlineID()->GetXUID();
        SongStatusMgr *pSongStatusMgr = pProfile->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0x10b);
        bool b = false;
        int score = pSongStatusMgr->GetScore(mSongID, b);
        Difficulty diff = pSongStatusMgr->GetDifficulty(mSongID);
        if (0 < score) {
            bool check = false;
            FOREACH (it, mLBRows) {
                if (it->unk20 == xuid && score > (unsigned int)it->unkc) {
                    mLBRows.erase(it);
                    check = true;
                    break;
                }
            }

            if (check) {
                LeaderboardRow row;
                row.unk20 = xuid;
                row.unkc = score;
                row.unk1d = false;
                row.unk10 = 0;
                row.unk1e = true;
                row.unk0 = pProfile->GetName();
                row.unk1c = b;
                row.unk18 = diff;
                bool b2 = false;
                FOREACH (it, mLBRows) {
                    if (score >= (unsigned int)it->unkc) {
                        mLBRows.insert(it, 1, row);
                        b2 = true;
                        break;
                    }
                }
                if (!b2) {
                    row.unk14 = 1;
                    mLBRows.push_back(row);
                }
                int count = 0;
                FOREACH (it, mLBRows) {
                    it->unk14 = count;
                    count++;
                }
            }
        }
    }
}

void AppMiniLeaderboardDisplay::Text(
    int i1, int data, UIListLabel *listLabel, UILabel *label
) const {
    if (data < NumData()) {
        String name = gNullStr;
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        if (pProfile) {
            name = pProfile->GetName();
        }

        if (listLabel->Matches("gamertag")) {
            static Symbol gamertag("gamertag");
            if (name == mLBRows[data].unk0) {
                label->SetTextToken(gNullStr);
            } else {
                label->SetTokenFmt(gamertag, mLBRows[data].unk0);
            }
        } else if (listLabel->Matches("score")) {
            label->SetInt(mLBRows[data].unkc, false);
        } else if (listLabel->Matches("no_flashcards")) {
            static Symbol no_flashcards_icon("no_flashcards_icon");
            if (mLBRows[data].unk1c) {
                label->SetTextToken(no_flashcards_icon);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (listLabel->Matches("rank")) {
            static Symbol rank_fmt("rank_fmt");
            label->SetInt(mLBRows[data].unk14, false);
        } else if (listLabel->Matches("difficulty")) {
            static Symbol beginner_short("beginner_short");
            static Symbol easy_short("easy_short");
            static Symbol medium_short("medium_short");
            static Symbol expert_short("expert_short");
            Difficulty diff = mLBRows[data].unk18;
            switch (diff) {
            case kDifficultyBeginner:
                label->SetTextToken(beginner_short);
                break;
            case kDifficultyEasy:
                label->SetTextToken(easy_short);
                break;
            case kDifficultyMedium:
                label->SetTextToken(medium_short);
                break;
            case kDifficultyExpert:
                label->SetTextToken(expert_short);
                break;
            default:
                MILO_NOTIFY(
                    "Bad difficulty %d retrieved from leaderboards for user                    %s at rank %d!", // yes this is what it should be
                    diff,
                    mLBRows[data].unk0,
                    mLBRows[data].unk10
                );
                break;
            }
        } else if (listLabel->Matches("self")) {
            static Symbol Gamertag("gamertag");
            if (name == mLBRows[data].unk0) {
                label->SetTokenFmt(Gamertag, mLBRows[data].unk0);
            } else {
                label->SetTextToken(gNullStr);
            }
        }
    } else {
        label->SetTextToken(gNullStr);
    }
}
