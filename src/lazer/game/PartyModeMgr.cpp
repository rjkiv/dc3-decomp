#include "game/PartyModeMgr.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongRecord.h"
#include "meta_ham/Utl.h"
#include "net_ham/PartyModeJobs.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "ui/UI.h"
#include "utl/DataPointMgr.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"
#include <cstdlib>

PartyModeMgr *ThePartyModeMgr;
int gRematchCount;

namespace {
    int GetEnumFromModeName(Symbol mode) {
        static Symbol bustamove("bustamove");
        static Symbol perform("perform");
        static Symbol dance_battle("dance_battle");
        static Symbol rhythm_battle("rhythm_battle");
        static Symbol strike_a_pose("strike_a_pose");
        if (mode == bustamove) {
            return 3;
        } else if (mode == perform) {
            return 0;
        } else if (mode == dance_battle) {
            return 1;
        } else if (mode == rhythm_battle) {
            return 2;
        } else if (mode == strike_a_pose) {
            return 4;
        } else {
            return 0x20;
        }
    }

    Symbol GetModeNameFromEnum(int enumType) {
        static Symbol bustamove("bustamove");
        static Symbol perform("perform");
        static Symbol dance_battle("dance_battle");
        static Symbol rhythm_battle("rhythm_battle");
        static Symbol strike_a_pose("strike_a_pose");
        switch (enumType) {
        case 0:
            return perform;
        case 1:
            return dance_battle;
        case 2:
            return rhythm_battle;
        case 3:
            return bustamove;
        case 4:
            return strike_a_pose;
        default:
            MILO_ASSERT(0, 0x49);
            return gNullStr;
        }
    }
}

#pragma region PartyModeARObject

const char *PartyModeARObject::GetTexPath() {
    const char *texPath = gNullStr;
    static Symbol image_path("image_path");
    DataArray *pathArr = unk8->FindArray(image_path);
    if (pathArr) {
        texPath = pathArr->Str(1);
    }
    return texPath;
}

#pragma endregion
#pragma region PartyModePlayer

PartyModePlayer::PartyModePlayer(PartyModeARObject *obj) : unk0(obj), unk14(0) {
    unk10 = new DataArray(3);
}

PartyModePlayer::~PartyModePlayer() {
    RELEASE(unk0);
    unk10->Release();
}

void PartyModePlayer::PushTitle(Symbol s) {
    unk8.push_back(s);
    if (unk8.size() > 3) {
        unk8.pop_front();
    }
    int idx = 0;
    for (auto it = unk8.begin(); it != unk8.end(); ++it, ++idx) {
        unk10->Node(idx) = *it;
    }
}

#pragma endregion
#pragma region PartyModeMgr

PartyModeMgr::PartyModeMgr() : mFrameSmoothers() {
    unk40 = false;
    unk328 = 0;
    static Symbol party_mode("party_mode");
    mPartyModeCfg = SystemConfig(party_mode);
    static Symbol ar_objects("ar_objects");
    mARObjects = mPartyModeCfg->FindArray(ar_objects);
    static Symbol good_titles("good_titles");
    mGoodTitles = mPartyModeCfg->FindArray(good_titles);
    static Symbol bad_titles("bad_titles");
    mBadTitles = mPartyModeCfg->FindArray(bad_titles);
    static Symbol event_scoring("event_scoring");
    mEventScoring = mPartyModeCfg->FindArray(event_scoring);
    mUsePlaytestData = false;
    mPartyModePlaytestEvents = nullptr;
    static Symbol party_mode_playtest_data("party_mode_playtest_data");
    mPartyModePlaytestData = mPartyModeCfg->FindArray(party_mode_playtest_data);
    if (mPartyModePlaytestData) {
        static Symbol use_playtest_data("use_playtest_data");
        DataArray *useData = mPartyModePlaytestData->FindArray(use_playtest_data);
        if (useData && useData->Int(1) != 0) {
            mUsePlaytestData = true;
        }
    }
    if (mUsePlaytestData) {
        static Symbol party_mode_playtest_events("party_mode_playtest_events");
        mPartyModePlaytestEvents =
            mPartyModePlaytestData->FindArray(party_mode_playtest_events);
    }
    std::vector<Symbol> vec;
    int numGoodTitles = mGoodTitles->Size();
    vec.resize(numGoodTitles);
    for (int i = 1; i < numGoodTitles; i++) {
        vec[i - 1] = mGoodTitles->Sym(i);
    }
    mGoodTitlePicker.AddItems(vec);
    mGoodTitlePicker.SetMode(0);
    int numBadTitles = mBadTitles->Size();
    vec.resize(numBadTitles);
    for (int i = 1; i < numBadTitles; i++) {
        vec[i - 1] = mBadTitles->Sym(i);
    }
    mBadTitlePicker.AddItems(vec);
    mBadTitlePicker.SetMode(0);
    int numARObjects = mARObjects->Size() - 1;
    for (int i = 1; i <= numARObjects; i++) {
        unkb0.push_back(i);
    }
    for (int i = 0; i < numARObjects; i++) {
        int randIdx = rand() % numARObjects;
        int old = unkb0[i];
        unkb0[i] = unkb0[randIdx];
        unkb0[randIdx] = old;
    }
    mCurrEvent = nullptr;
    InitCharacters();
    for (int i = 0; i < 6; i++) {
        mFrameSmoothers[i].SetSmoothParameters(10, 1);
        mFrameSmoothers[i].ForceValue(Vector2(0.5, 0.5));
    }
    mDifficulty = DefaultDifficulty();
    mPlaylist = 0;
    mIsPlaylistShuffled = false;
    unk2dc = -1;
    mUseFullLengthSongs = false;
    static DataNode &n = DataVariable("force_song_shortening_off");
    if (n.Int()) {
        mUseFullLengthSongs = true;
    }
    mPerSongDifficulty = false;
    mCustomParty = false;
    mUsingPerSongOptions = false;
    mSetPartyOptionsJob = nullptr;
    mGetPartyOptionsJob = nullptr;
    mGetPartySongQueueJob = nullptr;
    mAddSongToPartySongQueueJob = nullptr;
    mDeleteSongFromPartySongQueueJob = nullptr;
    unk314 = false;
    unk324 = 0;
}

PartyModeMgr::~PartyModeMgr() { ResetPlayers(); }

