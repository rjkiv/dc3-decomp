#include "meta_ham/HamProfile.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "game/HamUser.h"
#include "game/HamUserMgr.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta/Profile.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/CampaignProgress.h"
#include "meta_ham/FitnessGoalMgr.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/Leaderboards.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/MetagameStats.h"
#include "meta_ham/MoveRatingHistory.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SaveLoadManager.h"
#include "meta_ham/SongStatusMgr.h"
#include "meta_ham/Utl.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "utl/JobMgr.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

HamProfile::HamProfile(int i1)
    : Profile(i1), mAccomplishmentProgress(this), mNeedToRefreshPlaylists(0),
      mInFitnessMode(0), mFitnessLbs(130), mIsFitnessWeightEntered(0),
      mFitnessSessionSecs(0), mFitnessSessionCals(0), mFitnessSongCals(0),
      mUploadFriendsToken(0), mOnlineID(new OnlineID()), mHasOnlineID(0),
      mExportChecked(kUnknown), mChallengeTimeStamp(0), mSkippedSongCount(0),
      mUsedUnlockableOutfitCount(0), mLastBestScore(0), mLastModePractice(0),
      mLastPracticeSong(gNullStr), mHasValidFitnessGoal(0), mFitnessGoalStartDay(0),
      mFitnessGoalStartMonth(0), mFitnessGoalStartYear(0), mFitnessGoalTargetDays(0),
      mFitnessGoalTargetCalories(0), mFitnessGoalDaysActive(0),
      mFitnessGoalCaloriesBurned(0), mFitnessGoalLastDayNumber(0),
      mFitnessGoalNeedUpload(0), mLastNewSong(0), mBootsSinceLastNag(0), mFirstTimeNag(1),
      mNagType(0), mAllowedNagTypes(3) {
    mSaveSizeMethod = SaveSize;
    mScores = new SongStatusMgr(&TheHamSongMgr);
    mStats = new MetagameStats();
    mRank = new MetagameRank(this);
    mMoveRatingHistory = new MoveRatingHistory();
    mCampaignProgress[0].SetParent(this);
    mCampaignProgress[1].SetParent(this);
    mCampaignProgress[2].SetParent(this);
    ResetOutfitPrefs();
    static Symbol playlist_custom("playlist_custom");
    for (int i = 0; i < 5; i++) {
        Symbol cur = MakeString("playlist_custom_%02i", i + 1);
        mPlaylists[i].SetParentProfile(this);
        mPlaylists[i].SetName(cur);
    }
}

HamProfile::~HamProfile() {
    DeleteAll();
    delete mScores;
    delete mStats;
    delete mRank;
    delete mMoveRatingHistory;
}

void HamProfile::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << *mScores;
    FixedSizeSaveable::SaveStd(fs, mUnlockedContent, 100);
    FixedSizeSaveable::SaveStd(fs, mNewContent, 100);
    fs << mAccomplishmentProgress;
    fs << mCampaignProgress[0];
    fs << mCampaignProgress[1];
    fs << mCampaignProgress[2];
    fs << *mStats;
    fs << *mRank;
    fs << *mMoveRatingHistory;
    int numPrefs = mCharPrefs.size();
    fs << numPrefs;
    for (int i = 0; i < numPrefs; i++) {
        const CharacterPref &curPref = mCharPrefs[i];
        FixedSizeSaveable::SaveSymbolID(fs, curPref.mChar);
        FixedSizeSaveable::SaveSymbolID(fs, curPref.mOutfit);
    }
    for (int i = 0; i < 5; i++) {
        fs << mPlaylists[i];
    }
    fs << mUploadFriendsToken;
    fs << mInFitnessMode;
    fs << mFitnessLbs;
    fs << mIsFitnessWeightEntered;
    fs << mChallengeTimeStamp;
    fs << mFitnessGoalStartDay;
    fs << mFitnessGoalStartMonth;
    fs << mFitnessGoalStartYear;
    fs << mFitnessGoalTargetDays;
    fs << mFitnessGoalTargetCalories;
    fs << mFitnessGoalDaysActive;
    fs << mFitnessGoalCaloriesBurned;
    fs << mFitnessGoalLastDayNumber;
    fs << mUsedUnlockableOutfitCount;
    fs << mLastNewSong;
    fs << mBootsSinceLastNag;
    fs << mFirstTimeNag;
    fs << mNagType;
    fs << mAllowedNagTypes;
    fs << mHasValidFitnessGoal;
    fs << mFitnessGoalNeedUpload;
    const_cast<HamProfile *>(this)->mDirty = false;
}

