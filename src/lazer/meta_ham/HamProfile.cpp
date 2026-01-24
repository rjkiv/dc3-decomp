#include "meta_ham/HamProfile.h"
#include "flow/PropertyEventProvider.h"
#include "game/HamUser.h"
#include "game/HamUserMgr.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
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
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "utl/JobMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

HamProfile::HamProfile(int i1)
    : Profile(i1), mAccProgress(this), unk2fc(0), mInFitnessMode(0), mFitnessPounds(130),
      mIsFitnessWeightEntered(0), mFitnessTime(0), mFitnessCalories(0), unk310(0),
      mUploadFriendsToken(0), mOnlineID(new OnlineID()), mSignedIn(0), unk320(0),
      unk324(0), mSkippedSongCount(0), unk32c(0), unk330(0), unk334(0), unk338(gNullStr),
      mIsFitnessGoalSet(0), mFitnessGoalStartDay(0), mFitnessGoalStartMonth(0),
      mFitnessGoalStartYear(0), mFitnessGoalDaysActive(0), mFitnessGoalCalories(0),
      mTrackedDaysActive(0), mTrackedCalories(0), unk35c(0), unk360(0), mProfileTime(0),
      unk368(0), unk36c(1), unk370(0), unk374(3) {
    mSaveSizeMethod = SaveSize;
    mSongStatusMgr = new SongStatusMgr(&TheHamSongMgr);
    mStats = new MetagameStats();
    mRank = new MetagameRank(this);
    mRatingHistory = new MoveRatingHistory();
    mCampaignProgress[0].SetProfile(this);
    mCampaignProgress[1].SetProfile(this);
    mCampaignProgress[2].SetProfile(this);
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
    delete mSongStatusMgr;
    delete mStats;
    delete mRank;
    delete mRatingHistory;
}

void HamProfile::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << *mSongStatusMgr;
    FixedSizeSaveable::SaveStd(fs, mUnlockedContent, 100);
    FixedSizeSaveable::SaveStd(fs, mNewContent, 100);
    fs << mAccProgress;
    fs << mCampaignProgress[0];
    fs << mCampaignProgress[1];
    fs << mCampaignProgress[2];
    fs << *mStats;
    fs << *mRank;
    fs << *mRatingHistory;
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
    fs << mFitnessPounds;
    fs << mIsFitnessWeightEntered;
    fs << unk324;
    fs << mFitnessGoalStartDay;
    fs << mFitnessGoalStartMonth;
    fs << mFitnessGoalStartYear;
    fs << mFitnessGoalDaysActive;
    fs << mFitnessGoalCalories;
    fs << mTrackedDaysActive;
    fs << mTrackedCalories;
    fs << unk35c;
    fs << unk32c;
    fs << mProfileTime;
    fs << unk368;
    fs << unk36c;
    fs << unk370;
    fs << unk374;
    fs << mIsFitnessGoalSet;
    fs << unk360;
    const_cast<HamProfile *>(this)->mDirty = false;
}

void HamProfile::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    DeleteAll();
    fs >> *mSongStatusMgr;
    FixedSizeSaveable::LoadStd(fs, mUnlockedContent, 100);
    FixedSizeSaveable::LoadStd(fs, mNewContent, 100);
    fs >> mAccProgress;
    fs >> mCampaignProgress[0];
    fs >> mCampaignProgress[1];
    fs >> mCampaignProgress[2];
    fs >> *mStats;
    fs >> *mRank;
    fs >> *mRatingHistory;
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
    fs >> mFitnessPounds;
    fs >> mIsFitnessWeightEntered;
    fs >> unk324;
    fs >> mFitnessGoalStartDay;
    fs >> mFitnessGoalStartMonth;
    fs >> mFitnessGoalStartYear;
    fs >> mFitnessGoalDaysActive;
    fs >> mFitnessGoalCalories;
    fs >> mTrackedDaysActive;
    fs >> mTrackedCalories;
    fs >> unk35c;
    mSkippedSongCount = 0;
    fs >> unk32c;
    fs >> mProfileTime;
    fs >> unk368;
    fs >> unk36c;
    fs >> unk370;
    fs >> unk374;
    fs >> mIsFitnessGoalSet;
    fs >> unk360;
    mDirty = false;
    unk2fc = false;
}