BEGIN_HANDLERS(PartyModeMgr)
    HANDLE_ACTION(add_player_to_team, AddPlayerToTeam(_msg->Int(2)))
    HANDLE_ACTION(finalize_team, FinalizeTeam(_msg->Int(2)))
    HANDLE_ACTION(clear_team, ClearTeam(_msg->Int(2)))
    HANDLE_ACTION(finalize_party, FinalizeParty())
    HANDLE_ACTION(
        store_player_frame_pos,
        StorePlayerFramePos(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(
        store_player_frame_scale, StorePlayerFrameScale(_msg->Int(2), _msg->Float(3))
    )
    HANDLE_EXPR(get_tex_path, GetPlayerARTexPath(_msg->Int(2)))
    HANDLE_EXPR(num_enrolled, (int)mPlayers.size())
    HANDLE_EXPR(num_enrolled_team_1, (int)mTeam1Players.size())
    HANDLE_EXPR(num_enrolled_team_2, (int)mTeam2Players.size())
    HANDLE_EXPR(get_curr_event_name, GetCurrEventName())
    HANDLE_EXPR(get_curr_event_display_name, GetCurrEventDisplayName())
    HANDLE_EXPR(get_curr_event_microgame_name, GetCurrEventMicrogameName())
    HANDLE_EXPR(get_curr_event_song_name, GetCurrEventSongName())
    HANDLE_EXPR(get_curr_event_song_shortname, GetCurrEventSongShortName())
    HANDLE_EXPR(get_curr_event_player_flags, GetCurrEventPlayerFlags())
    HANDLE_EXPR(get_curr_event_num_players, GetCurrEventNumPlayers())
    HANDLE_EXPR(get_curr_event_artist_name, GetCurrEventSongArtistName())
    HANDLE_EXPR(get_curr_event_players, GetCurrEventPlayers())
    HANDLE_ACTION(update_curr_event, SetCurrEvent())
    HANDLE_ACTION(update_rounds_played, UpdateRoundsPlayed())
    HANDLE_EXPR(get_max_participants, 8)
    HANDLE_ACTION(set_random_characters, SetRandomCharacters())
    HANDLE_ACTION(setup_character_data, SetupCharacterData())
    HANDLE_ACTION(reset_party, ResetParty())
    HANDLE_ACTION(crew_showdown_rematch, CrewShowdownRematch())
    HANDLE_EXPR(get_left_player_index, GetLeftPlayerIndex())
    HANDLE_EXPR(get_right_player_index, GetRightPlayerIndex())
    HANDLE_ACTION(inc_left_player_score, IncLeftPlayerScore(_msg->Int(2)))
    HANDLE_ACTION(inc_right_player_score, IncRightPlayerScore(_msg->Int(2)))
    HANDLE_EXPR(get_player_photo_index, GetPlayerPhotoIndex(_msg->Int(2)))
    HANDLE_ACTION(push_left_player_title, PushLeftPlayerTitle(_msg->Sym(2)))
    HANDLE_ACTION(push_right_player_title, PushRightPlayerTitle(_msg->Sym(2)))
    HANDLE_EXPR(is_showdown, mIsShowdown)
    HANDLE_EXPR(is_team_signed_in, IsTeamSignedIn(_msg->Int(2)))
    HANDLE_ACTION(set_left_team_score, SetLeftTeamScore(_msg->Float(2)))
    HANDLE_ACTION(set_right_team_score, SetRightTeamScore(_msg->Float(2)))
    HANDLE_ACTION(inc_left_team_score, IncLeftTeamScore(_msg->Float(2)))
    HANDLE_ACTION(inc_right_team_score, IncRightTeamScore(_msg->Float(2)))
    HANDLE_EXPR(get_left_team_score, mLeftTeamScore)
    HANDLE_EXPR(get_right_team_score, mRightTeamScore)
    HANDLE_EXPR(get_left_team_prev_score, mLeftTeamPrevScore)
    HANDLE_EXPR(get_right_team_prev_score, mRightTeamPrevScore)
    HANDLE_ACTION(start_new_round, StartNewRound())
    HANDLE_ACTION(
        smooth_frame_motion,
        SmoothFrameMotion(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(
        force_frame_smoother_pos,
        ForceFrameSmootherPos(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE(get_smoothed_frame_pos, OnGetSmoothedFramePos)
    HANDLE_ACTION(set_difficulty, mDifficulty = (Difficulty)_msg->Int(2))
    HANDLE_EXPR(get_difficulty, mDifficulty)
    HANDLE_ACTION(set_left_team_crew, mLeftTeamCrew = _msg->Sym(2))
    HANDLE_ACTION(set_right_team_crew, mRightTeamCrew = _msg->Sym(2))
    HANDLE_EXPR(get_left_team_crew, mLeftTeamCrew)
    HANDLE_EXPR(get_right_team_crew, mRightTeamCrew)
    HANDLE_EXPR(get_points_for_win, GetPointsForWin())
    HANDLE_EXPR(get_points_for_loss, GetPointsForLoss())
    HANDLE_ACTION(update_scores, UpdateScores())
    HANDLE_ACTION(use_selected_playlist, UseSelectedPlaylist(_msg->Int(2)))
    HANDLE_EXPR(is_using_playlist, IsUsingPlaylist())
    HANDLE_ACTION(shuffle_playlist, ShufflePlaylist(_msg->Int(2)))
    HANDLE_EXPR(is_playlist_shuffled, mIsPlaylistShuffled)
    HANDLE_ACTION(use_full_length_songs, mUseFullLengthSongs = _msg->Int(2))
    HANDLE_EXPR(is_using_full_length_songs, mUseFullLengthSongs)
    HANDLE_ACTION(toggle_included_mode, ToggleIncludedMode(_msg->Sym(2)))
    HANDLE_ACTION(
        toggle_included_mode_on, ToggleIncludedModeOn(_msg->Sym(2), _msg->Int(3))
    )
    HANDLE_ACTION(set_modes, SetModes())
    HANDLE_EXPR(is_mode_included, IsModeIncluded(_msg->Sym(2)))
    HANDLE_ACTION(setup_infinite_party_mode, SetupInfinitePartyMode())
    HANDLE(set_song_and_defaults, OnSetSongAndDefaults)
    HANDLE_EXPR(get_playlist_string, GetPlaylistString())
    HANDLE_ACTION(set_per_song_difficulty, mPerSongDifficulty = _msg->Int(2))
    HANDLE_EXPR(use_per_song_difficulty, mPerSongDifficulty)
    HANDLE_ACTION(set_custom_party, mCustomParty = _msg->Int(2))
    HANDLE_EXPR(is_custom_party, mCustomParty)
    HANDLE_EXPR(get_left_crew_color_1, GetLeftCrewColor1AsArray())
    HANDLE_EXPR(get_left_crew_color_2, GetLeftCrewColor2AsArray())
    HANDLE_EXPR(get_right_crew_color_1, GetRightCrewColor1AsArray())
    HANDLE_EXPR(get_right_crew_color_2, GetRightCrewColor2AsArray())
    HANDLE_EXPR(get_crew_color, GetCrewColor(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(
        get_left_crew_char_outfit, GetLeftCrewCharOutfit(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_EXPR(
        get_right_crew_char_outfit, GetRightCrewCharOutfit(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_EXPR(get_left_team_prev_pct_of_max_points, unk2e4)
    HANDLE_EXPR(get_right_team_prev_pct_of_max_points, unk2e8)
    HANDLE_EXPR(get_left_team_curr_pct_of_max_points, unk2e4 = mLeftTeamScore / unk2ec)
    HANDLE_EXPR(get_right_team_curr_pct_of_max_points, unk2e8 = mRightTeamScore / unk2ec)
    HANDLE_EXPR(get_winning_side, mWinningSide)
    HANDLE_EXPR(get_just_won_side, mJustWonSide)
    HANDLE_EXPR(left_team_max_wins, LeftTeamMaxWins())
    HANDLE_EXPR(right_team_max_wins, RightTeamMaxWins())
    HANDLE_ACTION(send_party_options_to_rc, SendPartyOptionsToRC())
    HANDLE_ACTION(get_party_options_from_rc, GetPartyOptionsFromRC())
    HANDLE_ACTION(get_party_song_queue_from_rc, GetPartySongQueueFromRC())
    HANDLE_EXPR(get_next_song, GetNextSongName())
    HANDLE_ACTION(change_to_another_game_mode, ChangeToAnotherGameMode())
    HANDLE_EXPR(get_rounds_played, mRoundsPlayed)
    HANDLE_EXPR(get_rounds_total, mRoundsTotal)
    HANDLE_ACTION(start_party_stats, GetDateAndTime(unk31b))
    HANDLE_ACTION(end_party_stats, EndPartyStats())
    HANDLE_ACTION(smart_glass_listen, OnSmartGlassListen(_msg->Int(2)))
    HANDLE_ACTION(prune_history, PruneHistory())
    HANDLE_EXPR(stable_song, OnStableSong())
    HANDLE_EXPR(stable_mode, OnStableMode())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(SmartGlassMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PartyModeMgr)
    SYNC_PROP(is_playlist_shuffled, mIsPlaylistShuffled)
    SYNC_PROP(is_using_per_song_options, mUsingPerSongOptions)
    SYNC_PROP(curr_synced_song_id, mCurrSyncedSongID)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void PartyModeMgr::ContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0x154);
    if (!TheContentMgr.RefreshInProgress() && mCurrEvent) {
        if (TheHamSongMgr.IsContentUsedForSong(contentName, mCurrEvent->mSongID)) {
            static Symbol song_data_mounted("song_data_mounted");
            static Message msg(song_data_mounted, gNullStr);
            msg[0] = GetCurrEventSongShortName();
            TheUI->Export(msg, false);
        }
    }
}

void PartyModeMgr::Init() {
    MILO_ASSERT(ThePartyModeMgr == NULL, 0x142);
    ThePartyModeMgr = new PartyModeMgr();
    if (ObjectDir::Main()) {
        ThePartyModeMgr->SetName("partymode_mgr", ObjectDir::Main());
    }
    TheContentMgr.RegisterCallback(ThePartyModeMgr, false);
}

int PartyModeMgr::GetLeftPlayerIndex() const {
    int idx = -1;
    if (mLeftPlayer) {
        idx = mLeftPlayer->Index();
    }
    return idx;
}

int PartyModeMgr::GetRightPlayerIndex() const {
    int idx = -1;
    if (mRightPlayer) {
        idx = mRightPlayer->Index();
    }
    return idx;
}

void PartyModeMgr::IncLeftPlayerScore(int score) {
    if (mLeftPlayer) {
        mLeftPlayer->IncScore(score);
    }
}

void PartyModeMgr::IncRightPlayerScore(int score) {
    if (mRightPlayer) {
        mRightPlayer->IncScore(score);
    }
}

void PartyModeMgr::PushLeftPlayerTitle(Symbol title) {
    if (mLeftPlayer) {
        mLeftPlayer->PushTitle(title);
    }
}

void PartyModeMgr::PushRightPlayerTitle(Symbol title) {
    if (mRightPlayer) {
        mRightPlayer->PushTitle(title);
    }
}

void PartyModeMgr::SetLeftTeamScore(float score) {
    mLeftTeamPrevScore = mLeftTeamScore;
    mLeftTeamScore = score;
}

void PartyModeMgr::SetRightTeamScore(float score) {
    mRightTeamPrevScore = mRightTeamScore;
    mRightTeamScore = score;
}

void PartyModeMgr::IncLeftTeamScore(float score) {
    mLeftTeamPrevScore = mLeftTeamScore;
    mLeftTeamScore += score;
}

void PartyModeMgr::IncRightTeamScore(float score) {
    mRightTeamPrevScore = mRightTeamScore;
    mRightTeamScore += score;
}

void PartyModeMgr::StartNewRound() {
    mLeftTeamPrevScore = mLeftTeamScore;
    mRightTeamPrevScore = mRightTeamScore;
    unk2e4 = 0;
    unk2e8 = 0;
    mLeftTeamScore = 0;
    mRightTeamScore = 0;
}

bool PartyModeMgr::LeftTeamMaxWins() const {
    return 0.001f >= unk2ec - mLeftTeamScore && mWinningSide == 0;
}

bool PartyModeMgr::RightTeamMaxWins() const {
    return 0.001f >= unk2ec - mRightTeamScore && mWinningSide == 1;
}

bool PartyModeMgr::IsModeIncluded(Symbol mode) {
    return (1 << GetEnumFromModeName(mode)) & unk2dc;
}

Symbol PartyModeMgr::GetNextSongName() {
    if (mCurrSyncedSongID == 0) {
        return gNullStr;
    } else {
        return TheHamSongMgr.GetShortNameFromSongID(mCurrSyncedSongID, false);
    }
}

HamProfile *PartyModeMgr::GetValidProfile() {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        profile->UpdateOnlineID();
        if (profile->IsSignedIn() && ThePlatformMgr.IsSignedIntoLive(profile->GetPadNum())
            && TheRockCentral.IsOnline()) {
            return profile;
        }
    }
    return nullptr;
}

void PartyModeMgr::SetLeftTeamStarBonus() {
    mLeftTeamStarBonus = 0;
    if (mIsShowdown) {
        HamPlayerData *playerData0 = TheGameData->Player(0);
        Hmx::Object *provider0 = playerData0->Provider();
        HamPlayerData *playerData1 = TheGameData->Player(1);
        Hmx::Object *provider1 = playerData1->Provider();
        float score0 = provider0->Property("score")->Float();
        float score1 = provider1->Property("score")->Float();
        if (score1 > score0) {
            int numStars = TheHamProvider->Property("stars_earned", false)->Int();
            if (numStars == 5) {
                mLeftTeamStarBonus = mEventScoring->FindFloat("five_star_bonus");
            } else if (numStars == 6) {
                mLeftTeamStarBonus = mEventScoring->FindFloat("six_star_bonus");
            }
        }
    }
}

void PartyModeMgr::SetRightTeamStarBonus() {
    mRightTeamStarBonus = 0;
    if (mIsShowdown) {
        HamPlayerData *playerData0 = TheGameData->Player(0);
        Hmx::Object *provider0 = playerData0->Provider();
        HamPlayerData *playerData1 = TheGameData->Player(1);
        Hmx::Object *provider1 = playerData1->Provider();
        float score0 = provider0->Property("score")->Float();
        float score1 = provider1->Property("score")->Float();
        if (score0 > score1) {
            int numStars = TheHamProvider->Property("stars_earned", false)->Int();
            if (numStars == 5) {
                mRightTeamStarBonus = mEventScoring->FindFloat("five_star_bonus");
            } else if (numStars == 6) {
                mRightTeamStarBonus = mEventScoring->FindFloat("six_star_bonus");
            }
        }
    }
}

float PartyModeMgr::GetPointsForWin() {
    static Symbol win("win");
    DataArray *winPoints = mEventScoring->FindArray(win);
    MILO_ASSERT(winPoints, 0x427);
    DataArray *winData = winPoints->FindArray(mCurrEvent->mModeName, false);
    if (winData) {
        return winData->Float(1);
    } else {
        MILO_NOTIFY(
            "Party mode event %s does not have win scoring data",
            mCurrEvent->mModeName.Str()
        );
        return 0;
    }
}

float PartyModeMgr::GetPointsForLoss() {
    static Symbol lose("lose");
    DataArray *losePoints = mEventScoring->FindArray(lose);
    MILO_ASSERT(losePoints, 0x43D);
    DataArray *loseData = losePoints->FindArray(mCurrEvent->mModeName, false);
    if (loseData) {
        return loseData->Float(1);
    } else {
        MILO_NOTIFY(
            "Party mode event %s does not have lose scoring data",
            mCurrEvent->mModeName.Str()
        );
        return 0;
    }
}

void PartyModeMgr::UpdateRoundsPlayed() {
    mRoundsPlayed++;
    if (mRoundsUntilShowdown == 0) {
        mRoundsUntilShowdown = mRoundsTotal;
    } else {
        mRoundsUntilShowdown--;
    }
    MILO_LOG(
        "----- updating rounds played - rounds played: %d; rounds until showdown: %d\n",
        mRoundsPlayed,
        mRoundsUntilShowdown
    );
}

Symbol PartyModeMgr::GetCurrEventName() {
    MILO_ASSERT(mCurrEvent, 0x4BC);
    Symbol ret(gNullStr);
    ret = mCurrEvent->mModeName;
    return ret;
}

Symbol PartyModeMgr::GetCurrEventMicrogameName() {
    MILO_ASSERT(mCurrEvent, 0x4CB);
    Symbol ret(gNullStr);
    ret = mCurrEvent->mSubModeName;
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongName() {
    MILO_ASSERT(mCurrEvent, 0x4D4);
    Symbol ret(gNullStr);
    const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
    MILO_ASSERT(data, 0x4DB);
    ret = Symbol(data->Title());
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongShortName() {
    MILO_ASSERT(mCurrEvent, 0x4E3);
    Symbol ret(gNullStr);
    const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
    MILO_ASSERT(data, 0x4EA);
    SongRecord record(data);
    ret = record.ShortName();
    return ret;
}

Symbol PartyModeMgr::GetCurrEventSongArtistName() {
    MILO_ASSERT(mCurrEvent, 0x4F3);
    Symbol ret(gNullStr);
    static Symbol partymode_intermission("partymode_intermission");
    if (mCurrEvent->mModeName == partymode_intermission) {
        return ret;
    } else {
        const HamSongMetadata *data = TheHamSongMgr.Data(mCurrEvent->mSongID);
        MILO_ASSERT(data, 0x500);
        ret = Symbol(data->Artist());
    }
    return ret;
}

int PartyModeMgr::GetCurrEventPlayerFlags() {
    MILO_ASSERT(mCurrEvent, 0x508);
    return mCurrEvent->mPlayerFlags;
}

int PartyModeMgr::GetCurrEventNumPlayers() {
    MILO_ASSERT(mCurrEvent, 0x511);
    return mCurrEvent->mNumPlayers;
}

DataArray *PartyModeMgr::GetCurrEventPlayers() {
    MILO_ASSERT(mCurrEvent, 0x51A);
    return mCurrEvent->mPlayers;
}

void PartyModeMgr::SetupCharacterData() {
    MILO_ASSERT(TheHamProvider->Property("is_in_party_mode")->Int(), 0x5DB);
    for (int i = 0; i < 2; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        Symbol crew;
        if (hpd->Side() == kSkeletonRight) {
            crew = mRightTeamCrew;
        } else {
            crew = mLeftTeamCrew;
        }
        hpd->SetCrew(crew);
        Symbol crewChar = GetCrewCharacter(crew, rand() % GetNumCrewCharacters(crew));
        hpd->SetCharacter(crewChar);
        Symbol outfit = GetCrewLookOutfit(crewChar);
        hpd->SetCharacterOutfit(outfit);
    }
    const HamSongMetadata *pData = TheHamSongMgr.Data(
        TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong(), false)
    );
    MILO_ASSERT(pData, 0x5F8);
    TheGameData->SetVenue(pData->Venue());
}

void PartyModeMgr::SmoothFrameMotion(int frame_idx, float f2, float f3) {
    MILO_ASSERT_RANGE(frame_idx, 0, 6, 0x64E);
    mFrameSmoothers[frame_idx].Smooth(Vector2(f2, f3), TheTaskMgr.DeltaUISeconds(), false);
}

void PartyModeMgr::ForceFrameSmootherPos(int frame_idx, float f2, float f3) {
    MILO_ASSERT_RANGE(frame_idx, 0, 6, 0x656);
    mFrameSmoothers[frame_idx].ForceValue(Vector2(f2, f3));
}

const char *PartyModeMgr::GetPlaylistString() {
    if (!mPlaylist) {
        return gNullStr;
    } else {
        String str;
        if (mPlaylist->IsCustom()) {
            str = mPlaylist->GetName();
        } else {
            str = MakeString("%s_title", mPlaylist->GetName());
        }
        const char *fmt = FormatTimeMS(mPlaylist->GetDuration());
        static Symbol songname_duration("songname_duration");
        str = MakeString(
            Localize(songname_duration, nullptr, TheLocale),
            Localize(str.c_str(), nullptr, TheLocale),
            fmt
        );
        return str.c_str();
    }
}

DataArray *PartyModeMgr::GetLeftCrewColor1AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x803);
    static Symbol left("left");
    DataArray *pTeamData = pTeamArray->FindArray(left);
    MILO_ASSERT(pTeamData, 0x807);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x80B);
    DataArray *pTeamColor = pTeamColors->Array(1);
    MILO_ASSERT(pTeamColor, 0x80E);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetLeftCrewColor2AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x817);
    static Symbol left("left");
    DataArray *pTeamData = pTeamArray->FindArray(left);
    MILO_ASSERT(pTeamData, 0x81B);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x81F);
    DataArray *pTeamColor = pTeamColors->Array(2);
    MILO_ASSERT(pTeamColor, 0x822);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetRightCrewColor1AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x82B);
    static Symbol right("right");
    DataArray *pTeamData = pTeamArray->FindArray(right);
    MILO_ASSERT(pTeamData, 0x82F);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x833);
    DataArray *pTeamColor = pTeamColors->Array(1);
    MILO_ASSERT(pTeamColor, 0x836);
    return pTeamColor;
}

DataArray *PartyModeMgr::GetRightCrewColor2AsArray() {
    static Symbol TEAM_COLORS("TEAM_COLORS");
    DataArray *pTeamArray = DataGetMacro(TEAM_COLORS);
    MILO_ASSERT(pTeamArray, 0x83F);
    static Symbol right("right");
    DataArray *pTeamData = pTeamArray->FindArray(right);
    MILO_ASSERT(pTeamData, 0x843);
    static Symbol colors("colors");
    DataArray *pTeamColors = pTeamData->FindArray(colors);
    MILO_ASSERT(pTeamColors, 0x847);
    DataArray *pTeamColor = pTeamColors->Array(2);
    MILO_ASSERT(pTeamColor, 0x84A);
    return pTeamColor;
}

Symbol PartyModeMgr::GetLeftCrewCharOutfit(int char_idx, int outfit_idx) {
    int numCrewChars = GetNumCrewCharacters(mLeftTeamCrew);
    MILO_ASSERT(char_idx < numCrewChars, 0x877);
    if (char_idx < 0) {
        char_idx = rand() % numCrewChars;
    }
    Symbol charSym = GetCrewCharacter(mLeftTeamCrew, char_idx);
    int numCharOutfits = GetNumCharacterOutfits(charSym);
    MILO_ASSERT(outfit_idx < numCharOutfits, 0x880);
    if (outfit_idx < 0) {
        outfit_idx = rand() % numCharOutfits;
    }
    return GetCharacterOutfit(charSym, outfit_idx);
}

Symbol PartyModeMgr::GetRightCrewCharOutfit(int char_idx, int outfit_idx) {
    int numCrewChars = GetNumCrewCharacters(mRightTeamCrew);
    MILO_ASSERT(char_idx < numCrewChars, 0x88E);
    if (char_idx < 0) {
        char_idx = rand() % numCrewChars;
    }
    Symbol charSym = GetCrewCharacter(mRightTeamCrew, char_idx);
    int numCharOutfits = GetNumCharacterOutfits(charSym);
    MILO_ASSERT(outfit_idx < numCharOutfits, 0x897);
    if (outfit_idx < 0) {
        outfit_idx = rand() % numCharOutfits;
    }
    return GetCharacterOutfit(charSym, outfit_idx);
}

void PartyModeMgr::ChangeToAnotherGameMode() {
    int i1 = (GetEnumFromModeName(mCurrEvent->mModeName) + 1) % 5;
    Symbol name = GetModeNameFromEnum(i1);
    while (!IsModeIncluded(name)) {
        i1 = (i1 + 1) % 5;
        name = GetModeNameFromEnum(i1);
    }
    mCurrEvent->mModeName = name;
}

void PartyModeMgr::EndPartyStats() {
    DateTime dt;
    GetDateAndTime(dt);
    unsigned int diff = dt.DiffSeconds(unk31b);
    for (int i = 0; i < 2; i++) {
        HamPlayerData *playerData = TheGameData->Player(i);
        MILO_ASSERT(playerData, 0x9DA);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(playerData->PadNum());
        if (profile && profile->GetMetagameStats()) {
            profile->GetMetagameStats()->UpdatePartyStats(diff);
        }
    }
}

void PartyModeMgr::StorePlayerFramePos(int player, float f2, float f3) {
    MILO_ASSERT(player >= 0 && player < mPlayers.size(), 0x369);
    mPlayers[player]->StoreFramePos(f2, f3);
}

void PartyModeMgr::StorePlayerFrameScale(int player, float scale) {
    MILO_ASSERT(player >= 0 && player < mPlayers.size(), 0x370);
    mPlayers[player]->StoreFrameScale(scale);
}

const char *PartyModeMgr::GetPlayerARTexPath(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, mPlayers.size(), 0x377);
    return mPlayers[playerIndex]->GetTexPath();
}

void PartyModeMgr::SetRandomCharacters() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x5CA);
        Symbol symRandomCharacter = mCharacters[rand() % mCharacters.size()];
        MILO_ASSERT(symRandomCharacter != gNullStr, 0x5CE);
        Symbol crew = GetCrewForCharacter(symRandomCharacter);
        pPlayerData->SetCharacter(symRandomCharacter);
        pPlayerData->SetCrew(crew);
        pPlayerData->SetOutfit(GetCharacterOutfit(symRandomCharacter, 0));
    }
}