void HamProfile::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    DeleteAll();
    fs >> *mScores;
    FixedSizeSaveable::LoadStd(fs, mUnlockedContent, 100);
    FixedSizeSaveable::LoadStd(fs, mNewContent, 100);
    fs >> mAccomplishmentProgress;
    fs >> mCampaignProgress[0];
    fs >> mCampaignProgress[1];
    fs >> mCampaignProgress[2];
    fs >> *mStats;
    fs >> *mRank;
    fs >> *mMoveRatingHistory;
    int count;
    fs >> count;
    for (int i = 0; i < count; i++) {
        Symbol charSym, outfit;
        FixedSizeSaveable::LoadSymbolFromID(fs, charSym);
        FixedSizeSaveable::LoadSymbolFromID(fs, outfit);
        auto itEnd = mCharPrefs.end();
        auto it = std::find(mCharPrefs.begin(), itEnd, charSym);
        if (it == itEnd || GetOutfitCharacter(outfit) != charSym) {
            MILO_NOTIFY("Obsolete outfit pref %s for %s", outfit, charSym);
        } else {
            it->mOutfit = outfit;
        }
    }
    for (int i = 0; i < 5; i++) {
        fs >> mPlaylists[i];
    }
    fs >> mUploadFriendsToken;
    fs >> mInFitnessMode;
    fs >> mFitnessLbs;
    fs >> mIsFitnessWeightEntered;
    fs >> mChallengeTimeStamp;
    fs >> mFitnessGoalStartDay;
    fs >> mFitnessGoalStartMonth;
    fs >> mFitnessGoalStartYear;
    fs >> mFitnessGoalTargetDays;
    fs >> mFitnessGoalTargetCalories;
    fs >> mFitnessGoalDaysActive;
    fs >> mFitnessGoalCaloriesBurned;
    fs >> mFitnessGoalLastDayNumber;
    mSkippedSongCount = 0;
    fs >> mUsedUnlockableOutfitCount;
    fs >> mLastNewSong;
    fs >> mBootsSinceLastNag;
    fs >> mFirstTimeNag;
    fs >> mNagType;
    fs >> mAllowedNagTypes;
    fs >> mHasValidFitnessGoal;
    fs >> mFitnessGoalNeedUpload;
    mDirty = false;
    mNeedToRefreshPlaylists = false;
}

