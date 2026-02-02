#include "meta_ham/Leaderboards.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/LeaderboardJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "net_ham/ScoreJobs.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "stl/_pair.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

Leaderboards::Leaderboards() : unk7c(100), unk80(0), unk84(0), unk88(2) {
    unk54 = 0;
    unk94 = 0;
    unk98 = 0;
    unk9c = 0;
    SetName("leaderboards", ObjectDir::Main());
}

Leaderboards::~Leaderboards() {}

BEGIN_HANDLERS(Leaderboards)
    HANDLE_ACTION(download_scores, DownloadScores(_msg->Sym(2)))
    HANDLE_EXPR(show_gamercard, ShowGamercard(_msg->Int(2), _msg->Obj<HamProfile>(3)))
    HANDLE_EXPR(get_type, unk84)
    HANDLE_ACTION(set_type, SetType(_msg->Int(2)))
    HANDLE_EXPR(get_mode, unk88)
    HANDLE_ACTION(set_mode, SetMode(_msg->Int(2)))
    HANDLE_EXPR(num_scores, NumData())
    HANDLE_EXPR(has_self, HasSelf())
    HANDLE_EXPR(is_self, IsSelf(_msg->Int(2)))
    HANDLE_ACTION(clear_cache, ClearCache())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void Leaderboards::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    if (data < NumData()) {
        if (slot->Matches("gamertag")) {
            static Symbol gamertag("gamertag");
            label->SetTokenFmt(gamertag, unk58[data].unk0);
        } else if (slot->Matches("score")) {
            label->SetInt(unk58[data].unkc, false);
        } else if (slot->Matches("no_flashcards")) {
            static Symbol no_flashcards_icon("no_flashcards_icon");
            if (unk58[data].unk1c) {
                label->SetTextToken(no_flashcards_icon);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("rank")) {
            if (!unk94) {
                if (unk58[data].unk1d && unk88 != 2) {
                    static char sBuffer[20];
                    Hx_snprintf(sBuffer, 20, "%d%% ", unk58[data].unk10);
                    String str58(sBuffer);
                    label->SetTextToken(str58.c_str());
                } else {
                    static Symbol rank_fmt("rank_fmt");
                    label->SetInt(unk58[data].unk10, false);
                }
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("difficulty")) {
            static Symbol beginner_short("beginner_short");
            static Symbol easy_short("easy_short");
            static Symbol medium_short("medium_short");
            static Symbol expert_short("expert_short");
            Difficulty d = unk58[data].unk18;
            switch (d) {
            case kDifficultyEasy:
                label->SetTextToken(easy_short);
                break;
            case kDifficultyMedium:
                label->SetTextToken(medium_short);
                break;
            case kDifficultyExpert:
                label->SetTextToken(expert_short);
                break;
            case kDifficultyBeginner:
                label->SetTextToken(beginner_short);
                break;
            default:
                MILO_NOTIFY(
                    "Bad difficulty %d retrieved from leaderboards for user                    %s at rank %d!",
                    d,
                    unk58[data].unk0,
                    unk58[data].unk10
                );
                break;
            }
        }
    } else {
        label->SetTextToken(gNullStr);
    }
}

int Leaderboards::NumData() const {
    if (!unk80) {
        return unk58.size();
    } else {
        return 0;
    }
}

void Leaderboards::SetType(int t) {
    if (!unk80) {
        unk84 = t;
    }
}

void Leaderboards::SetMode(int m) {
    if (!unk80) {
        unk88 = m;
    }
}

void Leaderboards::ClearCache() {
    unk64.clear();
    unk9c = false;
}

void Leaderboards::DownloadScores(Symbol shortname) {
    int songID = 0;
    if (unk84 != 4 && unk84 != 5) {
        songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    }
    GetScores(songID);
}

bool Leaderboards::HasSelf() const {
    bool ret = false;
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1CB);
    pProfile->UpdateOnlineID();
    bool bHasOnlineID = pProfile->IsSignedIn();
    MILO_ASSERT(bHasOnlineID, 0x1CF);
    XUID theXUID = pProfile->GetOnlineID()->GetXUID();
    FOREACH (it, unk58) {
        if (it->unk20 == theXUID) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool Leaderboards::IsSelf(int i1) const {
    bool ret = false;
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1EA);
    pProfile->UpdateOnlineID();
    bool bHasOnlineID = pProfile->IsSignedIn();
    MILO_ASSERT(bHasOnlineID, 0x1EE);
    XUID theXUID = pProfile->GetOnlineID()->GetXUID();
    int idx = 0;
    for (auto it = unk58.begin(); it != unk58.end(); ++it, ++idx) {
        if (i1 == idx && it->unk20 == theXUID) {
            ret = true;
            break;
        }
    }
    return ret;
}

void Leaderboards::Init() {
    MILO_ASSERT(!TheLeaderboards, 0x27);
    TheLeaderboards = new Leaderboards();
}

void Leaderboards::Poll() {
    if (!unk9c && !ThePlatformMgr.IsConnected()) {
        UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("leaderboards.panel");
        if (panel->GetState() == UIPanel::kUp) {
            static Message ethernetDisconnectedMsg("leaderboards_ethernet_disconnected");
            TheUI->Handle(ethernetDisconnectedMsg, true);
            unk9c = true;
        }
    }
}

void Leaderboards::UploadNextScore() {
    static Symbol ham3("ham3");
    mRecordScoreData.mStatus = &unk30.front();
    mRecordScoreData.mProfile = unk54;
    mRecordScoreData.unkc = unk54->GetSongStatusMgr()->CalculateTotalScore(gNullStr);
    mRecordScoreData.unk10 = unk54->GetSongStatusMgr()->CalculateTotalScore(ham3);
    TheRockCentral.ManageJob(new RecordScoreJob(
        this, mRecordScoreData, mRecordScoreData.mStatus->mSongID, true
    ));
}

void Leaderboards::PostProcScores() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    static Message msg("set_focus", 0);
    UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("leaderboards_panel");
    bool b1 = false;
    if (profile) {
        for (int i = 1; i < unk58.size(); i++) {
            LeaderboardRow row = unk58[i];
            if (panel->GetState() == UIPanel::kUp) {
                msg[0] = i;
                panel->HandleType(msg);
                b1 = true;
            }
        }
    }
    if (!b1 && panel->GetState() == UIPanel::kUp) {
        msg[0] = 0;
        panel->HandleType(msg);
    }
}