int PartyModeMgr::GetPlayerPhotoIndex(int player) {
    MILO_ASSERT_RANGE(player, 0, mPlayers.size(), 0x5FF);
    return mPlayers[player]->GetPhotoIndex();
}

void PartyModeMgr::BroadcastSyncMsg(Symbol msgType) {
    MILO_LOG("[PartyModeMgr::BroadcastSyncMsg] Broadcasting msg (%s).\n", msgType.Str());
    Message msg(msgType);
    HandleType(msg);
    TheUI->Handle(msg, false);
}

void PartyModeMgr::SendPartyOptionsToRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        mSetPartyOptionsJob =
            new SetPartyOptionsJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(mSetPartyOptionsJob);
    }
}

void PartyModeMgr::GetPartyOptionsFromRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        mGetPartyOptionsJob =
            new GetPartyOptionsJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(mGetPartyOptionsJob);
    }
}

void PartyModeMgr::ReadPartyOptions() {
    mGetPartyOptionsJob->GetOptions();
    mGetPartyOptionsJob = nullptr;
    BroadcastSyncMsg("options_updated");
}

void PartyModeMgr::GetPartySongQueueFromRC() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        mGetPartySongQueueJob =
            new GetPartySongQueueJob(this, profile->GetOnlineID()->ToString());
        TheRockCentral.ManageJob(mGetPartySongQueueJob);
    }
}

