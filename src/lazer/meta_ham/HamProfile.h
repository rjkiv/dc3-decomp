#pragma once
#include "game/HamUser.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
#include "meta/Profile.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/CampaignProgress.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/MetagameStats.h"
#include "meta_ham/MoveRatingHistory.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/SongStatusMgr.h"
#include "os/OnlineID.h"
#include "utl/JobMgr.h"
#include "utl/Symbol.h"

struct CharacterPref {
    bool operator==(Symbol s) const { return mChar == s; }

    Symbol mChar; // 0x0
    Symbol mOutfit; // 0x4
    int mVOIndex; // 0x8
};

class HamProfile : public Profile {
public:
    enum {
        kMaxPlaylists = 5,
        kNumCharacters = 23
    };

    enum ExportCheckState {
        kUnknown = 0x0000,
        kEnumerating = 0x0001,
        kPurchased = 0x0002,
        kNotPurchased = 0x0003
    };

    HamProfile(int);
    virtual ~HamProfile();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    virtual bool HasCheated() const;
    virtual bool IsUnsaved() const;
    virtual void SaveLoadComplete(ProfileSaveState);
    virtual bool HasSomethingToUpload();
    virtual void DeleteAll();

    static int SaveSize(int);

    void CheckForIconManUnlock();
    void CheckForNinjaUnlock();
    void SetFitnessMode(bool);
    HamUser *GetHamUser() const;
    SongStatusMgr *GetSongStatusMgr() const;
    const CampaignProgress &GetCampaignProgress(Difficulty) const;
    CampaignProgress &AccessCampaignProgress(Difficulty);
    const AccomplishmentProgress &GetAccomplishmentProgress() const;
    AccomplishmentProgress &AccessAccomplishmentProgress();
    void EarnAccomplishment(Symbol);
    void GetFitnessStats(float &, float &, float &);
    void UnlockContent(Symbol);
    void MarkContentNew(Symbol);
    void MarkContentNotNew(Symbol);
    Symbol CharacterOutfit(Symbol) const;
    void SetCharacterOutfit(Symbol, Symbol);
    const char *NextOutfitSample(Symbol);
    bool HasSongStatus(Symbol) const;
    void SetFitnessPounds(float);
    void SetFitnessStats(int, float, float);
    void UpdateFitnessTime(HamLabel *);
    void UpdateFitnessTotalTime(HamLabel *);
    void UpdateFitnessWeight(HamLabel *);
    void UpdateInfinitePlaylistTime(HamLabel *);
    int GetBattleWonCount(int) const;
    int GetBattleLostCount(int) const;
    bool GetWonLastBattle(int) const;
    void SetFitnessGoalsThrough(HamLabel *);
    void SetFitnessGoalDays(HamLabel *);
    void SetFitnessGoalCalories(HamLabel *);
    void SetFitnessGoalDaysResult(HamLabel *);
    void SetFitnessGoalCaloriesResult(HamLabel *);
    void ResetFitnessGoal();
    void SetFitnessGoalTargetDays(int);
    void SetFitnessGoalTargetCalories(int);
    void SendFitnessGoalToRC();
    void SetLastNewSong();
    bool NeedsToBeNagged();
    Symbol Nag();
    void CompleteCurrentNag(bool);
    void CompleteNag(int, bool);
    void ResetNags();
    bool IsFitnessDaysGoalMet();
    bool IsFitnessCaloriesGoalMet();
    bool IsOkToUpdateProfile();
    int GetUploadFriendsToken() const;
    void SetUploadFriendsToken(int);
    bool HasFinishedCampaign() const;
    bool HasAnyEraSongBeenPlayed(Symbol) const;
    float GetFitnessPounds();
    float GetKgFromPounds(float);
    float GetPoundsFromKgs(float);
    void DiscardRecentCampaignProgress();
    void UpdateOnlineID();
    int GetFlauntCount();
    void IncrementFlauntCount();
    void IncrementChallengesMet();
    void SetFitnessGoal(bool, int, int, int, int, int, int, int);
    void GetFitnessGoal(int &, int &);
    void GetFitnessGoalStatus(int &, int &);
    void ClearFitnessGoalNeedUpload();
    Playlist &GetPlaylist(int);
    bool IsFitnessGoalPeriodExpired();
    void UpdateFlaunt();
    bool IsContentUnlockedForProfile(Symbol) const;
    bool IsContentNew(Symbol) const;
    bool IsDifficultyUnlockedForProfile(Symbol, Symbol);
    void UpdateBattleScore(int, const HamPlayerData *, int, bool);
    void UpdateScore(
        int, HamPlayerData const *, Difficulty, int, int, int, int, int, int, int, bool, bool
    );

