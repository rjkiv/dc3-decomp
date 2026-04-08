#pragma once
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "ui/UILabel.h"
#include "utl/Symbol.h"

class HamProfile;

enum AccomplishmentType {
    kAccomplishmentTypeUnique = 0,
    kAccomplishmentTypeSongListConditional = 1,
    kAccomplishmentTypeCountConditional = 2,
    kAccomplishmentTypeOneShot = 3,
    kAccomplishmentTypeCharacterListConditional = 4,
    kAccomplishmentTypeDiscSongConditional = 5,
    kAccomplishmentTypeCampaignConditional = 6,
};

class Accomplishment {
public:
    Accomplishment(DataArray *, int);
    virtual ~Accomplishment();
    virtual AccomplishmentType GetType() const { return kAccomplishmentTypeUnique; }
    virtual bool ShowBestAfterEarn() const;
    virtual void UpdateIncrementalEntryName(UILabel *, Symbol) {
        MILO_ASSERT(false, 0x4c);
    }
    virtual bool IsFulfilled(HamProfile *) const { return false; }
    virtual bool IsRelevantForSong(Symbol) const { return false; }
    virtual Difficulty GetRequiredDifficulty() const;
    virtual bool InqProgressValues(HamProfile *, int &, int &) { return false; }
    virtual bool InqIncrementalSymbols(HamProfile *, std::vector<Symbol> &) const {
        return false;
    }
    virtual bool IsSymbolEntryFulfilled(HamProfile *, Symbol) const { return false; }
    virtual Symbol GetFirstUnfinishedAccomplishmentEntry(HamProfile *) const {
        return gNullStr;
    }
    virtual bool CanBeLaunched() const;
    virtual bool HasSpecificSongsToLaunch() const { return false; }

    Symbol GetCategory() const;
    bool HasGamerpicReward() const;
    bool HasAvatarAssetReward() const;
    Symbol GetAward() const;
    bool HasAward() const;
    bool IsSecondaryGoal() const;
    bool IsDynamic() const;
    char const *GetIconArt() const;
    Symbol GetName() const;
    const std::vector<Symbol> &GetDynamicPrereqsSongs() const;
    int GetDynamicPrereqsNumSongs() const;
    int GetAvatarAssetReward() const;
    int GetContextID() const;
    int GetGamerpicReward() const;
    bool GiveToAll() const { return mGiveToAll; }

protected:
    Symbol mName; // 0x4
    std::vector<Symbol> mSecretPrereqs; // 0x8
    AccomplishmentType mAccomplishmentType; // 0x14
    Symbol mCategory; // 0x18
    Symbol mAward; // 0x1c
    Symbol mUnitsToken; // 0x20
    Difficulty mDifficulty; // 0x24
    Symbol mPassiveMsgChannel; // 0x28
    int mPassiveMsgPriority; // 0x2c
    bool mRequiresUnisonAbility; // 0x30
    int mPlayerCountMin; // 0x34
    int mPlayerCountMax; // 0x38
    int mNumSongs; // 0x3c
    std::vector<Symbol> mDynamicPrereqsSongs; // 0x40
    int mProgressStep; // 0x4c
    int mGamerpicReward; // 0x50
    int mAvatarAssetReward; // 0x54
    bool mShowBestAfterEarn; // 0x58
    bool mHideProgress; // 0x59
    int mIndex; // 0x5c
    int mContextID; // 0x60
    bool mEarnedNoFail; // 0x64
    bool mLeaderboard; // 0x65
    bool mIsSecondaryGoal; // 0x66
    bool mGiveToAll; // 0x67

private:
    void Configure(DataArray *);
};