void PartyModeMgr::DeleteSongFromRCPartySongQueue(int songID) {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        mDeleteSongFromPartySongQueueJob = new DeleteSongFromPartySongQueueJob(
            this, profile->GetOnlineID()->ToString(), songID
        );
        TheRockCentral.ManageJob(mDeleteSongFromPartySongQueueJob);
    }
}

void PartyModeMgr::AddNextSongToRCPartySongQueue() {
    HamProfile *profile = GetValidProfile();
    if (!profile) {
        BroadcastSyncMsg("skipped_sync");
    } else {
        mAddSongToPartySongQueueJob = new AddSongToPartySongQueueJob(
            this, profile->GetOnlineID()->ToString(), unk308.front().mSongID
        );
        TheRockCentral.ManageJob(mAddSongToPartySongQueueJob);
    }
}

Symbol PartyModeMgr::GetNextMode() {
    MILO_ASSERT(mModePicker.Size() > 0, 0x17B);
    return mModePicker.GetNext();
}

void PartyModeMgr::DetermineSubMode(Symbol *pMode, Symbol *pSubMode) {
    if (mUsePlaytestData) {
        *pMode = mModePicker.GetNext();
        *pSubMode = mSubModePicker.GetNext();
    } else if (TheHamProvider->Property("is_in_party_mode")->Int()
               && !mRoundsUntilShowdown) {
        static Symbol showdown("showdown");
        static Symbol ffa("ffa");
        *pMode = showdown;
        *pSubMode = ffa;
    } else {
        *pMode = mModePicker.GetNext();
        if (unk328) {
            Symbol sym = unk328->Sym(mRoundsPlayed + 1);
            static Symbol event_buckets("event_buckets");
            DataArray *arr = mPartyModeCfg->FindArray(event_buckets);
            arr = arr->FindArray(sym);
            int i12 = 0;
            for (int i = 1; i < arr->Size(); i++) {
                DataArray *curArr = arr->Array(i);
                if (IsModeIncluded(curArr->Sym(0))) {
                    i12 += curArr->Int(1);
                }
            }
            i12 = rand() % i12;
            int i = 1;
            int i4 = 0;
            for (; i < arr->Size(); i++) {
                DataArray *curArr = arr->Array(i);
                if (IsModeIncluded(curArr->Sym(0))) {
                    i4 += curArr->Int(1);
                    if (i12 < i4) {
                        *pMode = curArr->Sym(0);
                        break;
                    }
                }
            }
        }
        static Symbol dance_battle("dance_battle");
        if (*pMode == dance_battle) {
            static Symbol ffa("ffa");
            *pSubMode = ffa;
        }
    }
}

