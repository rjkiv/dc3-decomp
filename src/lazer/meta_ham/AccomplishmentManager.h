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
};

struct GoalAcquisitionInfo {
    Symbol unk0;
    String unk4;
    Symbol unkc;
};

struct GoalProgressionInfo {
    int unk0;
    String unk4;
    int unkc;
    int unk10;
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
    const std::vector<Symbol> &GetDiscSongs() const { return mDiscSongs; }
    bool Unk30(int i) const { return unk30[i]; }
    void SetUnk30(int i, bool b) { unk30[i] = b; }

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

    bool unk30[2]; // 0x30 - players are not signed in?
    std::map<Symbol, Accomplishment *> mAccomplishments; // 0x34
    std::map<Symbol, AccomplishmentCategory *> mAccomplishmentCategories; // 0x4c
    std::map<Symbol, AccomplishmentGroup *> mAccomplishmentGroups; // 0x64
    std::map<Symbol, Award *> mAwards; // 0x7c
    std::map<Symbol, Symbol> mAssetToAward; // 0x94
    std::map<Symbol, Symbol> mAwardToSource; // 0xac
    // key = group symbol, value = category list for group
    std::map<Symbol, std::list<Symbol> *> m_mapGroupToCategories; // 0xc4
    // key = category symbol, value = accomplishment set for category
    std::map<Symbol, std::set<Symbol> *> m_mapCategoryToAccomplishmentSet; // 0xdc
    int mLeaderboardThresholds[4]; // 0xf4
    int mIconThresholds[4]; // 0x104
    std::vector<GoalAcquisitionInfo> mGoalAcquisitionInfos; // 0x114
    std::vector<GoalProgressionInfo> mGoalProgressionInfos; // 0x120
    std::vector<Symbol> mDiscSongs; // 0x12c
};

extern AccomplishmentManager *TheAccomplishmentMgr;
