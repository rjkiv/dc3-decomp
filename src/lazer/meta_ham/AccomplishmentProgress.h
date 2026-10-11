#pragma once
#include "Accomplishment.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"
#include <list>
#include <utility>
#include <map>
#include <set>
#include <vector>

class HamProfile;

enum GamerAwardType {
    kGamerAwardInvalid = 0x0000,
    kGamerAwardGamerpic = 0x0001,
    kGamerAwardAvatarAsset = 0x0002
};

class GamerAwardStatus : public FixedSizeSaveable {
public:
    GamerAwardStatus();
    GamerAwardStatus(int, GamerAwardType);
    virtual ~GamerAwardStatus();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    static int SaveSize(int);

    int mID; // 0x8
    GamerAwardType mType; // 0xc
    bool mWaiting; // 0x10
    XUSER_AVATARASSET mAvatarAward; // 0x14
    XOVERLAPPED mOverlapped; // 0x1c
};

class AccomplishmentProgress : public Hmx::Object, public FixedSizeSaveable {
public:
    // Hmx::Object
    virtual ~AccomplishmentProgress();
    virtual DataNode Handle(DataArray *, bool);
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    static int SaveSize(int);

    AccomplishmentProgress(HamProfile *);
    int GetNiceMoveCount() const;
    void IncrementDanceBattleCount();
    void ClearAllPerfectMoves();
    void ClearPerfectStreak();
    bool HasNewAwards() const;
    void NotifyPlayerOfAccomplishment(Symbol, const char *);
    void SetTotalSongsPlayed(int songs);
    void SetTotalCampaignSongsPlayed(int songs);
    void MovePassed(Symbol gameplayMode, int ratingIndex);
    Symbol GetFirstNewAward() const;
    Symbol GetFirstNewAwardReason() const;
    void Poll();
    bool IsAccomplished(Symbol) const;
    void ClearFirstNewAward();
    int GetNumCompletedInCategory(Symbol) const;
    int GetNumCompletedInGroup(Symbol) const;
    int GetCharacterUseCount(Symbol) const;
    int GetCount(Symbol) const;
    bool AddAward(Symbol award, Symbol reason);
    bool AddAccomplishment(Symbol);
    void Clear();
    void IncrementCharacterUseCount(Symbol);
    void IncrementCount(Symbol, int);
    int GetTotalSongsPlayed() const;
    int GetTotalCampaignSongsPlayed() const;
    int GetNumCompleted() const;
    int GetFlawlessMoveCount() const;
    bool HasAward(Symbol award) const { return mAwards.find(award) != mAwards.end(); }
    int GetPlayDaysCount() const { return mPlayDaysCount; }
    void SetPlayDaysCount(int days) { mPlayDaysCount = days; }
    void IncrementPlayDaysCount() { mPlayDaysCount++; }
    void ClearPlayDaysCount() { mPlayDaysCount = 0; }
    int GetPlayWeekendsCount() const { return mPlayWeekendsCount; }
    int GetLastPlayDate() const { return mLastPlayDate; }
    void SetLastPlayDate(int i) { mLastPlayDate = i; }
    int GetLastWeekendPlayDate() const { return mLastWeekendPlayDate; }
    void SetWeekendPlayCount(int i) { mPlayWeekendsCount = i; }
    void SetLastWeekendPlayDate(int i) { mLastWeekendPlayDate = i; }
    const std::list<std::pair<Symbol, Symbol> > &GetNewAwards() const {
        return mNewAwards;
    }

private:
    void GiveGamerpic(Accomplishment *pAcc);
    void GiveAvatarAsset(Accomplishment *pAcc);

    std::map<Symbol, int> mStepTrackingMap; // 0x34
    HamProfile *mParentProfile; // 0x4c
    std::list<GamerAwardStatus *> mGamerAwardStatusList; // 0x50
    std::set<Symbol> mAccomplishments; // 0x58
    std::set<Symbol> mNewlyAcquiredAccomplishments; // 0x70
    std::vector<Symbol> mAccomplishmentsPendingUpdate; // 0x88
    std::set<Symbol> mAwards; // 0x94
    // award, reason
    std::list<std::pair<Symbol, Symbol> > mNewAwards; // 0xac
    int mTotalSongsPlayed; // 0xb4
    int mTotalCampaignSongsPlayed; // 0xb8
    std::map<Symbol, int> mAccomplishmentCountTracking; // 0xbc
    int mDanceBattleCount; // 0xd4
    int mFreestylePictureCount; // 0xd8
    bool mAllPerfectMoves; // 0xdc
    int mRandomCharacterCount; // 0xe0
    int mCurrentPerfectStreak; // 0xe4
    int mBestPerfectStreak; // 0xe8
    // symbol = char, int = use count
    std::map<Symbol, int> mCharacterUseMap; // 0xec
    int mFlawlessMoveCount; // 0x104
    int mNiceMoveCount; // 0x108
    Symbol mLastMove; // 0x10c
    int mSameRatingCount; // 0x110
    int mPlayDaysCount; // 0x114
    int mLastPlayDate; // 0x118
    int mPlayWeekendsCount; // 0x11c
    int mLastWeekendPlayDate; // 0x120
};