void PartyModeMgr::DetermineSubModeSong(Symbol *pShortName, int *pSongID) {
    if (unk324 && !mPlaylist) {
        DataArray *arr = unk324->Array(mRoundsPlayed + 1);
        if (arr) {
            int rank = arr->Int(rand() % arr->Size());
            MILO_ASSERT_FMT(
                rank >= 1 && rank <= 4, "%d is an invalid DJ logic intensity rank\n", rank
            );
            *pShortName = mSubModeSongPickers[rank].GetNext();
            *pSongID = TheHamSongMgr.GetSongIDFromShortName(*pShortName);
            return;
        } else {
            MILO_NOTIFY(
                "DJ logic data doesn't contain enough information for %d rounds, picking random song instead",
                mRoundsPlayed
            );
        }
    }
    *pShortName = unkf8.GetNext();
    *pSongID = TheHamSongMgr.GetSongIDFromShortName(*pShortName);
}

bool PartyModeMgr::IsTeamSignedIn(int i1) {
    if (i1 == 1) {
        return unkd0.Size() > 0;
    } else if (i1 == 2) {
        return unke4.Size() > 0;
    } else {
        return false;
    }
}

PartyModePlayer *PartyModeMgr::CreatePartyModePlayer() {
    int objIdx = mPlayers.size() % unkb0.size() + 1;
    DataArray *objArr = mARObjects->Array(objIdx);
    PartyModeARObject *arObj = new PartyModeARObject(objArr);
    PartyModePlayer *player = new PartyModePlayer(arObj);
    player->SetSym(mCharacters[rand() % mCharacters.size()]);
    player->SetIndex(mPlayers.size());
    if (unkd0.Size() <= 0) {
        player->SetPhotoIndex(mTeam1Players.size());
    } else {
        player->SetPhotoIndex(mTeam2Players.size() + 4);
    }
    return player;
}

