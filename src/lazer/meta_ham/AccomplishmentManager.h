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
    AccomplishmentManager(DataArray *);
    // Hmx::Object
    virtual ~AccomplishmentManager();
    virtual DataNode Handle(DataArray *, bool);
    // ContentMgr::Callback
    virtual void ContentDone();

    void EarnAccomplishmentForProfile(HamProfile *, Symbol, bool);
    void EarnAccomplishmentForPlayer(int, Symbol);
    void EarnAccomplishmentForAll(Symbol, bool);
    int GetNumAccomplishments() const;
    bool HasCompletedAccomplishment(HamUser *, Symbol) const;
    bool HasNewAwards() const;
    Symbol GetNameForFirstNewAward(HamProfile *) const;
    Symbol GetDescriptionForFirstNewAward(HamProfile *) const;
    String GetArtForFirstNewAward(HamProfile *) const;
    bool HasArtForFirstNewAward(HamProfile *) const;
    HamProfile *GetProfileForFirstNewAward() const;
    void ClearFirstNewAward(HamProfile *);
    void UpdateReasonLabelForFirstNewAward(HamProfile *, UILabel *);
    void ClearGoalProgressionAcquisitionInfo();
    bool IsUnlockableAsset(Symbol) const;
    bool IsGroupComplete(HamProfile *, Symbol) const;
    bool IsCategoryComplete(HamProfile *, Symbol) const;
    int GetNumCompletedAccomplishments(HamUser *) const;
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
    std::set<Symbol> *GetAccomplishmentSetForCategory(Symbol) const;
    std::list<Symbol> *GetCategoryListForGroup(Symbol) const;
    Symbol GetAssetSource(Symbol) const;
    Award *GetAward(Symbol) const;
    void EarnAwardForAll(Symbol, bool);
    void EarnAwardForProfile(HamProfile *, Symbol);
    Symbol GetReasonForFirstNewAward(HamProfile *) const;
    void Poll();
    AccomplishmentCategory *GetAccomplishmentCategory(Symbol) const;
    AccomplishmentGroup *GetAccomplishmentGroup(Symbol) const;
    bool IsAvailable(Symbol) const;
    int GetNumAccomplishmentsInCategory(Symbol) const;
    int GetNumAccomplishmentsInGroup(Symbol) const;
    void CheckForCampaignAccomplishmentsForProfile(HamProfile *);
    void CheckForOneShotAccomplishments(Symbol, HamPlayerData *, HamProfile *);
    void CheckForCharacterListAccomplishments(Symbol, HamPlayerData *, HamProfile *);
    void UpdateMiscellaneousSongDataForUser(Symbol, HamPlayerData *, HamProfile *);
    void CheckForSpecificModesAccomplishments(Symbol, HamPlayerData *, HamProfile *);
    void CheckForCrewsAccomplishments(HamProfile *);
    HardCoreStatus GetIconHardCoreStatus(int) const;
    void HandleSongCompleted(Symbol);
    const std::vector<Symbol> &GetDiscSongs() const { return mDiscSongs; }

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
    void HandleSongCompletedForProfile(Symbol, HamPlayerData *, HamProfile *);

    DataNode OnMsg(const SigninChangedMsg &);

    bool unk30[2]; // 0x30
    std::map<Symbol, Accomplishment *> mAccomplishments; // 0x34
    std::map<Symbol, AccomplishmentCategory *> mAccomplishmentCategories; // 0x4c
    std::map<Symbol, AccomplishmentGroup *> mAccomplishmentGroups; // 0x64
    std::map<Symbol, Award *> mAwards; // 0x7c
    std::map<Symbol, Symbol> mAssetToAward; // 0x94
    std::map<Symbol, Symbol> mAwardToSource; // 0xac
    std::map<Symbol, std::list<Symbol> *> m_mapGroupToCategories; // 0xc4
    std::map<Symbol, std::set<Symbol> *> m_mapCategoryToAccomplishmentSet; // 0xdc
    int mLeaderboardThresholds[4]; // 0xf4
    int mIconThresholds[4]; // 0x104
    std::vector<GoalAcquisitionInfo> mGoalAcquisitionInfos; // 0x114
    std::vector<GoalProgressionInfo> mGoalProgressionInfos; // 0x120
    std::vector<Symbol> mDiscSongs; // 0x12c
};

extern AccomplishmentManager *TheAccomplishmentMgr;
