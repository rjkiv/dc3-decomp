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
    bool ShouldGiveToAll() const { return m_bGiveToAll; }

private:
    void Configure(DataArray *);

    Symbol m_symName; // 0x4
    std::vector<Symbol> m_vSecretPrereqs; // 0x8
    AccomplishmentType m_eType; // 0x14
    Symbol m_symCategory; // 0x18
    Symbol m_symAward; // 0x1c
    Symbol m_symUnits; // 0x20
    Difficulty m_eLaunchableDifficulty; // 0x24
    Symbol m_symPassiveMsgChannel; // 0x28
    int m_iPassiveMsgPriority; // 0x2c
    bool m_bLaunchableRequiresUnisonAbility; // 0x30
    int m_iLaunchableMinPlayers; // 0x34
    int m_iLaunchableMaxPlayers; // 0x38
    int m_iDynamicPrereqsNumSongs; // 0x3c
    std::vector<Symbol> m_vDynamicPrereqsSongs; // 0x40
    int m_iProgressStep; // 0x4c
    int m_iGamerpicReward; // 0x50
    int m_iAvatarAssetReward; // 0x54
    bool m_bShowBestAfterEarn; // 0x58
    bool m_bHideProgress; // 0x59
    int m_iIndex; // 0x5c
    int m_iContextID; // 0x60
    bool m_bCanBeEarnedWithNoFail; // 0x64
    bool m_bTrackedInLeaderboard; // 0x65
    bool m_bIsSecondaryGoal; // 0x66
    bool m_bGiveToAll; // 0x67
};