    void IncrementSkippedSongCount() { mSkippedSongCount++; }
    void UpdateNag() { mBootsSinceLastNag++; }
    MoveRatingHistory *GetMoveRatingHistory() const { return mMoveRatingHistory; }
    bool InFitnessMode() { return mInFitnessMode; }
    MetagameRank *GetRank() const { return mRank; }
    MetagameStats *GetStats() const { return mStats; }
    const OnlineID *GetOnlineID() const { return mOnlineID; }
    bool HasOnlineID() const { return mHasOnlineID; }
    float GetFitnessTime() const { return mFitnessSessionSecs; }
    float GetFitnessCalories() const { return mFitnessSessionCals; }
    int GetLastNewSong() { return mLastNewSong; }
    bool FitnessGoalNeedUpload() { return mFitnessGoalNeedUpload; }
    void SetLastPracticeSong(Symbol s) { mLastPracticeSong = s; }
    Symbol GetLastPracticeSong() { return mLastPracticeSong; }
    void SetChallengeTimestamp(int i) { mChallengeTimeStamp = i; }
    int GetChallengeTimestamp() { return mChallengeTimeStamp; }
    void SetLastModePractice(bool b) { mLastModePractice = b; }
    bool IsLastModePractice() { return mLastModePractice; }
    int GetLastBestScore() { return mLastBestScore; }
    void SetLastBestScore(int i) { mLastBestScore = i; }
    int GetUsedUnlockableOutfitCount() { return mUsedUnlockableOutfitCount; }
    void IncrementUsedUnlockableOutfitCount() { mUsedUnlockableOutfitCount++; }

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void ResetOutfitPrefs();
    void RefreshPlaylists();

    DataNode OnMsg(const SingleItemEnumCompleteMsg &);

    SongStatusMgr *mScores; // 0x18
    std::vector<Symbol> mUnlockedContent; // 0x1c
    std::vector<Symbol> mNewContent; // 0x28
    AccomplishmentProgress mAccomplishmentProgress; // 0x34
    CampaignProgress mCampaignProgress[kNumDifficulties]; // 0x158
    MetagameStats *mStats; // 0x208
    MetagameRank *mRank; // 0x20c
    MoveRatingHistory *mMoveRatingHistory; // 0x210
    std::vector<CharacterPref> mCharPrefs; // 0x214
    CustomPlaylist mPlaylists[5]; // 0x220
    bool mNeedToRefreshPlaylists; // 0x2fc - not in pdb
    bool mInFitnessMode; // 0x2fd
    float mFitnessLbs; // 0x300
    bool mIsFitnessWeightEntered; // 0x304
    float mFitnessSessionSecs; // 0x308
    float mFitnessSessionCals; // 0x30c
    float mFitnessSongCals; // 0x310
    int mUploadFriendsToken; // 0x314
    OnlineID *mOnlineID; // 0x318
    bool mHasOnlineID; // 0x31c
    ExportCheckState mExportChecked; // 0x320
    int mChallengeTimeStamp; // 0x324 - challenge timestamp?
    int mSkippedSongCount; // 0x328
    int mUsedUnlockableOutfitCount; // 0x32c
    int mLastBestScore; // 0x330
    bool mLastModePractice; // 0x334
    Symbol mLastPracticeSong; // 0x338 - song? shortname?
    bool mHasValidFitnessGoal; // 0x33c
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartDay; // 0x340
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartMonth; // 0x344
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartYear; // 0x348
    /** The number of days active this profile set as a fitness goal. */
    int mFitnessGoalTargetDays; // 0x34c
    /** The number of calories burnt this profile set as a fitness goal. */
    int mFitnessGoalTargetCalories; // 0x350
    /** The actual number of days this profile has spent active. */
    int mFitnessGoalDaysActive; // 0x354
    /** The actual number of calories this profile has burnt. */
    int mFitnessGoalCaloriesBurned; // 0x358
    int mFitnessGoalLastDayNumber; // 0x35c
    bool mFitnessGoalNeedUpload; // 0x360
    int mLastNewSong; // 0x364
    int mBootsSinceLastNag; // 0x368
    bool mFirstTimeNag; // 0x36c
    int mNagType; // 0x370
    int mAllowedNagTypes; // 0x374
};
