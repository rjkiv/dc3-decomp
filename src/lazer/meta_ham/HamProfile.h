#pragma once
#include "game/HamUser.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
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

struct CharacterPref {
    bool operator==(Symbol s) const { return mChar == s; }

    Symbol mChar; // 0x0
    Symbol mOutfit; // 0x4
    int mVoicemailIdx; // 0x8
};

class HamProfile : public Profile {
public:
    enum {
        kMaxPlaylists = 5
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

    void IncrementSkippedSongCount() { mSkippedSongCount++; }
    void UpdateNag() { unk368++; }
    MoveRatingHistory *GetMoveRatingHistory() const { return mRatingHistory; }
    bool InFitnessMode() { return mInFitnessMode; }
    MetagameRank *GetMetagameRank() const { return mRank; }
    MetagameStats *GetMetagameStats() const { return mStats; }
    OnlineID *GetOnlineID() { return mOnlineID; }
    bool IsSignedIn() const { return mSignedIn; }

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void ResetOutfitPrefs();
    void RefreshPlaylists();

    DataNode OnMsg(const SingleItemEnumCompleteMsg &);

    SongStatusMgr *mSongStatusMgr; // 0x18
    std::vector<Symbol> mUnlockedContent; // 0x1c
    std::vector<Symbol> mNewContent; // 0x28
    AccomplishmentProgress mAccProgress; // 0x34
    CampaignProgress mCampaignProgress[kNumDifficulties]; // 0x158
    MetagameStats *mStats; // 0x208
    MetagameRank *mRank; // 0x20c
    MoveRatingHistory *mRatingHistory; // 0x210
    std::vector<CharacterPref> mCharPrefs; // 0x214
    CustomPlaylist mPlaylists[5]; // 0x220
    bool unk2fc; // 0x2fc - needs refresh?
    bool mInFitnessMode; // 0x2fd
    float mFitnessPounds; // 0x300
    bool mIsFitnessWeightEntered; // 0x304
    float mFitnessTime; // 0x308 - lifetime time?
    float mFitnessCalories; // 0x30c - lifetime calories?
    float unk310; // 0x310 - calories this current session?
    int mUploadFriendsToken; // 0x314
    OnlineID *mOnlineID; // 0x318
    bool mSignedIn; // 0x31c
    int unk320;
    int unk324; // 0x324 - challenge timestamp?
    int mSkippedSongCount; // 0x328
    int unk32c;
    int unk330;
    bool unk334;
    Symbol unk338;
    bool mIsFitnessGoalSet; // 0x33c
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartDay; // 0x340
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartMonth; // 0x344
    /** The day/month/year from which this fitness goal started. */
    int mFitnessGoalStartYear; // 0x348
    /** The number of days active this profile set as a fitness goal. */
    int mFitnessGoalDaysActive; // 0x34c
    /** The number of calories burnt this profile set as a fitness goal. */
    int mFitnessGoalCalories; // 0x350
    /** The actual number of days this profile has spent active. */
    int mTrackedDaysActive; // 0x354
    /** The actual number of calories this profile has burnt. */
    int mTrackedCalories; // 0x358
    int unk35c;
    bool unk360;
    int unk364;
    int unk368;
    bool unk36c;
    int unk370; // 0x370 - nag index?
    int unk374; // 0x374 - nag mask?
};