void Leaderboards::AddPendingProfile(HamProfile *pProfile) {
    MILO_ASSERT(pProfile, 0x56);
    bool found = false;
    FOREACH (it, mPendingProfiles) {
        if (*it == pProfile) {
            found = true;
            break;
        }
    }
    if (!found) {
        mPendingProfiles.push_back(pProfile);
    }
}

void Leaderboards::StartUploadingNextProfile() {
    for (auto it = mPendingProfiles.begin(); it != mPendingProfiles.end(); ++it) {
        unk54 = mPendingProfiles.front();
        mPendingProfiles.pop_front();
        unk54->GetSongStatusMgr()->GetScoresToUpload(unk30);
        if (!unk30.empty()) {
            UploadNextScore();
            return;
        }
    }
}

void Leaderboards::UploadScores(HamProfile *profile) {
    profile->UpdateOnlineID();
    if (profile->IsSignedIn()) {
        if (ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())
            && !SongStatusMgr::sFakeLeaderboardUploadFailure) {
            if (unk54 && unk54 != profile) {
                AddPendingProfile(profile);
            } else {
                unk54 = profile;
                unk30.clear();
                profile->GetSongStatusMgr()->GetScoresToUpload(unk30);
                if (!unk30.empty()) {
                    UploadNextScore();
                } else {
                    StartUploadingNextProfile();
                }
            }
        }
    }
}

Symbol Leaderboards::ShowGamercard(int i, HamProfile *profile) {
    static Symbol display_gamercard_pad_error("display_gamercard_pad_error");
    if ((0 <= i) && (i <= unk58.size())) {
        if (ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
            if (unk58.size() != 0) {
                const OnlineID id(unk58[i].unk20);
                ShowGamercardResult result =
                    ThePlatformMgr.ShowGamercardForPadNum(profile->GetPadNum(), &id);
                if (result == (ShowGamercardResult)-2) {
                    static Symbol display_gamercard_privilege_error(
                        "display_gamercard_privilege_error"
                    );
                    return display_gamercard_privilege_error;
                } else if (result == (ShowGamercardResult)-3) {
                    return display_gamercard_pad_error;
                } else {
                    if (0 > result) {
                        static Symbol on_select_gamertag_error("on_select_gamertag_error");
                        return on_select_gamertag_error;
                    }
                }
            }
            return gNullStr;
        }
    }
    return display_gamercard_pad_error;
}

DataNode Leaderboards::OnMsg(const RCJobCompleteMsg &msg) {
    if (msg.Job() == unk98) {
        ReadScoresComplete(false, msg.Success() != 0);
    } else {
        if (msg.Success() && !unk30.empty()) {
            SongStatusData data = unk30.front();
            if (unk54) {
                unk54->GetSongStatusMgr()->ClearNeedUpload(data.mSongID, data.mDifficulty);
            }
            unk30.pop_front();
            if (!unk30.empty()) {
                UploadNextScore();
            } else {
                if (!mPendingProfiles.empty()) {
                    StartUploadingNextProfile();
                } else {
                    unk54 = nullptr;
                }
            }
        }
    }
    return 1;
}

void Leaderboards::ReadScoresComplete(bool b1, bool b2) {
    unk94 = false;
    if (!b1) {
        GetLeaderboardByPlayerJob *job = unk98;
        unk58.clear();
        job->GetRows(&unk58);
        unk98 = nullptr;
        if (b2) {
            auto it = std::make_pair((unsigned int)job->SongID(), unk58); // somethins up
            unk64.insert(it);
        }
    }

    unk80 = false;
    PostProcScores();
    static Message leaderboards_loaded("leaderboards_loaded");
    static Message leaderboards_failed_rc("leaderboards_failed_rc");
    static Message leaderboards_failed_live("leaderboards_failed_live");
    if (b2) {
        TheUI->Handle(leaderboards_loaded, b2);
    } else if (ThePlatformMgr.IsConnected()) {
        TheUI->Handle(leaderboards_failed_rc, b2);
    } else {
        TheUI->Handle(leaderboards_failed_live, b2);
    }
}

void Leaderboards::GetScores(int i) {
    if (!unk80 && !unk98) {
        unk58.clear();
        HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
        if (!ThePlatformMgr.IsSignedIntoLive(activeProfile->GetPadNum())) {
            static Message leaderboards_failed("leaderboards_failed");
            TheUI->Handle(leaderboards_failed, false);
        } else {
            unk90 = i;
            unk80 = true;
            if (unk84 == 1 || unk84 == 5) {
                i = 1000; // idk
            }
            auto it = unk64.find(unk88 + i); // idk about the param here
            if (it->second.empty()) {
                unk98 = new GetLeaderboardByPlayerJob(
                    this, activeProfile, i, unk84, unk88, 10, 0
                );
                TheRockCentral.ManageJob(unk98);
            } else {
                unk58 = it->second;
                ReadScoresComplete(true, true);
            }
        }
    }
}
