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
#include "ui/UI.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Leaderboards::Leaderboards()
    : mMaxDownload(100), mReadingScores(0), mType(kScore), mMode(kLeaderboardsFriends) {
    mUploadingProfile = nullptr;
    mHideRank = false;
    mGetLeaderboardJob = nullptr;
    mEthernetDisconnected = false;
    SetName("leaderboards", ObjectDir::Main());
}

Leaderboards::~Leaderboards() {}

BEGIN_HANDLERS(Leaderboards)
    HANDLE_ACTION(download_scores, DownloadScores(_msg->Sym(2)))
    HANDLE_EXPR(show_gamercard, ShowGamercard(_msg->Int(2), _msg->Obj<HamProfile>(3)))
    HANDLE_EXPR(get_type, mType)
    HANDLE_ACTION(set_type, SetType(_msg->Int(2)))
    HANDLE_EXPR(get_mode, mMode)
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
            label->SetTokenFmt(gamertag, mRows[data].gamertag);
        } else if (slot->Matches("score")) {
            label->SetInt(mRows[data].score, false);
        } else if (slot->Matches("no_flashcards")) {
            static Symbol no_flashcards_icon("no_flashcards_icon");
            if (mRows[data].noFlashcards) {
                label->SetTextToken(no_flashcards_icon);
            } else {
                label->SetTextToken(gNullStr);
            }
        } else if (slot->Matches("rank")) {
            if (mHideRank) {
                label->SetTextToken(gNullStr);

            } else {
                if (mRows[data].isPercentile && mMode != kLeaderboardsFriends) {
                    static char sBuffer[20];
                    Hx_snprintf(sBuffer, 20, "%d%% ", mRows[data].rank);
                    String str58(sBuffer);
                    label->SetTextToken(str58.c_str());
                } else {
                    static Symbol rank_fmt("rank_fmt");
                    label->SetInt(mRows[data].rank, false);
                }
            }
        } else if (slot->Matches("difficulty")) {
            static Symbol beginner_short("beginner_short");
            static Symbol easy_short("easy_short");
            static Symbol medium_short("medium_short");
            static Symbol expert_short("expert_short");
            Difficulty d = mRows[data].diff;
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
                    mRows[data].gamertag,
                    mRows[data].rank
                );
                break;
            }
        }
    } else {
        label->SetTextToken(gNullStr);
    }
}

int Leaderboards::NumData() const {
    if (!mReadingScores) {
        return mRows.size();
    } else {
        return 0;
    }
}

void Leaderboards::SetType(int t) {
    if (!mReadingScores) {
        mType = (LeaderboardsType)t;
    }
}

void Leaderboards::SetMode(int m) {
    if (!mReadingScores) {
        mMode = (LeaderboardsMode)m;
    }
}

void Leaderboards::ClearCache() {
    mCache.clear();
    mEthernetDisconnected = false;
}