BEGIN_HANDLERS(HamProfile)
    HANDLE_EXPR(get_accomplishment_progress, &mAccProgress)
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
    HANDLE_EXPR(get_fitness_pounds, mFitnessPounds)
    HANDLE_ACTION(
        set_fitness_stats, SetFitnessStats(_msg->Int(2), _msg->Float(3), _msg->Float(4))
    )
    HANDLE_ACTION(toggle_fitness_mode, SetFitnessMode(!mInFitnessMode))
    HANDLE_EXPR(get_fitness_time, (int)mFitnessTime)
    HANDLE_ACTION(update_fitness_time, UpdateFitnessTime(_msg->Obj<HamLabel>(2)))
    HANDLE_ACTION(
        update_fitness_total_time, UpdateFitnessTotalTime(_msg->Obj<HamLabel>(2))
    )
    HANDLE_ACTION(update_fitness_weight, UpdateFitnessWeight(_msg->Obj<HamLabel>(2)))
    HANDLE_EXPR(get_fitness_calories, (int)mFitnessCalories)
    HANDLE_ACTION(
        update_infinite_playlist_time, UpdateInfinitePlaylistTime(_msg->Obj<HamLabel>(2))
    )
    HANDLE_EXPR(get_skipped_song_count, mSkippedSongCount)
    HANDLE_ACTION(increment_skipped_song_count, IncrementSkippedSongCount())
    HANDLE_EXPR(get_battle_won_count, GetBattleWonCount(_msg->Int(2)))
    HANDLE_EXPR(get_battle_lost_count, GetBattleLostCount(_msg->Int(2)))
    HANDLE_EXPR(get_won_last_battle, GetWonLastBattle(_msg->Int(2)))
    HANDLE_EXPR(is_fitness_goal_set, mIsFitnessGoalSet)
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
    } else {
    }
}

bool HamProfile::HasSomethingToUpload() {
    ThePlatformMgr.IsSignedIntoLive(GetPadNum());
    return false;
}