void PartyModeMgr::AddPlayerToTeam(int team) {
    PartyModePlayer *player = CreatePartyModePlayer();
    mPlayers.push_back(player);
    if (team == 1) {
        mTeam1Players.push_back(player);
    } else if (team == 2) {
        mTeam2Players.push_back(player);
    }
}

void PartyModeMgr::ClearTeam(int team) {
    switch (team) {
    case 1:
        for (int i = 0; i != mTeam1Players.size(); i++) {
            delete mTeam1Players[i];
        }
        mTeam1Players.clear();
        break;
    case 2:
        for (int i = 0; i != mTeam2Players.size(); i++) {
            delete mTeam2Players[i];
        }
        mTeam2Players.clear();
        break;
    default:
        MILO_ASSERT(team == 1 || team == 2, 0x20F);
        break;
    }
}

void PartyModeMgr::ResetPlayers() {
    for (int i = 0; i < mPlayers.size(); i++) {
        RELEASE(mPlayers[i]);
    }
    mPlayers.clear();
    mTeam1Players.clear();
    mTeam2Players.clear();
    unkd0.Clear();
    unke4.Clear();
    mLeftPlayer = nullptr;
    mRightPlayer = nullptr;
}

void PartyModeMgr::ResetMicrogames() {
    mSubModePicker.Clear();
    DataArray *gamesArr = mPartyModeCfg->FindArray("party_mode_microgames");
    for (int i = 1; i < gamesArr->Size(); i++) {
        mSubModePicker.AddItem(gamesArr->Sym(i));
    }
    mSubModePicker.Randomize();
}