void Leaderboards::DownloadScores(Symbol shortname) {
    int songID = 0;
    if (mType != kInfPlaylistScore && mType != kInfPlaylistTime) {
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
    FOREACH (it, mRows) {
        if (it->xuid == theXUID) {
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
    for (auto it = mRows.begin(); it != mRows.end(); ++it, ++idx) {
        if (i1 == idx && it->xuid == theXUID) {
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
    if (!mEthernetDisconnected && !ThePlatformMgr.IsConnected()) {
        UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("leaderboards.panel");
        if (panel->GetState() == UIPanel::kUp) {
            static Message ethernetDisconnectedMsg("leaderboards_ethernet_disconnected");
            TheUI->Handle(ethernetDisconnectedMsg, true);
            mEthernetDisconnected = true;
        }
    }
}

void Leaderboards::UploadNextScore() {
    static Symbol ham3("ham3");
    mScoreData.mSongData = &mScoreList.front();
    mScoreData.mProfile = mUploadingProfile;
    mScoreData.mCareerScore =
        mUploadingProfile->GetSongStatusMgr()->CalculateTotalScore(gNullStr);
    mScoreData.mCappedCareerScore =
        mUploadingProfile->GetSongStatusMgr()->CalculateTotalScore(ham3);
    TheRockCentral.ManageJob(
        new RecordScoreJob(this, mScoreData, mScoreData.mSongData->mSongID, true)
    );
}

void Leaderboards::PostProcScores() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    static Message msg("set_focus", 0);
    UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("leaderboards_panel");
    bool b1 = false;
    if (profile) {
        for (int i = 1; i < mRows.size(); i++) {
            LeaderboardRow row = mRows[i];
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
    while (!mPendingProfiles.empty()) {
        mUploadingProfile = mPendingProfiles.front();
        mPendingProfiles.pop_front();
        SongStatusMgr *mgr = mUploadingProfile->GetSongStatusMgr();
        mgr->GetScoresToUpload(mScoreList);
        if (!mScoreList.empty()) {
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
            if (mUploadingProfile && mUploadingProfile != profile) {
                AddPendingProfile(profile);
            } else {
                mUploadingProfile = profile;
                mScoreList.clear();
                profile->GetSongStatusMgr()->GetScoresToUpload(mScoreList);
                if (!mScoreList.empty()) {
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
    if ((0 <= i) && (i <= mRows.size())) {
        if (ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())) {
            if (mRows.size() != 0) {
                const OnlineID id(mRows[i].xuid);
                ShowGamercardResult result =
                    ThePlatformMgr.ShowGamercardForPadNum(profile->GetPadNum(), &id);
                if (result == (ShowGamercardResult)-2) {
                    static Symbol display_gamercard_privilege_error(
                        "display_gamercard_privilege_error"
                    );
                    return display_gamercard_privilege_error;
                } else if (result == (ShowGamercardResult)-3) {
                    return display_gamercard_pad_error;
                } else if (result < 0) {
                    static Symbol on_select_gamertag_error("on_select_gamertag_error");
                    return on_select_gamertag_error;
                }
            }
            return gNullStr;
        }
    }
    return display_gamercard_pad_error;
}

DataNode Leaderboards::OnMsg(const RCJobCompleteMsg &msg) {
    if (msg.Job() == mGetLeaderboardJob) {
        ReadScoresComplete(false, msg.Success() != 0);
    } else {
        if (msg.Success() && !mScoreList.empty()) {
            SongStatusData data = mScoreList.front();
            if (mUploadingProfile) {
                mUploadingProfile->GetSongStatusMgr()->ClearNeedUpload(
                    data.mSongID, data.mDiff
                );
            }
            mScoreList.pop_front();
            if (!mScoreList.empty()) {
                UploadNextScore();
            } else {
                if (!mPendingProfiles.empty()) {
                    StartUploadingNextProfile();
                } else {
                    mUploadingProfile = nullptr;
                }
            }
        }
    }
    return 1;
}

void Leaderboards::ReadScoresComplete(bool b1, bool b2) {
    mHideRank = false;
    if (!b1) {
        GetLeaderboardByPlayerJob *job = (GetLeaderboardByPlayerJob *)mGetLeaderboardJob;
        mRows.clear();
        job->GetRows(&mRows);
        mGetLeaderboardJob = nullptr;
        if (b2) {
            mCache.insert(std::make_pair((unsigned int)job->GetChecksum(), mRows));
        }
    }

    mReadingScores = false;
    PostProcScores();
    static Message leaderboardsLoadedMsg("leaderboards_loaded");
    static Message leaderboardsFailedRcMsg("leaderboards_failed_rc");
    static Message leaderboardsFailedLiveMsg("leaderboards_failed_live");
    if (b2) {
        TheUI->Handle(leaderboardsLoadedMsg, b2);
    } else if (ThePlatformMgr.IsConnected()) {
        TheUI->Handle(leaderboardsFailedRcMsg, b2);
    } else {
        TheUI->Handle(leaderboardsFailedLiveMsg, b2);
    }
}

void Leaderboards::GetScores(int songID) {
    if (!mReadingScores && !mGetLeaderboardJob) {
        mRows.clear();
        HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
        if (!ThePlatformMgr.IsSignedIntoLive(activeProfile->GetPadNum())) {
            static Message leaderboardsFailedMsg("leaderboards_failed");
            TheUI->Handle(leaderboardsFailedMsg, false);
        } else {
            int temp = mType;
            mCurrentSongID = songID;
            mReadingScores = true;
            if (mType == kInfPlaylistScore || mType == kInfPlaylistTime) {
                temp = temp == 4 ? 0 : temp;
                if (mType == kInfPlaylistTime) {
                    temp = 1;
                }
                songID = 0x989a6e;
            }

            int padnum = activeProfile->GetPadNum();
            int index =
                (songID & ~0xFC000000) + (((((padnum << 2) + temp) << 2) + mMode) << 26);
            auto it = mCache.find(index);
            if (it == mCache.end()) {
                mGetLeaderboardJob = new GetLeaderboardByPlayerJob(
                    this, activeProfile, songID, temp, mMode, 10, index
                );
                TheRockCentral.ManageJob(mGetLeaderboardJob);
            } else {
                mRows = it->second;
                ReadScoresComplete(true, true);
            }
        }
    }
}