void HamProfile::DeleteAll() {
    mAccProgress.Clear();
    if (mSongStatusMgr) {
        mSongStatusMgr->Clear();
    }
    if (mRatingHistory) {
        mRatingHistory->Clear();
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
    unk320 = 0;
    mFitnessPounds = 130;
    unk324 = 0;
    mFitnessTime = 0;
    mIsFitnessGoalSet = false;
    mFitnessCalories = 0;
    mFitnessGoalStartDay = 0;
    unk310 = 0;
    mFitnessGoalStartMonth = 0;
    mFitnessGoalStartYear = 0;
    mFitnessGoalDaysActive = 0;
    mFitnessGoalCalories = 0;
    mTrackedDaysActive = 0;
    mTrackedCalories = 0;
    unk35c = 0;
    unk360 = false;
    mSkippedSongCount = 0;
    mProfileTime = 0;
    unk368 = 0;
    unk36c = true;
    unk370 = 0;
    unk374 = 3;
    Profile::DeleteAll();
}

const AccomplishmentProgress &HamProfile::GetAccomplishmentProgress() const {
    return mAccProgress;
}
AccomplishmentProgress &HamProfile::AccessAccomplishmentProgress() {
    return mAccProgress;
}
const CampaignProgress &HamProfile::GetCampaignProgress(Difficulty d) const {
    return mCampaignProgress[d];
}
CampaignProgress &HamProfile::AccessCampaignProgress(Difficulty d) {
    return mCampaignProgress[d];
}
SongStatusMgr *HamProfile::GetSongStatusMgr() const { return mSongStatusMgr; }

bool HamProfile::IsFitnessDaysGoalMet() {
    return mIsFitnessGoalSet && mTrackedDaysActive >= mFitnessGoalDaysActive;
}
bool HamProfile::IsFitnessCaloriesGoalMet() {
    return mIsFitnessGoalSet && mTrackedCalories >= mFitnessGoalCalories;
}

void HamProfile::SendFitnessGoalToRC() {
    if (mIsFitnessGoalSet) {
        TheFitnessGoalMgr->OnSendFitnessGoalToRC(this);
    } else {
        TheFitnessGoalMgr->DeleteFitnessGoalFromRC(this);
    }
}

void HamProfile::CheckForNinjaUnlock() {}
void HamProfile::CheckForIconManUnlock() {}

bool HamProfile::HasSongStatus(Symbol shortname) const {
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname, false);
    if (songID != 0 && mSongStatusMgr && mSongStatusMgr->HasSongStatus(songID)) {
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
    if (mAccProgress.AddAccomplishment(s) && IsOkToUpdateProfile()) {
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
    if (songID != 0 && mSongStatusMgr && mSongStatusMgr->HasSongStatus(songID)) {
        return mSongStatusMgr->GetLastBattleResult(songID);
    } else {
        return false;
    }
}

int HamProfile::GetBattleWonCount(int songID) const {
    if (songID != 0 && mSongStatusMgr && mSongStatusMgr->HasSongStatus(songID)) {
        return mSongStatusMgr->GetTotalBattleWins(songID);
    } else {
        return 0;
    }
}

int HamProfile::GetBattleLostCount(int songID) const {
    if (songID != 0 && mSongStatusMgr && mSongStatusMgr->HasSongStatus(songID)) {
        return mSongStatusMgr->GetTotalBattleLosses(songID);
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

float HamProfile::GetFitnessPounds() { return mFitnessPounds; }
float HamProfile::GetKgFromPounds(float lbs) { return lbs / 2.2046227f; }
float HamProfile::GetPoundsFromKgs(float kgs) { return kgs * 2.2046227f; }

void HamProfile::SetFitnessPounds(float lbs) {
    if (IsOkToUpdateProfile()) {
        mFitnessPounds = lbs;
        mIsFitnessWeightEntered = true;
        mDirty = true;
    }
}

// per GamePanel::UpdateFitnessOverlay:
// float 1: total time
// float 2: total calories
// float 3: calories for this song
void HamProfile::GetFitnessStats(float &time, float &calories, float &f3) {
    time = mFitnessTime;
    calories = mFitnessCalories;
    f3 = unk310;
}

void HamProfile::DiscardRecentCampaignProgress() {
    mCampaignProgress[kDifficultyEasy].ResetProgressToBookmark();
    mCampaignProgress[kDifficultyMedium].ResetProgressToBookmark();
    mCampaignProgress[kDifficultyExpert].ResetProgressToBookmark();
}

void HamProfile::UpdateOnlineID() {
    mSignedIn = ThePlatformMgr.IsPadNumSignedIn(GetPadNum());
    XUID xuid;
    XUserGetXUID(GetPadNum(), &xuid);
    mOnlineID->SetXUID(xuid);
}

bool HamProfile::NeedsToBeNagged() {
    if (unk370 == 3) {
        return false;
    } else if (unk36c) {
        return unk368 >= 2;
    } else {
        return unk368 >= 4;
    }
}

Symbol HamProfile::Nag() {
    if (unk36c) {
        unk36c = false;
    }
    static Symbol main_screen("main_screen");
    static const char *sNagScreens[] = { "pre_redeem_code_nag_screen",
                                         "pre_facebook_nag_screen",
                                         "pre_mobile_app_nag_screen" };
    Symbol out;
    if ((1 << unk370) & unk374) {
        out = Symbol(sNagScreens[unk370]);
    } else {
        MILO_NOTIFY(
            "Attempted to nag, but there is no valid nag screen! Just going to main menu..."
        );
        out = main_screen;
    }
    unk368 = 0;
    return out;
}

void HamProfile::CompleteCurrentNag(bool b1) {
    if (IsOkToUpdateProfile()) {
        if (b1) {
            unk374 ^= (1 << unk370);
        }
        int i;
        for (i = 0; i < 3; i++) {
            unk370 = (unk370 + 1) % 3;
            if ((1 << unk370) & unk374)
                break;
        }
        if (i == 3) {
            unk370 = 3;
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
            unk374 ^= (1 << i1);
        }
        if (unk370 != i1) {
            if (!b2) {
                return;
            }
        } else {
            int i;
            for (i = 0; i < 3; i++) {
                unk370 = (unk370 + 1) % 3;
                if ((1 << unk370) & unk374)
                    break;
            }
            if (i == 3) {
                unk370 = 3;
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
        unk368 = 0;
        unk36c = true;
        unk370 = 0;
        unk374 = 3;
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
    mIsFitnessGoalSet = set;
    mFitnessGoalStartDay = set ? startDay : 0;
    mFitnessGoalStartMonth = set ? startMonth : 0;
    mFitnessGoalStartYear = set ? startYear : 0;
    mFitnessGoalDaysActive = set ? daysActive : 0;
    mFitnessGoalCalories = set ? calories : 0;
    mTrackedDaysActive = set ? daysActiveTd : 0;
    mTrackedCalories = set ? caloriesTd : 0;
}

void HamProfile::GetFitnessGoal(int &daysActive, int &calories) {
    daysActive = mFitnessGoalDaysActive;
    calories = mFitnessGoalCalories;
}

void HamProfile::ResetFitnessGoal() {
    if (IsOkToUpdateProfile()) {
        mIsFitnessGoalSet = false;
        mFitnessGoalStartDay = 0;
        mFitnessGoalStartMonth = 0;
        mFitnessGoalStartYear = 0;
        mFitnessGoalDaysActive = 0;
        mFitnessGoalCalories = 0;
        mTrackedDaysActive = 0;
        mTrackedCalories = 0;
        unk35c = 0;
        mDirty = true;
    }
}

void HamProfile::GetFitnessGoalStatus(int &curDaysActive, int &curCalories) {
    curDaysActive = mTrackedDaysActive;
    curCalories = mTrackedCalories;
}

void HamProfile::ClearFitnessGoalNeedUpload() {
    if (IsOkToUpdateProfile()) {
        unk360 = false;
        mDirty = true;
    }
}

void HamProfile::RefreshPlaylists() {
    if (IsOkToUpdateProfile() && unk2fc) {
        unk2fc = false;
        TheHamSongMgr.InitializePlaylists();
    }
}

Playlist &HamProfile::GetPlaylist(int i_iIndex) {
    MILO_ASSERT_RANGE(i_iIndex, 0, kMaxPlaylists, 0x3B3);
    return mPlaylists[i_iIndex];
}

void HamProfile::SetFitnessGoalTargetDays(int days) {
    if (IsOkToUpdateProfile()) {
        mFitnessGoalDaysActive = days;
        mIsFitnessGoalSet = true;
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
        mFitnessGoalCalories = calories;
        mIsFitnessGoalSet = true;
        DateTime dt;
        GetDateAndTime(dt);
        mFitnessGoalStartYear = dt.Year();
        mFitnessGoalStartMonth = dt.Month();
        mDirty = true;
        mFitnessGoalStartDay = dt.mDay;
    }
}

bool HamProfile::IsFitnessGoalPeriodExpired() {
    if (mIsFitnessGoalSet) {
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
    if (!mIsFitnessGoalSet) {
        label->SetTextToken(gNullStr);
    } else if (mTrackedDaysActive >= mFitnessGoalDaysActive) {
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
    if (!mIsFitnessGoalSet) {
        label->SetTextToken(gNullStr);
    } else if (mTrackedCalories >= mFitnessGoalCalories) {
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
        mFitnessTime += nearestTime;
        mFitnessCalories += nearestCalories;
        unk310 = nearestCalories;
        mStats->IncrementCount((MetagameStats::CountStatID)0xC, nearestTime);
        mStats->IncrementCount((MetagameStats::CountStatID)0xD, nearestCalories);
        if (mStats->GetCount((MetagameStats::CountStatID)0xD) >= 100) {
            static Symbol acc_100_calorie_burn("acc_100_calorie_burn");
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                this, acc_100_calorie_burn, true
            );
        }
        if (mIsFitnessGoalSet && !IsFitnessGoalPeriodExpired()) {
            mTrackedCalories += nearestCalories;
            DateTime dt;
            GetDateAndTime(dt);
            int dayNum = dt.ToDayNumber();
            if (dayNum != unk35c) {
                unk35c = dayNum;
                mTrackedDaysActive++;
            }
            unk360 = true;
        }
        mDirty = true;
    }
}

void HamProfile::SaveLoadComplete(ProfileSaveState state) {
    Profile::SaveLoadComplete(state);
    static ProfileChangedMsg msg(this);
    msg[0] = this;
    TheProfileMgr.Handle(msg, false);
    if (unk320 == 0) {
        int padnum = GetPadNum();
        if (ThePlatformMgr.IsSignedIntoLive(padnum)) {
            unk320 = 1;
            ThePlatformMgr.QueueEnumJob(
                new SingleItemEnumJob(this, padnum, 0x373307d200000004)
            );
        }
    }
}

void HamProfile::UpdateFitnessTime(HamLabel *label) {
    MILO_ASSERT(label, 0x3BA);
    int time = mFitnessTime;
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
        float weight_lbs = mFitnessPounds;
        if (!TheProfileMgr.GetUnk4c()) {
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
    if (!mIsFitnessGoalSet) {
        label->SetTokenFmt(
            fitness_goals_through_fmt,
            DateTime::GetDateFormatting() == kMDY ? "MM/DD/YY" : ""
        );
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
    int i2, i3;
    if (!mIsFitnessGoalSet) {
        i3 = 0;
        i2 = 0;
    } else {
        i3 = mFitnessGoalDaysActive;
        i2 = mTrackedDaysActive;
    }
    label->SetTokenFmt(fitness_goal_stat_fmt, i2, i3);
}

void HamProfile::SetFitnessGoalCalories(HamLabel *label) {
    MILO_ASSERT(label, 0x64D);
    static Symbol fitness_goal_stat_fmt("fitness_goal_stat_fmt");
    int i2, i3;
    if (!mIsFitnessGoalSet) {
        i3 = 0;
        i2 = 0;
    } else {
        i3 = mFitnessGoalCalories;
        i2 = mTrackedCalories;
    }
    label->SetTokenFmt(fitness_goal_stat_fmt, i2, i3);
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
            unk2fc = true;
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
    if (unk320 != 1) {
        return 0;
    }
    unk320 = 0;
    return 0;
}

void HamProfile::SetLastNewSong() {
    if (IsOkToUpdateProfile() && TheRockCentral.IsOnline()
        && 0 < TheRockCentral.GetRockCentralTime()) {
        MILO_LOG(
            "---- Updating mLastNewSong from %i to %i\n",
            mProfileTime,
            TheRockCentral.GetRockCentralTime()
        );
        int i = TheRockCentral.GetRockCentralTime();
        mDirty = true;
        mProfileTime = i;
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
                pref.mVoicemailIdx = -1;
                mCharPrefs.push_back(pref);
                playableCount++;
            }
        }
        MILO_ASSERT(playableCount == kNumCharacters, 0x147);
    }
}