int PartyModeMgr::PickNextPlayer() {
    int ret = -1;
    if (unk1c8 == 2) {
        ret = unkd0.GetNext();
        if (mUsePlaytestData) {
            ret = ret % mTeam1Players.size();
        }
        unk1c8 = 1;
        if (unk32c) {
            DataArray *arr = unk32c->Array(mRoundsPlayed + 1);
            int idx = 0;
            if (mTeam1Players.size() > mTeam2Players.size())
                idx = 1;
            ret = arr->Int(idx);
        }
    } else if (unk1c8 == 1) {
        ret = unke4.GetNext();
        if (mUsePlaytestData) {
            ret = ret % mTeam2Players.size() + mTeam1Players.size();
        }
        unk1c8 = 2;
        if (unk32c) {
            DataArray *arr = unk32c->Array(mRoundsPlayed + 1);
            int idx = 1;
            if (mTeam2Players.size() < mTeam1Players.size())
                idx = 0;
            ret = mTeam1Players.size() + arr->Int(idx);
        }
    }
    return ret;
}

void PartyModeMgr::ShufflePlaylist(bool b1) {
    MILO_ASSERT(IsUsingPlaylist(), 0x731);
    if (b1) {
        // unkf8.unk8 = 2;
        // unkf8.unk10 = 0;
    } else if (mIsPlaylistShuffled) {
        // unkf8.unk8 = 0;
        SetSongsFromPlaylist();
    }
    mIsPlaylistShuffled = b1;
}

void PartyModeMgr::ResetParty() {
    mRoundsPlayed = 0;
    mIsShowdown = false;
    unk1c8 = 2;
    ResetPlayers();
    mDifficulty = DefaultDifficulty();
    if (unk1d4.empty()) {
        TheHamSongMgr.GetRandomlySelectableRankedSongs(unk1d4);
    }
    mLeftTeamPrevScore = mLeftTeamScore;
    mRightTeamPrevScore = mRightTeamScore;
    mLeftTeamScore = 0;
    mRightTeamScore = 0;
    unk2e4 = 0;
    unk2e8 = 0;
    Symbol crew(gNullStr);
    HamPlayerData *pPlayerData = TheGameData->Player(0);
    MILO_ASSERT(pPlayerData, 0x17F);
    pPlayerData->SetCrew(crew);
    pPlayerData = TheGameData->Player(1);
    MILO_ASSERT(pPlayerData, 0x182);
    pPlayerData->SetCrew(crew);
    mWinningSide = 2;
    mJustWonSide = 2;
    unk324 = nullptr;
}

void PartyModeMgr::InitCharacters() {
    mCharacters.clear();
    DataArray *crewsArr = SystemConfig()->FindArray("selectable_crews", false);
    if (crewsArr) {
        for (int i = 1; i < crewsArr->Size(); i++) {
            Symbol crew = crewsArr->Sym(i);
            int numChars = GetNumCrewCharacters(crew);
            for (int j = 0; j < numChars; j++) {
                Symbol charSym = GetCrewCharacter(crew, j);
                mCharacters.push_back(charSym);
            }
        }
    }
}

void PartyModeMgr::CrewShowdownRematch() {
    mLeftTeamPrevScore = mLeftTeamScore;
    mRightTeamPrevScore = mRightTeamScore;
    mRoundsPlayed = 0;
    mIsShowdown = false;
    unk1c8 = 2;
    mLeftTeamScore = 0;
    mRightTeamPrevScore = 0;
    unk2e4 = 0;
    unk2e8 = 0;
    SetCurrEvent();
    mWinningSide = 2;
    mJustWonSide = 2;
    static Symbol rematches_this_boot("rematches_this_boot");
    gRematchCount++;
    SendDataPoint("crew_throwdown/rematch", rematches_this_boot, gRematchCount);
}

void PartyModeMgr::SetupInfinitePartyMode() {
    TheHamSongMgr.GetRandomlySelectableRankedSongs(unk1d4);
    if (mPlaylist) {
        unkf8.SetMode(0);
    } else {
        ResetSongs();
    }
    ResetModes(true);
    ResetMicrogames();
    RELEASE(mCurrEvent);
    mCurrEvent = new SubMode();
    GetDateAndTime(unk315);
}

void PartyModeMgr::SetModes() {
    ResetModes(false);
    if (mCurrEvent && !IsModeIncluded(mCurrEvent->mModeName)) {
        Symbol mode, submode;
        DetermineSubMode(&mode, &submode);
        mCurrEvent->mModeName = mode;
        mCurrEvent->mSubModeName = submode;
    }
}

void PartyModeMgr::SetSongAndDefaults(Symbol song, Symbol mode, bool force_crew_outfit) {
    static Symbol dance_battle("dance_battle");
    static Symbol strike_a_pose("strike_a_pose");
    if (mCurrEvent) {
        RELEASE(mCurrEvent);
    }
    mCurrEvent = new SubMode();
    if (song.Null()) {
        song = unkf8.GetNext();
    }
    mCurrEvent->mSongName = song;
    if (mode.Null()) {
        mode = GetNextMode();
    }
    mCurrEvent->mModeName = mode;
    int songID = TheHamSongMgr.GetSongIDFromShortName(song);
    mCurrEvent->mSongID = songID;
    const HamSongMetadata *data = TheHamSongMgr.Data(songID);
    HamPlayerData *songPlayerData;
    Symbol songCrew;
    Symbol songChar;
    Symbol songOutfit;
    HamPlayerData *altPlayerData;
    Symbol altCrew;
    Symbol altChar;
    Symbol altOutfit;
    MetaPerformer::Current()->CalcCharacters(
        data,
        mode == dance_battle || mode == strike_a_pose,
        (PlayerFlag)2,
        songPlayerData,
        songCrew,
        songChar,
        songOutfit,
        altPlayerData,
        altCrew,
        altChar,
        altOutfit
    );
    if (force_crew_outfit) {
        songOutfit = GetCrewLookOutfit(songChar);
        altOutfit = GetCrewLookOutfit(altChar);
    }
    songPlayerData->SetCharacter(songChar);
    songPlayerData->SetOutfit(songOutfit);
    songPlayerData->SetCrew(songCrew);
    altPlayerData->SetCharacter(altChar);
    altPlayerData->SetOutfit(altOutfit);
    altPlayerData->SetCrew(altCrew);
    MILO_LOG(
        "PartyModeMgr::SetSongAndDefaults(Symbol song = '%s', Symbol mode = '%s', bool force_crew_outfit = %d)\n",
        song,
        mode,
        force_crew_outfit
    );
    MILO_LOG(
        "   %s: songChar = '%s' songCrew = '%s' songOutfit = %s\n",
        songPlayerData->Side() == kSkeletonLeft ? "left" : "right",
        songChar,
        songCrew,
        songOutfit
    );
    MILO_LOG(
        "   %s: altChar  = '%s' altCrew  = '%s' altOutfit  = %s\n",
        altPlayerData->Side() == kSkeletonLeft ? "left" : "right",
        altChar,
        altCrew,
        altOutfit
    );
    TheGameData->SetSong(song);
    MetaPerformer::Current()->SetVenuePref("default");
    MetaPerformer::Current()->Handle(Message("setup_venue"), true);
    ConfigHistory ch;
    ch.mForceCrewOutfit = force_crew_outfit;
    ch.mMode = mode;
    ch.mSong = song;
    ch.mTimeStamp = SystemMs();
    mCfgHistories.push_back(ch);
    PruneHistory();
}