BEGIN_HANDLERS(HamProfile)
    HANDLE_EXPR(get_accomplishment_progress, &mAccomplishmentProgress)
    HANDLE_EXPR(get_stats, mStats)
    HANDLE_EXPR(get_rank, mRank)
    HANDLE_ACTION(mark_content_not_new, MarkContentNotNew(_msg->ForceSym(2)))
    HANDLE_EXPR(character_outfit, CharacterOutfit(_msg->Sym(2)))
    HANDLE_ACTION(set_character_outfit, SetCharacterOutfit(_msg->Sym(2), _msg->Sym(3)))
    HANDLE_EXPR(next_outfit_vo_sample, NextOutfitSample(_msg->Sym(2)))
    HANDLE_EXPR(has_song_status, HasSongStatus(_msg->Sym(2)))
    HANDLE_ACTION(set_pad_num, mPadNum = _msg->Int(2))
    HANDLE_EXPR(get_pad_num, GetPadNum())
    HANDLE_EXPR(in_fitness_mode, mInFitnessMode)
    HANDLE_EXPR(is_fitness_weight_entered, mIsFitnessWeightEntered)
    HANDLE_ACTION(set_fitness_pounds, SetFitnessPounds(_msg->Float(2)))
    HANDLE_EXPR(get_fitness_pounds, mFitnessLbs)
    HANDLE_ACTION(
        set_fitness_stats, SetFitnessStats(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(toggle_fitness_mode, SetFitnessMode(!mInFitnessMode))
    HANDLE_EXPR(get_fitness_time, (int)mFitnessSessionSecs)
    HANDLE_ACTION(update_fitness_time, UpdateFitnessTime(_msg->Obj<HamLabel>(2)))
    HANDLE_ACTION(
        update_fitness_total_time, UpdateFitnessTotalTime(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(update_fitness_weight, UpdateFitnessWeight(_msg->Obj<HamLabel>(2)))
    HANDLE_EXPR(get_fitness_calories, (int)mFitnessSessionCals)
    HANDLE_ACTION(
        update_infinite_playlist_time, UpdateInfinitePlaylistTime(_msg->Obj<HamLabel>(2))
    )
    HANDLE_EXPR(get_skipped_song_count, mSkippedSongCount)
    HANDLE_ACTION(increment_skipped_song_count, IncrementSkippedSongCount())
    HANDLE_EXPR(get_battle_won_count, GetBattleWonCount(_msg->Int(2)))
    HANDLE_EXPR(get_battle_lost_count, GetBattleLostCount(_msg->Int(2)))
    HANDLE_EXPR(get_won_last_battle, GetWonLastBattle(_msg->Int(2)))
    HANDLE_EXPR(is_fitness_goal_set, mHasValidFitnessGoal)
    HANDLE_EXPR(is_fitness_days_goal_met, IsFitnessDaysGoalMet())
    HANDLE_EXPR(is_fitness_calories_goal_met, IsFitnessCaloriesGoalMet())
    HANDLE_ACTION(
        set_fitness_goals_through, SetFitnessGoalsThrough(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(set_fitness_goal_days, SetFitnessGoalDays(_msg->Obj<HamLabel>(2)))
    HANDLE_ACTION(
        set_fitness_goal_calories, SetFitnessGoalCalories(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(
        set_fitness_goal_days_result, SetFitnessGoalDaysResult(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(
        set_fitness_goal_calories_result,
        SetFitnessGoalCaloriesResult(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(reset_fitness_goal, ResetFitnessGoal())
    HANDLE_ACTION(send_fitness_goal_target_days, SetFitnessGoalTargetDays(_msg->Int(2)))
    HANDLE_ACTION(
        send_fitness_goal_target_calories, SetFitnessGoalTargetCalories(_msg->Int(2))
    )
    HANDLE_ACTION(send_fitness_goal_to_rc, SendFitnessGoalToRC())
    HANDLE_ACTION(set_last_new_song, SetLastNewSong())
    HANDLE_ACTION(update_nag, UpdateNag())
    HANDLE_EXPR(needs_to_be_nagged, NeedsToBeNagged())
    HANDLE_EXPR(nag, Nag())
    HANDLE_ACTION(complete_current_nag, CompleteCurrentNag(_msg->Int(2)))
    HANDLE_ACTION(complete_nag, CompleteNag(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(reset_nags, ResetNags())
    HANDLE_ACTION(refresh_playlists, RefreshPlaylists())
    HANDLE_ACTION(unlock_content, UnlockContent(_msg->Sym(2)))
    HANDLE_MESSAGE(SingleItemEnumCompleteMsg)
    HANDLE_SUPERCLASS(Profile)
END_HANDLERS

bool HamProfile::HasCheated() const {
    return MetaPanel::sUnlockAll || TheProfileMgr.GetAllUnlocked();
}

bool HamProfile::IsUnsaved() const {
    if (HasCheated()) {
        return false;
    }

    if (Profile::IsUnsaved()) {
        return true;
    }

    if (mStats->IsDirty()) {
        return true;
    }

    if (mRank->Dirty()) {
        return true;
    }

    return mMoveRatingHistory->Dirty() != false;
}

bool HamProfile::HasSomethingToUpload() {
    ThePlatformMgr.IsSignedIntoLive(GetPadNum());
    return false;
}

void HamProfile::DeleteAll() {
    mAccomplishmentProgress.Clear();
    if (mScores) {
        mScores->Clear();
    }
    if (mMoveRatingHistory) {
        mMoveRatingHistory->Clear();
    }
    if (mStats) {
        mStats->Clear();
    }
    if (mRank) {
        mRank->Clear();
    }
    mUnlockedContent.clear();
    mNewContent.clear();
    mCampaignProgress[0].Clear();
    mCampaignProgress[1].Clear();
    mCampaignProgress[2].Clear();
    ResetOutfitPrefs();
    for (int i = 0; i < 5; i++) {
        mPlaylists[i].Clear();
    }
    mIsFitnessWeightEntered = false;
    mInFitnessMode = false;
    mExportChecked = kUnknown;
    mFitnessLbs = 130;
    mChallengeTimeStamp = 0;
    mFitnessSessionSecs = 0;
    mHasValidFitnessGoal = false;
    mFitnessSessionCals = 0;
    mFitnessGoalStartDay = 0;
    mFitnessSongCals = 0;
    mFitnessGoalStartMonth = 0;
    mFitnessGoalStartYear = 0;
    mFitnessGoalTargetDays = 0;
    mFitnessGoalTargetCalories = 0;
    mFitnessGoalDaysActive = 0;
    mFitnessGoalCaloriesBurned = 0;
    mFitnessGoalLastDayNumber = 0;
    mFitnessGoalNeedUpload = false;
    mSkippedSongCount = 0;
    mLastNewSong = 0;
    mBootsSinceLastNag = 0;
    mFirstTimeNag = true;
    mNagType = 0;
    mAllowedNagTypes = 3;
    Profile::DeleteAll();
}

const AccomplishmentProgress &HamProfile::GetAccomplishmentProgress() const {
    return mAccomplishmentProgress;
}
AccomplishmentProgress &HamProfile::AccessAccomplishmentProgress() {
    return mAccomplishmentProgress;
}
const CampaignProgress &HamProfile::GetCampaignProgress(Difficulty d) const {
    return mCampaignProgress[d];
}
CampaignProgress &HamProfile::AccessCampaignProgress(Difficulty d) {
    return mCampaignProgress[d];
}
SongStatusMgr *HamProfile::GetSongStatusMgr() const { return mScores; }

bool HamProfile::IsFitnessDaysGoalMet() {
    return mHasValidFitnessGoal && mFitnessGoalDaysActive >= mFitnessGoalTargetDays;
}
bool HamProfile::IsFitnessCaloriesGoalMet() {
    return mHasValidFitnessGoal
        && mFitnessGoalCaloriesBurned >= mFitnessGoalTargetCalories;
}

void HamProfile::SendFitnessGoalToRC() {
    if (mHasValidFitnessGoal) {
        TheFitnessGoalMgr->OnSendFitnessGoalToRC(this);
    } else {
        TheFitnessGoalMgr->DeleteFitnessGoalFromRC(this);
    }
}

void HamProfile::CheckForNinjaUnlock() {}
void HamProfile::CheckForIconManUnlock() {}

bool HamProfile::HasSongStatus(Symbol shortname) const {
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname, false);
    if (songID != 0 && mScores && mScores->HasSongStatus(songID)) {
        return true;
    } else {
        return false;
    }
}

HamUser *HamProfile::GetHamUser() const {
    return TheHamUserMgr->GetUserFromPad(GetPadNum());
}

bool HamProfile::IsOkToUpdateProfile() {
    int partyMode = TheHamProvider->Property("is_in_party_mode")->Int();
    if (partyMode <= 0) {
        int infinite = TheHamProvider->Property("is_in_infinite_party_mode")->Int();
        if (infinite <= 0) {
            return true;
        }
    }
    return false;
}

int HamProfile::GetUploadFriendsToken() const { return mUploadFriendsToken; }
void HamProfile::SetUploadFriendsToken(int token) { mUploadFriendsToken = token; }

Symbol HamProfile::CharacterOutfit(Symbol character) const {
    auto itEnd = mCharPrefs.end();
    auto it = std::find(mCharPrefs.begin(), itEnd, character);
    if (it == itEnd) {
        MILO_NOTIFY("Could not find outfit for %s", character);
        return GetCharacterOutfit(character, false);
    } else {
        return it->mOutfit;
    }
}

void HamProfile::EarnAccomplishment(Symbol s) {
    if (mAccomplishmentProgress.AddAccomplishment(s) && IsOkToUpdateProfile()) {
        mDirty = true;
    }
}

bool HamProfile::HasFinishedCampaign() const {
    return mCampaignProgress[kDifficultyEasy].IsCampaignTanBattleCompleted();
}

bool HamProfile::HasAnyEraSongBeenPlayed(Symbol s) const {
    if (TheCampaign->GetCampaignEra(s)) {
        for (int i = 0; i < kNumDifficulties; i++) {
            if (mCampaignProgress[i].IsEraPlayed(s)) {
                return true;
            }
        }
    }
    return false;
}

bool HamProfile::GetWonLastBattle(int songID) const {
    if (songID != 0 && mScores && mScores->HasSongStatus(songID)) {
        return mScores->GetLastBattleResult(songID);
    } else {
        return false;
    }
}

int HamProfile::GetBattleWonCount(int songID) const {
    if (songID != 0 && mScores && mScores->HasSongStatus(songID)) {
        return mScores->GetTotalBattleWins(songID);
    } else {
        return 0;
    }
}

int HamProfile::GetBattleLostCount(int songID) const {
    if (songID != 0 && mScores && mScores->HasSongStatus(songID)) {
        return mScores->GetTotalBattleLosses(songID);
    } else {
        return 0;
    }
}

void HamProfile::SetFitnessMode(bool inMode) {
    if (IsOkToUpdateProfile()) {
        mInFitnessMode = inMode;
        TheProfileMgr.UpdateUsingFitnessState();
        mDirty = true;
    }
}

float HamProfile::GetFitnessPounds() { return mFitnessLbs; }
float HamProfile::GetKgFromPounds(float lbs) { return lbs / 2.2046227f; }
float HamProfile::GetPoundsFromKgs(float kgs) { return kgs * 2.2046227f; }

void HamProfile::SetFitnessPounds(float lbs) {
    if (IsOkToUpdateProfile()) {
        mFitnessLbs = lbs;
        mIsFitnessWeightEntered = true;
        mDirty = true;
    }
}

// per GamePanel::UpdateFitnessOverlay:
// float 1: total time
// float 2: total calories
// float 3: calories for this song
void HamProfile::GetFitnessStats(float &time, float &calories, float &f3) {
    time = mFitnessSessionSecs;
    calories = mFitnessSessionCals;
    f3 = mFitnessSongCals;
}

void HamProfile::DiscardRecentCampaignProgress() {
    mCampaignProgress[kDifficultyEasy].ResetProgressToBookmark();
    mCampaignProgress[kDifficultyMedium].ResetProgressToBookmark();
    mCampaignProgress[kDifficultyExpert].ResetProgressToBookmark();
}

void HamProfile::UpdateOnlineID() {
    mHasOnlineID = ThePlatformMgr.IsPadNumSignedIn(GetPadNum());
    XUID xuid;
    XUserGetXUID(GetPadNum(), &xuid);
    mOnlineID->SetXUID(xuid);
}

bool HamProfile::NeedsToBeNagged() {
    if (mNagType == 3) {
        return false;
    } else if (mFirstTimeNag) {
        return mBootsSinceLastNag >= 2;
    } else {
        return mBootsSinceLastNag >= 4;
    }
}

Symbol HamProfile::Nag() {
    if (mFirstTimeNag) {
        mFirstTimeNag = false;
    }
    static Symbol main_screen("main_screen");
    static const char *sNagScreens[] = { "pre_redeem_code_nag_screen",
                                         "pre_facebook_nag_screen",
                                         "pre_mobile_app_nag_screen" };
    Symbol out;
    if ((1 << mNagType) & mAllowedNagTypes) {
        out = Symbol(sNagScreens[mNagType]);
    } else {
        MILO_NOTIFY(
            "Attempted to nag, but there is no valid nag screen! Just going to main menu..."
        );
        out = main_screen;
    }
    mBootsSinceLastNag = 0;
    return out;
}

void HamProfile::CompleteCurrentNag(bool b1) {
    if (IsOkToUpdateProfile()) {
        if (b1) {
            mAllowedNagTypes ^= (1 << mNagType);
        }
        int i;
        for (i = 0; i < 3; i++) {
            mNagType = (mNagType + 1) % 3;
            if ((1 << mNagType) & mAllowedNagTypes)
                break;
        }
        if (i == 3) {
            mNagType = 3;
        }
        MakeDirty();
        if (TheSaveLoadMgr) {
            TheSaveLoadMgr->AutoSave();
        }
    }
}

void HamProfile::CompleteNag(int i1, bool b2) {
    if (IsOkToUpdateProfile()) {
        if (b2) {
            mAllowedNagTypes ^= (1 << i1);
        }
        if (mNagType != i1) {
            if (!b2) {
                return;
            }
        } else {
            int i;
            for (i = 0; i < 3; i++) {
                mNagType = (mNagType + 1) % 3;
                if ((1 << mNagType) & mAllowedNagTypes)
                    break;
            }
            if (i == 3) {
                mNagType = 3;
            }
        }
        MakeDirty();
        if (TheSaveLoadMgr) {
            TheSaveLoadMgr->AutoSave();
        }
    }
}

void HamProfile::ResetNags() {
    if (IsOkToUpdateProfile()) {
        mBootsSinceLastNag = 0;
        mFirstTimeNag = true;
        mNagType = 0;
        mAllowedNagTypes = 3;
        MakeDirty();
        if (TheSaveLoadMgr) {
            TheSaveLoadMgr->AutoSave();
        }
    }
}

int HamProfile::GetFlauntCount() {
    if (mStats) {
        return mStats->GetCount((MetagameStats::CountStatID)0x15);
    } else {
        return 0;
    }
}

void HamProfile::IncrementFlauntCount() {
    if (mStats) {
        mStats->IncrementCount((MetagameStats::CountStatID)0x15, 1);
    }
}

void HamProfile::IncrementChallengesMet() {
    if (mStats) {
        mStats->IncrementCount((MetagameStats::CountStatID)0x16, 1);
    }
}

void HamProfile::SetFitnessGoal(
    bool set,
    int startDay,
    int startMonth,
    int startYear,
    int daysActive,
    int calories,
    int daysActiveTd,
    int caloriesTd
) {
    mHasValidFitnessGoal = set;
    mFitnessGoalStartDay = set ? startDay : 0;
    mFitnessGoalStartMonth = set ? startMonth : 0;
    mFitnessGoalStartYear = set ? startYear : 0;
    mFitnessGoalTargetDays = set ? daysActive : 0;
    mFitnessGoalTargetCalories = set ? calories : 0;
    mFitnessGoalDaysActive = set ? daysActiveTd : 0;
    mFitnessGoalCaloriesBurned = set ? caloriesTd : 0;
}

void HamProfile::GetFitnessGoal(int &daysActive, int &calories) {
    daysActive = mFitnessGoalTargetDays;
    calories = mFitnessGoalTargetCalories;
}

void HamProfile::ResetFitnessGoal() {
    if (IsOkToUpdateProfile()) {
        mHasValidFitnessGoal = false;
        mFitnessGoalStartDay = 0;
        mFitnessGoalStartMonth = 0;
        mFitnessGoalStartYear = 0;
        mFitnessGoalTargetDays = 0;
        mFitnessGoalTargetCalories = 0;
        mFitnessGoalDaysActive = 0;
        mFitnessGoalCaloriesBurned = 0;
        mFitnessGoalLastDayNumber = 0;
        mDirty = true;
    }
}

void HamProfile::GetFitnessGoalStatus(int &curDaysActive, int &curCalories) {
    curDaysActive = mFitnessGoalDaysActive;
    curCalories = mFitnessGoalCaloriesBurned;
}

void HamProfile::ClearFitnessGoalNeedUpload() {
    if (IsOkToUpdateProfile()) {
        mFitnessGoalNeedUpload = false;
        mDirty = true;
    }
}

void HamProfile::RefreshPlaylists() {
    if (IsOkToUpdateProfile() && mNeedToRefreshPlaylists) {
        mNeedToRefreshPlaylists = false;
        TheHamSongMgr.InitializePlaylists();
    }
}

Playlist &HamProfile::GetPlaylist(int i_iIndex) {
    MILO_ASSERT_RANGE(i_iIndex, 0, kMaxPlaylists, 0x3B3);
    return mPlaylists[i_iIndex];
}

void HamProfile::SetFitnessGoalTargetDays(int days) {
    if (IsOkToUpdateProfile()) {
        mFitnessGoalTargetDays = days;
        mHasValidFitnessGoal = true;
        DateTime dt;
        GetDateAndTime(dt);
        mFitnessGoalStartYear = dt.Year();
        mFitnessGoalStartMonth = dt.Month();
        mDirty = true;
        mFitnessGoalStartDay = dt.mDay;
    }
}

void HamProfile::SetFitnessGoalTargetCalories(int calories) {
    if (IsOkToUpdateProfile()) {
        mFitnessGoalTargetCalories = calories;
        mHasValidFitnessGoal = true;
        DateTime dt;
        GetDateAndTime(dt);
        mFitnessGoalStartYear = dt.Year();
        mFitnessGoalStartMonth = dt.Month();
        mDirty = true;
        mFitnessGoalStartDay = dt.mDay;
    }
}

bool HamProfile::IsFitnessGoalPeriodExpired() {
    if (mHasValidFitnessGoal) {
        DateTime dt(
            mFitnessGoalStartYear, mFitnessGoalStartMonth, mFitnessGoalStartDay, 0, 0, 0
        );
        int dtNum = dt.ToDayNumber() + 7;
        if (DateTime().ToDayNumber() - dtNum <= 0) {
            return false;
        }
    }
    return true;
}

void HamProfile::SetFitnessGoalDaysResult(HamLabel *label) {
    MILO_ASSERT(label, 0x65D);
    static Symbol fitness_goal_complete("fitness_goal_complete");
    static Symbol fitness_goal_not_met("fitness_goal_not_met");
    if (!mHasValidFitnessGoal) {
        label->SetTextToken(gNullStr);
    } else if (mFitnessGoalDaysActive >= mFitnessGoalTargetDays) {
        label->SetTextToken(fitness_goal_complete);
    } else if (IsFitnessGoalPeriodExpired()) {
        label->SetTextToken(fitness_goal_not_met);
    } else {
        label->SetTextToken(gNullStr);
    }
}

void HamProfile::SetFitnessGoalCaloriesResult(HamLabel *label) {
    MILO_ASSERT(label, 0x67C);
    static Symbol fitness_goal_complete("fitness_goal_complete");
    static Symbol fitness_goal_not_met("fitness_goal_not_met");
    if (!mHasValidFitnessGoal) {
        label->SetTextToken(gNullStr);
    } else if (mFitnessGoalCaloriesBurned >= mFitnessGoalTargetCalories) {
        label->SetTextToken(fitness_goal_complete);
    } else if (IsFitnessGoalPeriodExpired()) {
        label->SetTextToken(fitness_goal_not_met);
    } else {
        label->SetTextToken(gNullStr);
    }
}

void HamProfile::UpdateFlaunt() {
    if (IsOkToUpdateProfile()) {
        int songID = TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong());
        for (int i = 0; i < 2; i++) {
            HamPlayerData *playerData = TheGameData->Player(i);
            MILO_ASSERT(playerData, 0xE5);
            Hmx::Object *playerProvider = playerData->Provider();
            MILO_ASSERT(playerProvider, 0xE8);
            Difficulty diff = playerData->GetDifficulty();
            int playerPad = playerData->PadNum();
            int thisPad = GetPadNum();
            if (playerPad == thisPad) {
                SongStatusMgr *songStatusMgr = GetSongStatusMgr();
                MILO_ASSERT(songStatusMgr, 0xF0);
                static Symbol score("score");
                int scoreProp = playerProvider->Property(score)->Int();
                songStatusMgr->UpdateFlaunt(songID, scoreProp, diff, false);
            }
        }
        mDirty = true;
    }
}

void HamProfile::SetCharacterOutfit(Symbol character, Symbol outfit) {
    if (IsOkToUpdateProfile()) {
        auto itEnd = mCharPrefs.end();
        auto it = std::find(mCharPrefs.begin(), itEnd, character);
        if (it == itEnd) {
            MILO_NOTIFY("Could not find %s", character);
        } else {
            MILO_ASSERT(GetOutfitCharacter(outfit) == character, 0x123);
            it->mOutfit = outfit;
            mDirty = true;
        }
    }
}

bool HamProfile::IsContentUnlockedForProfile(Symbol content) const {
    if (MetaPanel::sUnlockAll)
        return true;
    else
        return std::find(mUnlockedContent.begin(), mUnlockedContent.end(), content)
            != mUnlockedContent.end();
}

bool HamProfile::IsContentNew(Symbol content) const {
    if (MetaPanel::sUnlockAll)
        return false;
    else
        return std::find(mNewContent.begin(), mNewContent.end(), content)
            != mNewContent.end();
}

void HamProfile::SetFitnessStats(int, float calories, float time) {
    if (IsOkToUpdateProfile()) {
        int nearestTime = Round(time);
        int nearestCalories = Round(calories);
        mFitnessSessionSecs += nearestTime;
        mFitnessSessionCals += nearestCalories;
        mFitnessSongCals = nearestCalories;
        mStats->IncrementCount((MetagameStats::CountStatID)0xC, nearestTime);
        mStats->IncrementCount((MetagameStats::CountStatID)0xD, nearestCalories);
        if (mStats->GetCount((MetagameStats::CountStatID)0xD) >= 100) {
            static Symbol acc_100_calorie_burn("acc_100_calorie_burn");
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                this, acc_100_calorie_burn, true
            );
        }
        if (mHasValidFitnessGoal && !IsFitnessGoalPeriodExpired()) {
            mFitnessGoalCaloriesBurned += nearestCalories;
            DateTime dt;
            GetDateAndTime(dt);
            int dayNum = dt.ToDayNumber();
            if (dayNum != mFitnessGoalLastDayNumber) {
                mFitnessGoalLastDayNumber = dayNum;
                mFitnessGoalDaysActive++;
            }
            mFitnessGoalNeedUpload = true;
        }
        mDirty = true;
    }
}

void HamProfile::SaveLoadComplete(ProfileSaveState state) {
    Profile::SaveLoadComplete(state);
    static ProfileChangedMsg msg(this);
    msg[0] = this;
    TheProfileMgr.Handle(msg, false);
    if (mExportChecked == kUnknown) {
        int padnum = GetPadNum();
        if (ThePlatformMgr.IsSignedIntoLive(padnum)) {
            mExportChecked = kEnumerating;
            ThePlatformMgr.QueueEnumJob(
                new SingleItemEnumJob(this, padnum, 0x373307d200000004)
            );
        }
    }
}

void HamProfile::UpdateFitnessTime(HamLabel *label) {
    MILO_ASSERT(label, 0x3BA);
    int time = mFitnessSessionSecs;
    static Symbol time_text("time_text");
    label->SetTokenFmt(time_text, FormatTimeHMS(time));
}

void HamProfile::UpdateFitnessTotalTime(HamLabel *label) {
    MILO_ASSERT(label, 0x3C4);
    int count = mStats->GetCount((MetagameStats::CountStatID)0xC);
    static Symbol time_text("time_text");
    label->SetTokenFmt(time_text, FormatTimeHMS(count));
}

void HamProfile::UpdateFitnessWeight(HamLabel *label) {
    MILO_ASSERT(label, 0x3CE);
    if (mIsFitnessWeightEntered) {
        float weight_lbs = mFitnessLbs;
        if (!TheProfileMgr.GetPreferredWeightUnit()) {
            static Symbol weight_pounds("weight_pounds");
            label->SetTokenFmt(weight_pounds, (int)weight_lbs);
        } else {
            float kgs = GetKgFromPounds(weight_lbs);
            static Symbol weight_kgs("weight_kgs");
            label->SetTokenFmt(weight_kgs, kgs);
        }
    } else {
        static Symbol fitness_weight_unknown("fitness_weight_unknown");
        label->SetTextToken(fitness_weight_unknown);
    }
}

void HamProfile::UpdateInfinitePlaylistTime(HamLabel *label) {
    MILO_ASSERT(label, 0x43C);
    int count = mStats->GetCount((MetagameStats::CountStatID)0x12);
    static Symbol time_text("time_text");
    label->SetTokenFmt(time_text, FormatTimeHMS(count));
}

void HamProfile::SetFitnessGoalsThrough(HamLabel *label) {
    MILO_ASSERT(label, 0x624);
    static Symbol fitness_goals_through_fmt("fitness_goals_through_fmt");
    if (!mHasValidFitnessGoal) {
        if (DateTime::GetDateFormatting() == kMDY) {
            label->SetTokenFmt(fitness_goals_through_fmt, "MM/DD/YY");
        } else {
            label->SetTokenFmt(fitness_goals_through_fmt, "");
        }
    } else {
        DateTime dt(
            mFitnessGoalStartYear, mFitnessGoalStartMonth, mFitnessGoalStartDay, 0, 0, 0
        );
        dt.FromDayNumber(dt.ToDayNumber() + 7);
        String str;
        dt.ToDateString(str);
        label->SetTokenFmt(fitness_goals_through_fmt, str.c_str());
    }
}

void HamProfile::SetFitnessGoalDays(HamLabel *label) {
    MILO_ASSERT(label, 0x63D);
    static Symbol fitness_goal_stat_fmt("fitness_goal_stat_fmt");
    if (!mHasValidFitnessGoal) {
        label->SetTokenFmt(fitness_goal_stat_fmt, 0, 0);
    } else {
        label->SetTokenFmt(
            fitness_goal_stat_fmt, mFitnessGoalDaysActive, mFitnessGoalTargetDays
        );
    }
}

void HamProfile::SetFitnessGoalCalories(HamLabel *label) {
    MILO_ASSERT(label, 0x64D);
    static Symbol fitness_goal_stat_fmt("fitness_goal_stat_fmt");
    if (!mHasValidFitnessGoal) {
        label->SetTokenFmt(fitness_goal_stat_fmt, 0, 0);
    } else {
        label->SetTokenFmt(
            fitness_goal_stat_fmt, mFitnessGoalCaloriesBurned, mFitnessGoalTargetCalories
        );
    }
}

void HamProfile::MarkContentNotNew(Symbol content) {
    if (IsOkToUpdateProfile()) {
        auto it = std::find(mNewContent.begin(), mNewContent.end(), content);
        if (it != mNewContent.end()) {
            mNewContent.erase(it);
            mDirty = true;
        }
    }
}

void HamProfile::MarkContentNew(Symbol content) {
    if (IsOkToUpdateProfile()) {
        auto it = std::find(mNewContent.begin(), mNewContent.end(), content);
        if (it == mNewContent.end()) {
            mNewContent.push_back(content);
            mDirty = true;
        }
    }
}

void HamProfile::UnlockContent(Symbol content) {
    if (IsOkToUpdateProfile()) {
        MILO_ASSERT(TheProfileMgr.IsUnlockableContent(content), 0x170);
        if (std::find(mUnlockedContent.begin(), mUnlockedContent.end(), content)
            == mUnlockedContent.end()) {
            mUnlockedContent.push_back(content);
            MarkContentNew(content);
            mDirty = true;
            mNeedToRefreshPlaylists = true;
        }
    }
}

int HamProfile::SaveSize(int i1) {
    int ret = 0x328;
    ret += SongStatusMgr::SaveSize(i1);
    ret += AccomplishmentProgress::SaveSize(i1);
    ret += CampaignProgress::SaveSize(i1);
    ret += CampaignProgress::SaveSize(i1);
    ret += CampaignProgress::SaveSize(i1);
    ret += MetagameStats::SaveSize(i1);
    ret += MetagameRank::SaveSize(i1);
    ret += MoveRatingHistory::SaveSize(i1);
    ret += 0xBC;
    ret += CustomPlaylist::SaveSize(i1) * 5;
    ret += 0x45;
    return ret;
}

DataNode HamProfile::OnMsg(const SingleItemEnumCompleteMsg &msg) {
    if (mExportChecked != kEnumerating) {
        return 0;
    }
    mExportChecked = kUnknown;
    return 0;
}

void HamProfile::SetLastNewSong() {
    if (IsOkToUpdateProfile() && TheRockCentral.IsOnline()
        && 0 < TheRockCentral.GetLastNewSong()) {
        MILO_LOG(
            "---- Updating mLastNewSong from %i to %i\n",
            mLastNewSong,
            TheRockCentral.GetLastNewSong()
        );
        int i = TheRockCentral.GetLastNewSong();
        mDirty = true;
        mLastNewSong = i;
    }
}

void HamProfile::ResetOutfitPrefs() {
    if (IsOkToUpdateProfile()) {
        mCharPrefs.clear();
        int playableCount = 0;
        int numChars = GetNumCharacters();
        for (int i = 0; i < numChars; i++) {
            DataArray *charEntry = GetCharacterEntry(i);
            Symbol charSym = charEntry->Sym(0);
            static Symbol playable("playable");
            bool dataFound = true;
            charEntry->FindData(playable, dataFound, false);
            if (dataFound) {
                Symbol outfit = GetCharacterOutfit(charSym, 0);
                CharacterPref pref;
                pref.mChar = charSym;
                pref.mOutfit = outfit;
                pref.mVOIndex = -1;
                mCharPrefs.push_back(pref);
                playableCount++;
            }
        }
        MILO_ASSERT(playableCount == kNumCharacters, 0x147);
    }
}

void HamProfile::UpdateBattleScore(
    int songID, const HamPlayerData *playerdata, int stars, bool b
) {
    if (IsOkToUpdateProfile()) {
        if (playerdata) {
            mRank->UpdateScore(songID, playerdata, mScores, stars, 0);
            mDirty = mDirty || mRank->Dirty();
        }
        bool updatedSong = mScores->UpdateBattleSong(songID, stars, b);
        mDirty = mDirty || updatedSong;
    }
}

void HamProfile::UpdateScore(
    int songID,
    HamPlayerData const *playerdata,
    Difficulty diff,
    int score,
    int i3,
    int stars,
    int numNices,
    int numPerfects,
    int percentPassed,
    int i8,
    bool b1,
    bool b2
) {
    if (IsOkToUpdateProfile()) {
        if (playerdata) {
            mRank->UpdateScore(songID, playerdata, mScores, score, stars);
            mDirty = mDirty || mRank->Dirty();
        }
        bool updateSong = mScores->UpdateSong(
            songID,
            score,
            i3,
            diff,
            i8,
            stars,
            numNices,
            numPerfects,
            percentPassed,
            b1,
            b2,
            false
        ); // i hated this so much why didnt it go in order
        mDirty = mDirty || updateSong;

        static Symbol challenge("challenge");
        if (TheGameMode->InMode(challenge)) {
            bool updateFlaunt = mScores->UpdateFlaunt(songID, score, diff, b1);
            mDirty = mDirty || updateFlaunt;
        }

        if (!b1 && TheLeaderboards) {
            TheLeaderboards->UploadScores(this);
        }

        if (!b1) {
            TheFitnessGoalMgr->UpdateFitnessGoal(this);
        }
    }
}

char const *HamProfile::NextOutfitSample(Symbol s) {
    auto it = std::find(mCharPrefs.begin(), mCharPrefs.end(), s);
    if (it == mCharPrefs.end()) {
        MILO_NOTIFY("Could not find vo index for %s", s);
        return gNullStr;
    } else {
        int idx = it->mVOIndex;
        if (idx == -1) {
            idx = RandomInt(0, 2);
        }
        int set = (idx + 1) % 2;
        if (set < 0) {
            set += 2;
        }
        it->mVOIndex = set;
        return FileLocalize(
            MakeString("sfx/loc/eng/%s/%s_voicemail_%02d.mogg", s, s, idx), nullptr
        );
    }
}
