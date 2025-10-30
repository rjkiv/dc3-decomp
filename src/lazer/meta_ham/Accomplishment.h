#pragma once

#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "ui/UILabel.h"
#include "utl/Symbol.h"
class Accomplishment {
public:
    virtual ~Accomplishment();
    virtual bool ShowBestAfterEarn() const;
    virtual void UpdateIncrementalEntryName(UILabel *, Symbol);
    virtual bool CanBeLaunched() const;

    Accomplishment(DataArray *, int);
    Symbol GetCategory() const;
    bool HasGamerpicReward() const;
    bool HasAvatarAssetReward() const;
    Symbol GetAward() const;
    bool HasAward() const;
    bool IsSecondaryGoal() const;
    bool IsDynamic() const;
    char const *GetIconArt() const;
    Symbol GetName();
    Difficulty GetRequiredDifficulty();

    Symbol mAccomplishment_Name; // 0x4
    std::vector<Symbol> m_vSecret_Prereqs; // 0x8
    int mAccomplishment_Type; // 0x14
    Symbol mCategory; // 0x18
    Symbol mAward; // 0x1c
    Symbol mUnits_Token; // 0x20
    int mDifficulty; // 0x24
    Symbol mPassive_Msg_Channel; // 0x28
    int mPassive_Msg_Priority; // 0x2c
    bool mRequires_Unison_Ability; // 0x30
    int mPlayerCount_Min; // 0x34
    int mPlayerCount_Max; // 0x38
    int mNumSongs; // 0x3c
    std::vector<Symbol> m_vDynamic_Prereq_Songs; // 0x40
    int mProgressStep; // 0x4c
    int mGamerpic_Reward; // 0x50
    int mAvatarAsset_Reward; // 0x54
    bool mShowBestAfterEarn; // 0x58
    bool mHideProgress; // 0x59
    int unk5c;
    int mLastID; // 0x60
    bool mEarnedNoFail; // 0x64
    bool mLeaderboard; // 0x65
    bool isSecondaryGoal; // 0x66
    bool mGiveToAll; // 0x67

private:
    void Configure(DataArray *);
};