PartyModeMgr::SubMode *PartyModeMgr::CreateEventA() {
    Symbol mode;
    Symbol submode;
    DetermineSubMode(&mode, &submode);
    int flags = 0;
    int numPlayers = 0;
    std::vector<int> vec;
    DetermineSubModePlayers(mode, &flags, &numPlayers, &vec);
    int songID = 0;
    Symbol shortname;
    DetermineSubModeSong(&shortname, &songID);
    SubMode *event = new SubMode();
    event->mModeName = mode;
    event->mSubModeName = submode;
    event->mSongName = shortname;
    event->mSongID = songID;
    event->mPlayerFlags = flags;
    event->mNumPlayers = numPlayers;
    event->unk1c.insert(event->unk1c.begin(), vec.begin(), vec.end());
    DataArray *a = new DataArray(numPlayers);
    for (int i = 0; i < numPlayers; i++) {
        a->Node(i) = event->unk1c[i];
    }
    event->mPlayers = a;
    return event;
}

void PartyModeMgr::ToggleIncludedMode(Symbol mode) {
    unk2dc ^= 1 << GetEnumFromModeName(mode);
    bool high = (1 << GetEnumFromModeName(mode)) & unk2dc;
    MILO_LOG("----- TOGGLING %s to %s\n", mode.Str(), high ? "true" : "false");
    static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
    if (!unk40) {
        if (TheHamProvider->Property(is_in_infinite_party_mode)->Int() != 0) {
            SendDataPoint("partymode/mode_toggle", mode, high);
        } else {
            SendDataPoint("crew_throwdown/mode_toggle", mode, high);
        }
    }
}

void PartyModeMgr::UseSelectedPlaylist(bool b1) {
    if (b1) {
        MetaPerformer *pPerformer = MetaPerformer::Current();
        MILO_ASSERT(pPerformer, 0x6F1);
        mPlaylist = pPerformer->GetPlaylist();
        MILO_ASSERT(mPlaylist, 0x6F4);
        SetSongsFromPlaylist();
    } else {
        if (mPlaylist) {
            ResetSongs();
        }
        mPlaylist = nullptr;
    }
}

void PartyModeMgr::SetPlaylist(Playlist *playlist) {
    MILO_ASSERT(playlist, 0x706);
    mPlaylist = playlist;
    SetSongsFromPlaylist();
}

void PartyModeMgr::SetCurrEvent() {
    if (mCurrEvent) {
        RELEASE(mCurrEvent);
    }
    mCurrEvent = CreateEventA();
    static Symbol showdown("showdown");
    mIsShowdown = mCurrEvent->mModeName == showdown;
    mLeftPlayer = mCurrEvent->mNumPlayers > 0 ? mPlayers[mCurrEvent->unk1c[0]] : nullptr;
    mRightPlayer = mCurrEvent->mNumPlayers > 1 ? mPlayers[mCurrEvent->unk1c[1]] : nullptr;
}

DataNode PartyModeMgr::OnGetSmoothedFramePos(const DataArray *a) {
    MILO_ASSERT(a->Size() == 5, 0x65E);
    int idx = a->Int(2);
    Vector2 v = mFrameSmoothers[idx].Value();
    *a->Var(3) = v.x;
    *a->Var(4) = v.y;
    return 0;
}

DataNode PartyModeMgr::OnStableSong() {
    return mCfgHistories.empty() ? Symbol("") : mCfgHistories[0].mSong;
}

DataNode PartyModeMgr::OnStableMode() {
    return mCfgHistories.empty() ? Symbol("") : mCfgHistories[0].mMode;
}

DataNode PartyModeMgr::OnMsg(const SmartGlassMsg &msg) {
    MILO_LOG("SmartGlass: I should update Party Mode options/song from RC\n");
    SendDataPoint("smartglass/party");
    GetPartyOptionsFromRC();
    GetPartySongQueueFromRC();
    BroadcastSyncMsg("update_party_from_rc");
    return 1;
}

void PartyModeMgr::FinalizePlaytestParty() {
    int numEvents = mPartyModePlaytestEvents->Size() - 1;
    std::vector<Symbol> modeVec(numEvents, gNullStr);
    std::vector<Symbol> subModeVec(numEvents, gNullStr);
    std::vector<Symbol> songVec(numEvents, gNullStr);
    std::vector<int> team1Players;
    std::vector<int> team2Players;

    for (int i = 1; i <= numEvents; i++) {
        DataArray *eventArr = mPartyModePlaytestEvents->Array(i);
        Symbol mode = eventArr->Sym(0);
        Symbol subMode = eventArr->Sym(1);
        Symbol song = eventArr->Sym(2);
        if (eventArr->Size() > 4) {
            int team = eventArr->Int(3);
            int playerIdx = eventArr->Int(4) + mPlayers.size();
            team1Players.push_back(team);
            team2Players.push_back(playerIdx);
        }
        modeVec[i - 1] = mode;
        subModeVec[i - 1] = subMode;
        songVec[i - 1] = song;
    }

    mModePicker.Clear();
    unkf8.Clear();
    mModePicker.AddItems(modeVec);
    mSubModePicker.Clear();
    mSubModePicker.AddItems(subModeVec);
    unkf8.Clear();
    unkf8.AddItems(songVec);
    unkd0.Clear();
    unkd0.AddItems(team1Players);
    unke4.Clear();
    unke4.AddItems(team2Players);

    mRoundsTotal = numEvents - 1;
    mRoundsUntilShowdown = numEvents - 1;
    unk2ec = (float)(numEvents - 1) + 1.0f;
    static Symbol six_star_bonus("six_star_bonus");
    mSixStarBonus = mEventScoring->FindArray(six_star_bonus)->Float(1);
    SetCurrEvent();
}
