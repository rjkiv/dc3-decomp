#pragma once
#include "hamobj/HamPlayerData.h"
#include "meta_ham/Accomplishment.h"
#include "meta_ham/AccomplishmentCategory.h"
#include "meta_ham/AccomplishmentGroup.h"
#include "meta_ham/Award.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/PlatformMgr.h"
#include "utl/Symbol.h"

enum HardCoreStatus {
    kHardCoreStatusNormal = 0x0000,
    kHardCoreStatusSilver = 0x0001,
    kHardCoreStatusGold = 0x0002,
    kHardCoreStatusPsychedelic = 0x0003,
    kHardCoreStatusNumLevels = 0x0004
};

struct GoalAcquisitionInfo {
    Symbol m_symGoal; // 0x0
    String m_strName; // 0x4
    Symbol m_symSong; // 0xc
};

struct GoalProgressionInfo {
    Symbol m_symGoal; // 0x0
    String m_strName; // 0x4
    Symbol m_symSong; // 0xc
    int m_iAmountNeeded; // 0x10
};

class AccomplishmentManager : public Hmx::Object, public ContentMgr::Callback {
public:
    AccomplishmentManager(DataArray *cfg);
    // Hmx::Object
    virtual ~AccomplishmentManager();
    virtual DataNode Handle(DataArray *, bool);
    // ContentMgr::Callback
    virtual void ContentDone();

    void EarnAccomplishmentForProfile(HamProfile *profile, Symbol accSym, bool);
    void EarnAccomplishmentForPlayer(int i_iPlayerIndex, Symbol accSym);
    void EarnAccomplishmentForAll(Symbol accSym, bool);
    int GetNumAccomplishments() const;
    bool HasCompletedAccomplishment(HamUser *user, Symbol) const;
    bool HasNewAwards() const;
    Symbol GetNameForFirstNewAward(HamProfile *i_pProfile) const;
    Symbol GetDescriptionForFirstNewAward(HamProfile *i_pProfile) const;
    String GetArtForFirstNewAward(HamProfile *i_pProfile) const;
    bool HasArtForFirstNewAward(HamProfile *i_pProfile) const;
    HamProfile *GetProfileForFirstNewAward() const;
    void ClearFirstNewAward(HamProfile *i_pProfile);
    void UpdateReasonLabelForFirstNewAward(HamProfile *i_pProfile, UILabel *i_pLabel);
    void ClearGoalProgressionAcquisitionInfo();
    bool IsUnlockableAsset(Symbol) const;
    bool IsGroupComplete(HamProfile *i_pProfile, Symbol group) const;
    bool IsCategoryComplete(HamProfile *i_pProfile, Symbol category) const;
    int GetNumCompletedAccomplishments(HamUser *user) const;
    void AddGoalAcquisitionInfo(Symbol, const char *, Symbol);
    void AddAssetAward(Symbol, Symbol);
    Symbol GetAssetAward(Symbol) const;
    void AddAwardSource(Symbol, Symbol);
    Accomplishment *GetAccomplishment(Symbol) const;
    Symbol GetAwardSource(Symbol) const;
    bool HasAccomplishment(Symbol) const;
    bool HasAccomplishmentCategory(Symbol) const;
    bool HasAccomplishmentGroup(Symbol) const;
    bool HasAward(Symbol) const;
    std::set<Symbol> *GetAccomplishmentSetForCategory(Symbol category) const;
    std::list<Symbol> *GetCategoryListForGroup(Symbol group) const;
    Symbol GetAssetSource(Symbol) const;
    Award *GetAward(Symbol) const;
    void EarnAwardForAll(Symbol, bool);
    void EarnAwardForProfile(HamProfile *, Symbol);
    Symbol GetReasonForFirstNewAward(HamProfile *i_pProfile) const;
    void Poll();
    AccomplishmentCategory *GetAccomplishmentCategory(Symbol) const;
    AccomplishmentGroup *GetAccomplishmentGroup(Symbol) const;
    bool IsAvailable(Symbol) const;
    int GetNumAccomplishmentsInCategory(Symbol category) const;
    int GetNumAccomplishmentsInGroup(Symbol i_symGroup) const;
    void CheckForCampaignAccomplishmentsForProfile(HamProfile *profile);
    void CheckForOneShotAccomplishments(
        Symbol song, HamPlayerData *i_pPlayerData, HamProfile *profile
    );
    void CheckForCharacterListAccomplishments(
        Symbol song, HamPlayerData *i_pPlayerData, HamProfile *profile
    );
    void UpdateMiscellaneousSongDataForUser(
        Symbol song, HamPlayerData *playerData, HamProfile *profile
    );
    void CheckForSpecificModesAccomplishments(
        Symbol song, HamPlayerData *playerData, HamProfile *profile
    );
    void CheckForCrewsAccomplishments(HamProfile *profile);
    HardCoreStatus GetIconHardCoreStatus(int) const;
    void HandleSongCompleted(Symbol song);
    const std::vector<Symbol> &GetDiscSongs() { return m_vDiscSongs; }
    bool GetProfileChanged(int i) const { return mProfileChanged[i]; }
    void SetProfileChanged(int i, bool b) { mProfileChanged[i] = b; }

    static void Init(DataArray *);

private:
    void InitializeDiscSongs();
    void UpdateConsecutiveDaysPlayed(HamProfile *);
    void UpdateWeekendWarrior(HamProfile *);

protected:
    void Cleanup();
    void ConfigureAccomplishmentGroupData(DataArray *);
    void ConfigureAccomplishmentCategoryData(DataArray *);
    void ConfigureAccomplishmentData(DataArray *);
    void ConfigureAwardData(DataArray *);
    void ConfigureAccomplishmentCategoryGroupingData();
    void ConfigureAccomplishmentGroupToCategoriesData();
    void ConfigureAccomplishmentRewardData(DataArray *);
    Accomplishment *FactoryCreateAccomplishment(DataArray *, int);
    void HandleSongCompletedForProfile(
        Symbol song, HamPlayerData *playerData, HamProfile *profile
    );

    DataNode OnMsg(const SigninChangedMsg &);

private:
    bool mProfileChanged[2]; // 0x30
    std::map<Symbol, Accomplishment *> m_mapAccomplishments; // 0x34
    std::map<Symbol, AccomplishmentCategory *> m_mapAccomplishmentCategories; // 0x4c
    std::map<Symbol, AccomplishmentGroup *> m_mapAccomplishmentGroups; // 0x64
    std::map<Symbol, Award *> m_mapAwards; // 0x7c
    std::map<Symbol, Symbol> m_mapAssetToAward; // 0x94
    std::map<Symbol, Symbol> m_mapAwardToSource; // 0xac
    // key = group symbol, value = category list for group
    std::map<Symbol, std::list<Symbol> *> m_mapGroupToCategories; // 0xc4
    // key = category symbol, value = accomplishment set for category
    std::map<Symbol, std::set<Symbol> *> m_mapCategoryToAccomplishmentSet; // 0xdc
    int m_aAccomplishmentRewardLeaderboardThresholds[4]; // 0xf4
    int m_aAccomplishmentRewardIconThresholds[4]; // 0x104
    std::vector<GoalAcquisitionInfo> m_vGoalAcquisitionInfos; // 0x114
    std::vector<GoalProgressionInfo> m_vGoalProgressionInfos; // 0x120
    std::vector<Symbol> m_vDiscSongs; // 0x12c
};

extern AccomplishmentManager *TheAccomplishmentMgr;
