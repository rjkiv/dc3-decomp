#include "meta_ham/AccomplishmentManager.h"
#include "AccomplishmentOneShot.h"
#include "game/GameMode.h"
#include "game/HamUser.h"
#include "game/HamUserMgr.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta/Achievements.h"
#include "meta_ham/Accomplishment.h"
#include "meta_ham/AccomplishmentCampaignConditional.h"
#include "meta_ham/AccomplishmentCategory.h"
#include "meta_ham/AccomplishmentCharacterListConditional.h"
#include "meta_ham/AccomplishmentCountConditional.h"
#include "meta_ham/AccomplishmentDiscSongConditional.h"
#include "meta_ham/AccomplishmentGroup.h"
#include "meta_ham/AccomplishmentOneShot.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/AccomplishmentSongConditional.h"
#include "meta_ham/AccomplishmentSongListConditional.h"
#include "meta_ham/Award.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UILabel.h"
#include "utl/Symbol.h"

AccomplishmentManager *TheAccomplishmentMgr;

AccomplishmentManager::AccomplishmentManager(DataArray *cfg) {
    MILO_ASSERT(!TheAccomplishmentMgr, 0x2B);
    TheAccomplishmentMgr = this;
    SetName("acc_mgr", ObjectDir::Main());
    ConfigureAccomplishmentGroupData(cfg->FindArray("accomplishment_groups"));
    ConfigureAccomplishmentCategoryData(cfg->FindArray("accomplishment_categories"));
    ConfigureAccomplishmentData(cfg->FindArray("accomplishments"));
    ConfigureAwardData(cfg->FindArray("awards"));
    ConfigureAccomplishmentCategoryGroupingData();
    ConfigureAccomplishmentGroupToCategoriesData();
    ConfigureAccomplishmentRewardData(cfg->FindArray("accomplishment_rewards"));
    for (int i = 0; i < 2; i++) {
        unk30[i] = false;
    }
    TheContentMgr.RegisterCallback(this, false);
}

AccomplishmentManager::~AccomplishmentManager() { Cleanup(); }

BEGIN_HANDLERS(AccomplishmentManager)
    HANDLE_ACTION(
        earn_accomplishment,
        EarnAccomplishmentForProfile(_msg->Obj<HamProfile>(2), _msg->Sym(3), true)
    )
    HANDLE_ACTION(
        earn_accomplishment_for_player,
        EarnAccomplishmentForPlayer(_msg->Int(2), _msg->Sym(3))
    )
    HANDLE_ACTION(
        earn_accomplishment_for_all, EarnAccomplishmentForAll(_msg->Sym(2), _msg->Int(3))
    )
    HANDLE_EXPR(get_num_goals, GetNumAccomplishments())
    HANDLE_EXPR(
        get_num_completed_goals, GetNumCompletedAccomplishments(_msg->Obj<HamUser>(2))
    )
    HANDLE_EXPR(
        has_completed_goal,
        HasCompletedAccomplishment(_msg->Obj<HamUser>(2), _msg->Sym(3))
    )
    HANDLE_EXPR(has_new_awards, HasNewAwards())
    HANDLE_EXPR(
        get_name_for_first_new_award, GetNameForFirstNewAward(_msg->Obj<HamProfile>(2))
    )
    HANDLE_EXPR(
        get_description_for_first_new_award,
        GetDescriptionForFirstNewAward(_msg->Obj<HamProfile>(2))
    )
    HANDLE_EXPR(
        get_art_for_first_new_award, GetArtForFirstNewAward(_msg->Obj<HamProfile>(2))
    )
    HANDLE_EXPR(
        has_art_for_first_new_award, HasArtForFirstNewAward(_msg->Obj<HamProfile>(2))
    )
    HANDLE_EXPR(get_profile_for_first_new_award, GetProfileForFirstNewAward())
    HANDLE_ACTION(clear_first_new_award, ClearFirstNewAward(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(
        update_reason_for_first_new_award,
        UpdateReasonLabelForFirstNewAward(_msg->Obj<HamProfile>(2), _msg->Obj<UILabel>(3))
    )
    HANDLE_ACTION(clear_goal_info, ClearGoalProgressionAcquisitionInfo())
    HANDLE_EXPR(is_unlockable_asset, IsUnlockableAsset(_msg->Sym(2)))
    HANDLE_EXPR(is_group_complete, IsGroupComplete(_msg->Obj<HamProfile>(2), _msg->Sym(3)))
    HANDLE_EXPR(
        is_category_complete, IsCategoryComplete(_msg->Obj<HamProfile>(2), _msg->Sym(3))
    )
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void AccomplishmentManager::ContentDone() { InitializeDiscSongs(); }

bool AccomplishmentManager::IsUnlockableAsset(Symbol s) const {
    return mAssetToAward.find(s) != mAssetToAward.end();
}

void AccomplishmentManager::InitializeDiscSongs() {
    mDiscSongs.clear();
    std::vector<int> songs;
    TheHamSongMgr.GetRankedSongs(songs);
    FOREACH (it, songs) {
        int curSongID = *it;
        const HamSongMetadata *pSongData = TheHamSongMgr.Data(curSongID);
        MILO_ASSERT(pSongData, 0x9D);
        static Symbol ham3("ham3");
        if (pSongData->GameOrigin() == ham3) {
            Symbol shortname = TheHamSongMgr.GetShortNameFromSongID(curSongID);
            mDiscSongs.push_back(shortname);
        }
    }
}

// void AccomplishmentManager::UpdateConsecutiveDaysPlayed(HamProfile *profile) {
//     DateTime dt;
//     AccomplishmentProgress &progress = profile->AccessAccomplishmentProgress();
// }

void AccomplishmentManager::AddGoalAcquisitionInfo(Symbol s1, const char *cc, Symbol s2) {
    GoalAcquisitionInfo info;
    info.unk0 = s1;
    info.unk4 = cc;
    info.unkc = s2;
    mGoalAcquisitionInfos.push_back(info);
}

// @BUG: what is the point of this
void AccomplishmentManager::Init(DataArray *cfg) { new AccomplishmentManager(cfg); }

void AccomplishmentManager::ClearGoalProgressionAcquisitionInfo() {
    mGoalAcquisitionInfos.clear();
    mGoalProgressionInfos.clear();
}

void AccomplishmentManager::Cleanup() {
    FOREACH (it, mAccomplishments) {
        RELEASE(it->second);
    }
    mAccomplishments.clear();
    FOREACH (it, mAccomplishmentCategories) {
        RELEASE(it->second);
    }
    mAccomplishmentCategories.clear();
    FOREACH (it, m_mapCategoryToAccomplishmentSet) {
        RELEASE(it->second);
    }
    m_mapCategoryToAccomplishmentSet.clear();
    FOREACH (it, m_mapGroupToCategories) {
        RELEASE(it->second);
    }
    m_mapGroupToCategories.clear();
    FOREACH (it, mAccomplishmentGroups) {
        RELEASE(it->second);
    }
    mAccomplishmentGroups.clear();
    FOREACH (it, mAwards) {
        RELEASE(it->second);
    }
    mAwards.clear();
    mAssetToAward.clear();
    mAwardToSource.clear();
    TheContentMgr.UnregisterCallback(this, false);
    TheAccomplishmentMgr = nullptr;
}

void AccomplishmentManager::AddAssetAward(Symbol s1, Symbol s2) {
    if (GetAssetAward(s1) != gNullStr) {
        MILO_NOTIFY("Asset:%s is earned by multiple sources!", s1.Str());
    } else {
        mAssetToAward[s1] = s2;
    }
}

void AccomplishmentManager::AddAwardSource(Symbol s1, Symbol s2) {
    Accomplishment *pAcc = GetAccomplishment(s2);
    MILO_ASSERT(pAcc, 0x230);
    if (GetAwardSource(s1) != gNullStr) {
        if (!pAcc->IsSecondaryGoal()) {
            MILO_NOTIFY("Award:%s is earned by multiple sources!", s1.Str());
        }
    } else {
        if (pAcc->IsSecondaryGoal()) {
            MILO_NOTIFY(
                "Award:%s has a first source that is marked as a secondary goal!",
                s1.Str()
            );
        }
        mAwardToSource[s1] = s2;
    }
}

bool AccomplishmentManager::HasAccomplishmentCategory(Symbol s) const {
    return mAccomplishmentCategories.find(s) != mAccomplishmentCategories.end();
}

bool AccomplishmentManager::HasAccomplishmentGroup(Symbol s) const {
    return mAccomplishmentGroups.find(s) != mAccomplishmentGroups.end();
}

void AccomplishmentManager::ConfigureAccomplishmentCategoryData(DataArray *cfg) {
    for (int i = 1; i < cfg->Size(); i++) {
        AccomplishmentCategory *pAccomplishmentCategory =
            new AccomplishmentCategory(cfg->Array(i), i);
        MILO_ASSERT(pAccomplishmentCategory, 0xED);
        Symbol name = pAccomplishmentCategory->GetName();
        if (HasAccomplishmentCategory(name)) {
            MILO_NOTIFY("%s accomplishment category already exists, skipping", name.Str());
            delete pAccomplishmentCategory;
        } else {
            Symbol group = pAccomplishmentCategory->GetGroup();
            if (!HasAccomplishmentGroup(group)) {
                MILO_NOTIFY(
                    "%s accomplishment category has invalid group: %s, skipping",
                    name.Str(),
                    group.Str()
                );
                delete pAccomplishmentCategory;
            } else {
                mAccomplishmentCategories[name] = pAccomplishmentCategory;
                if (pAccomplishmentCategory->HasAward()) {
                    AddAwardSource(
                        pAccomplishmentCategory->GetAward(),
                        pAccomplishmentCategory->GetName()
                    );
                }
            }
        }
    }
}

void AccomplishmentManager::ConfigureAccomplishmentCategoryGroupingData() {
    MILO_ASSERT(m_mapCategoryToAccomplishmentSet.empty(), 0x15C);
    FOREACH (it, mAccomplishments) {
        Accomplishment *pAccomplishment = it->second;
        MILO_ASSERT(pAccomplishment, 0x164);
        Symbol category = pAccomplishment->GetCategory();
        Symbol name = pAccomplishment->GetName();
        std::set<Symbol> *set = GetAccomplishmentSetForCategory(category);
        if (!set) {
            set = new std::set<Symbol>();
            m_mapCategoryToAccomplishmentSet[category] = set;
        }
        set->insert(name);
    }
}

bool AccomplishmentManager::HasAccomplishment(Symbol s) const {
    return mAccomplishments.find(s) != mAccomplishments.end();
}

void AccomplishmentManager::ConfigureAccomplishmentData(DataArray *cfg) {
    for (int i = 1; i < cfg->Size(); i++) {
        Accomplishment *pAccomplishment = FactoryCreateAccomplishment(cfg->Array(i), i);
        MILO_ASSERT(pAccomplishment, 0x13E);
        Symbol name = pAccomplishment->GetName();
        if (HasAccomplishment(name)) {
            MILO_NOTIFY("%s accomplishment already exists, skipping", name.Str());
            delete pAccomplishment;
        } else {
            Symbol cat = pAccomplishment->GetCategory();
            if (!HasAccomplishmentCategory(cat)) {
                MILO_NOTIFY(
                    "%s accomplishment is using unknown category: %s",
                    name.Str(),
                    cat.Str()
                );
                delete pAccomplishment;
            } else {
                mAccomplishments[name] = pAccomplishment;
                if (pAccomplishment->HasAward()) {
                    AddAwardSource(
                        pAccomplishment->GetAward(), pAccomplishment->GetName()
                    );
                }
            }
        }
    }
}

void AccomplishmentManager::ConfigureAccomplishmentGroupData(DataArray *cfg) {
    for (int i = 1; i < cfg->Size(); i++) {
        AccomplishmentGroup *pAccomplishmentGroup =
            new AccomplishmentGroup(cfg->Array(i), i);
        MILO_ASSERT(pAccomplishmentGroup, 0x10F);
        Symbol name = pAccomplishmentGroup->GetName();
        if (HasAccomplishmentGroup(name)) {
            MILO_NOTIFY("%s accomplishment category already exists, skipping", name.Str());
            delete pAccomplishmentGroup;
        } else {
            mAccomplishmentGroups[name] = pAccomplishmentGroup;
            if (pAccomplishmentGroup->HasAward()) {
                AddAwardSource(
                    pAccomplishmentGroup->GetAward(), pAccomplishmentGroup->GetName()
                );
            }
        }
    }
}

void AccomplishmentManager::ConfigureAccomplishmentGroupToCategoriesData() {
    MILO_ASSERT(m_mapGroupToCategories.empty(), 0x178);
    FOREACH (it, mAccomplishmentCategories) {
        AccomplishmentCategory *pCategory = it->second;
        MILO_ASSERT(pCategory, 0x180);
        Symbol name = pCategory->GetName();
        Symbol group = pCategory->GetGroup();
        std::list<Symbol> *list = GetCategoryListForGroup(group);
        if (!list) {
            list = new std::list<Symbol>();
            m_mapGroupToCategories[group] = list;
        }
        list->push_back(name);
    }
}

void AccomplishmentManager::ConfigureAccomplishmentRewardData(DataArray *cfg) {
    DataArray *pLeaderboardThresholds = cfg->Array(1);
    for (int i = 0; i < 4; i++) {
        MILO_ASSERT(pLeaderboardThresholds->Array(i+1)->Int(0) == i, 0x197);
        mLeaderboardThresholds[i] = pLeaderboardThresholds->Array(i + 1)->Int(1);
    }
    DataArray *pIconThresholds = cfg->Array(2);
    for (int i = 0; i < 4; i++) {
        MILO_ASSERT(pIconThresholds->Array( i + 1 )->Int( 0 ) == i, 0x19E);
        mIconThresholds[i] = pIconThresholds->Array(i + 1)->Int(1);
    }
}

bool AccomplishmentManager::HasAward(Symbol s) const {
    return mAwards.find(s) != mAwards.end();
}

void AccomplishmentManager::ConfigureAwardData(DataArray *cfg) {
    for (int i = 1; i < cfg->Size(); i++) {
        Award *pAward = new Award(cfg->Array(i), i);
        MILO_ASSERT(pAward, 0x129);
        Symbol name = pAward->GetName();
        if (HasAward(name)) {
            MILO_NOTIFY("%s award already exists, skipping", name.Str());
            delete pAward;
        } else {
            mAwards[name] = pAward;
        }
    }
}

Symbol AccomplishmentManager::GetAssetAward(Symbol s) const {
    auto it = mAssetToAward.find(s);
    if (it != mAssetToAward.end())
        return it->second;
    else
        return gNullStr;
}

Symbol AccomplishmentManager::GetAssetSource(Symbol s) const {
    Symbol award = GetAssetAward(s);
    return GetAwardSource(award);
}

Award *AccomplishmentManager::GetAward(Symbol s) const {
    auto it = mAwards.find(s);
    if (it != mAwards.end())
        return it->second;
    else
        return nullptr;
}

Symbol AccomplishmentManager::GetAwardSource(Symbol s) const {
    auto it = mAwardToSource.find(s);
    if (it != mAwardToSource.end())
        return it->second;
    else
        return gNullStr;
}

void AccomplishmentManager::EarnAwardForAll(Symbol s1, bool b2) {
    if (b2) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x2D1);
            HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
            if (profile && profile->HasValidSaveData() && pPlayer->IsPlaying()) {
                EarnAwardForProfile(profile, s1);
            }
        }
    } else {
        std::vector<HamProfile *> profiles = TheProfileMgr.GetSignedInProfiles();
        FOREACH (it, profiles) {
            HamProfile *pProfile = *it;
            MILO_ASSERT(pProfile, 0x2E2);
            if (pProfile && pProfile->HasValidSaveData()) {
                EarnAwardForProfile(pProfile, s1);
            }
        }
    }
}

void AccomplishmentManager::EarnAwardForProfile(HamProfile *i_pProfile, Symbol s2) {
    MILO_ASSERT(i_pProfile, 0x2C2);
    AccomplishmentProgress &progress = i_pProfile->AccessAccomplishmentProgress();
    progress.AddAward(s2, gNullStr);
}

bool AccomplishmentManager::HasNewAwards() const {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x6A5);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (profile && profile->HasValidSaveData()
            && profile->GetAccomplishmentProgress().HasNewAwards()) {
            return true;
        }
    }
    if (TheGameMode->InMode("campaign", true)) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x6B7);
        if (pProfile->GetAccomplishmentProgress().HasNewAwards()) {
            return true;
        }
    }
    return false;
}

Symbol AccomplishmentManager::GetReasonForFirstNewAward(HamProfile *i_pProfile) const {
    MILO_ASSERT(i_pProfile, 0x6C7);
    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    Symbol out;
    if (progress.HasNewAwards()) {
        out = progress.GetFirstNewAwardReason();
    } else {
        MILO_ASSERT(false, 0x6D2);
        out = "";
    }
    return out;
}

HamProfile *AccomplishmentManager::GetProfileForFirstNewAward() const {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x74E);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (profile && profile->HasValidSaveData()
            && profile->GetAccomplishmentProgress().HasNewAwards()) {
            return profile;
        }
    }
    if (TheGameMode->InMode("campaign", true)) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x762);
        if (pProfile->GetAccomplishmentProgress().HasNewAwards()) {
            return pProfile;
        }
    }
    MILO_ASSERT(false, 0x76C);
    return nullptr;
}

void AccomplishmentManager::ClearFirstNewAward(HamProfile *i_pProfile) {
    MILO_ASSERT(i_pProfile, 0x78E);
    i_pProfile->AccessAccomplishmentProgress().ClearFirstNewAward();
}

Symbol AccomplishmentManager::GetNameForFirstNewAward(HamProfile *i_pProfile) const {
    MILO_ASSERT(i_pProfile, 0x6DC);
    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    Symbol out;
    if (progress.HasNewAwards()) {
        Symbol firstAward = progress.GetFirstNewAward();
        Award *pAward = GetAward(firstAward);
        MILO_ASSERT(pAward, 0x6E4);
        out = pAward->GetDisplayName();
    } else {
        MILO_ASSERT(false, 0x6EA);
        out = "";
    }
    return out;
}

Symbol
AccomplishmentManager::GetDescriptionForFirstNewAward(HamProfile *i_pProfile) const {
    MILO_ASSERT(i_pProfile, 0x6F4);
    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    Symbol firstAward;
    if (progress.HasNewAwards()) {
        firstAward = progress.GetFirstNewAward();
    }
    Symbol out;
    if (firstAward != "") {
        Award *pAward = GetAward(firstAward);
        MILO_ASSERT(pAward, 0x700);
        out = MakeString("%s%s%s", "award_", pAward->GetDisplayName(), "_desc");
    } else {
        MILO_ASSERT(false, 0x706);
        out = "";
    }
    return out;
}

void AccomplishmentManager::UpdateReasonLabelForFirstNewAward(
    HamProfile *i_pProfile, UILabel *i_pLabel
) {
    MILO_ASSERT(i_pProfile, 0x710);
    MILO_ASSERT(i_pLabel, 0x711);
    Symbol symReason = GetReasonForFirstNewAward(i_pProfile);
    MILO_ASSERT(symReason != "", 0x715);
    if (HasAccomplishment(symReason)) {
        static Symbol campaign_award_earned_by_goal("campaign_award_earned_by_goal");
        i_pLabel->SetTokenFmt(campaign_award_earned_by_goal, symReason);
    } else if (HasAccomplishmentCategory(symReason)) {
        static Symbol campaign_award_earned_by_category(
            "campaign_award_earned_by_category"
        );
        i_pLabel->SetTokenFmt(campaign_award_earned_by_category, symReason);
    } else if (HasAccomplishmentGroup(symReason)) {
        static Symbol campaign_award_earned_by_group("campaign_award_earned_by_group");
        i_pLabel->SetTokenFmt(campaign_award_earned_by_group, symReason);
    } else {
        MILO_ASSERT(false, 0x729);
    }
}

void AccomplishmentManager::Poll() {
    std::vector<HamProfile *> profiles = TheProfileMgr.GetSignedInProfiles();
    FOREACH (it, profiles) {
        HamProfile *pProfile = *it;
        MILO_ASSERT(pProfile, 0x88);
        pProfile->AccessAccomplishmentProgress().Poll();
    }
}

std::list<Symbol> *AccomplishmentManager::GetCategoryListForGroup(Symbol s) const {
    auto it = m_mapGroupToCategories.find(s);
    if (it != m_mapGroupToCategories.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

std::set<Symbol> *AccomplishmentManager::GetAccomplishmentSetForCategory(Symbol s) const {
    auto it = m_mapCategoryToAccomplishmentSet.find(s);
    if (it != m_mapCategoryToAccomplishmentSet.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

AccomplishmentCategory *AccomplishmentManager::GetAccomplishmentCategory(Symbol s) const {
    auto it = mAccomplishmentCategories.find(s);
    if (it != mAccomplishmentCategories.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

AccomplishmentGroup *AccomplishmentManager::GetAccomplishmentGroup(Symbol s) const {
    auto it = mAccomplishmentGroups.find(s);
    if (it != mAccomplishmentGroups.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

Accomplishment *AccomplishmentManager::GetAccomplishment(Symbol s) const {
    auto it = mAccomplishments.find(s);
    if (it != mAccomplishments.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void AccomplishmentManager::EarnAccomplishmentForProfile(
    HamProfile *profile, Symbol s2, bool b3
) {
    Accomplishment *pAcc = GetAccomplishment(s2);
    if (!pAcc) {
        MILO_NOTIFY("No accomplishment for %s", s2.Str());
    } else {
        if (b3 && pAcc->GiveToAll()) {
            EarnAccomplishmentForAll(s2, false);
        } else if (profile) {
            int padnum = profile->GetPadNum();
            if (!unk30[padnum]
                && !profile->GetAccomplishmentProgress().IsAccomplished(s2)) {
                profile->EarnAccomplishment(s2);
                int ctxID = pAcc->GetContextID();
                if (ctxID != -1) {
                    TheAchievements->Submit(padnum, s2, ctxID);
                }
            }
        } else {
            MILO_NOTIFY("No active profile for accomplishment %s", s2.Str());
        }
    }
}

void AccomplishmentManager::EarnAccomplishmentForAll(Symbol s1, bool b2) {
    if (b2) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x2F5);
            int padnum = pPlayer->PadNum();
            if (!unk30[padnum]) {
                HamProfile *profile = TheProfileMgr.GetProfileFromPad(padnum);
                if (profile && profile->HasValidSaveData() && pPlayer->IsPlaying()) {
                    if (!profile->GetAccomplishmentProgress().IsAccomplished(s1)) {
                        EarnAccomplishmentForProfile(profile, s1, false);
                    }
                }
            }
        }
    } else {
        std::vector<HamProfile *> profiles = TheProfileMgr.GetSignedInProfiles();
        FOREACH (it, profiles) {
            HamProfile *pProfile = *it;
            MILO_ASSERT(pProfile, 0x310);
            if (pProfile && pProfile->HasValidSaveData()) {
                if (!pProfile->GetAccomplishmentProgress().IsAccomplished(s1)) {
                    EarnAccomplishmentForProfile(pProfile, s1, false);
                }
            }
        }
    }
}

String AccomplishmentManager::GetArtForFirstNewAward(HamProfile *i_pProfile) const {
    MILO_ASSERT(i_pProfile, 0x730);

    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    Symbol sym;
    if (progress.HasNewAwards()) {
        sym = progress.GetFirstNewAward();
    }
    String out;
    if (sym != "") {
        Award *pAward = GetAward(sym);
        MILO_ASSERT(pAward, 0x73C);
        out = pAward->GetArt();
    } else {
        MILO_ASSERT(false, 0x742);
    }
    return out;
}

bool AccomplishmentManager::HasArtForFirstNewAward(HamProfile *i_pProfile) const {
    MILO_ASSERT(i_pProfile, 0x773);

    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    Symbol sym;
    if (progress.HasNewAwards()) {
        sym = progress.GetFirstNewAward();
    }
    bool ret = false;
    if (sym != "") {
        Award *pAward = GetAward(sym);
        MILO_ASSERT(pAward, 0x77F);
        // ret = pAward->unk18 == gNullstr;
    } else {
        MILO_ASSERT(false, 0x785);
    }
    return ret;
}

bool AccomplishmentManager::IsAvailable(Symbol s) const {
    Accomplishment *pAccomplishment = TheAccomplishmentMgr->GetAccomplishment(s);
    MILO_ASSERT(pAccomplishment, 0x7E2);
    if (!pAccomplishment->IsDynamic()) {
        return true;
    } else {
        const std::vector<Symbol> &rSongs = pAccomplishment->GetDynamicPrereqsSongs();
        int iNumSongs = rSongs.size();
        int prereqNum = pAccomplishment->GetDynamicPrereqsNumSongs();
        if (prereqNum <= 0) {
            prereqNum = iNumSongs;
        }
        MILO_ASSERT(iNumSongs <= rSongs.size(), 0x7F3);
        int thresh = 0;
        for (int i = 0; i < iNumSongs; i++) {
            if (TheHamSongMgr.HasSong(rSongs[i], false)) {
                thresh++;
            }
            if (thresh >= prereqNum) {
                return true;
            }
        }
        return false;
    }
}

int AccomplishmentManager::GetNumAccomplishmentsInCategory(Symbol s) const {
    int num = 0;
    std::set<Symbol> *set = GetAccomplishmentSetForCategory(s);
    if (set) {
        FOREACH_PTR (it, set) {
            if (IsAvailable(*it)) {
                num++;
            }
        }
    }
    return num;
}

int AccomplishmentManager::GetNumAccomplishmentsInGroup(Symbol i_symGroup) const {
    MILO_ASSERT(i_symGroup != gNullStr, 0x1D4);
    std::list<Symbol> *pCategoryList = GetCategoryListForGroup(i_symGroup);
    MILO_ASSERT(pCategoryList, 0x1D7);
    int num = 0;
    FOREACH_PTR (it, pCategoryList) {
        num += GetNumAccomplishmentsInCategory(*it);
    }
    return num;
}

void AccomplishmentManager::EarnAccomplishmentForPlayer(int i_iPlayerIndex, Symbol s) {
    MILO_ASSERT_RANGE(i_iPlayerIndex, 0, 2, 0x282);
    HamPlayerData *pPlayer = TheGameData->Player(i_iPlayerIndex);
    MILO_ASSERT(pPlayer, 0x285);
    if (!unk30[pPlayer->PadNum()]) {
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (profile) {
            EarnAccomplishmentForProfile(profile, s, true);
        }
    }
}

void AccomplishmentManager::CheckForCampaignAccomplishmentsForProfile(
    HamProfile *profile
) {
    const AccomplishmentProgress &progress = profile->GetAccomplishmentProgress();
    FOREACH (it, mAccomplishments) {
        Symbol key = it->first;
        if (!progress.IsAccomplished(key)) {
            Accomplishment *pAccomplishment = it->second;
            MILO_ASSERT(pAccomplishment, 0x3A0);
            if (pAccomplishment->GetType() == 6 && IsAvailable(key)
                && pAccomplishment->IsFulfilled(profile)) {
                EarnAccomplishmentForProfile(profile, key, true);
            }
        }
    }
}

void AccomplishmentManager::CheckForOneShotAccomplishments(
    Symbol s, HamPlayerData *i_pPlayerData, HamProfile *profile
) {
    MILO_ASSERT(i_pPlayerData, 0x45B);
    Difficulty d = i_pPlayerData->GetDifficulty();
    const AccomplishmentProgress &progress = profile->GetAccomplishmentProgress();
    FOREACH (it, mAccomplishments) {
        Symbol key = it->first;
        if (!progress.IsAccomplished(key)) {
            Accomplishment *pAccomplishment = it->second;
            MILO_ASSERT(pAccomplishment, 0x46F);
            if (IsAvailable(key) && pAccomplishment->GetType() == 3) {
                AccomplishmentOneShot *pOneShotAccomplishment =
                    dynamic_cast<AccomplishmentOneShot *>(pAccomplishment);
                MILO_ASSERT(pOneShotAccomplishment, 0x47E);
                if (pOneShotAccomplishment->AreOneShotConditionsMet(
                        i_pPlayerData, profile, s, d
                    )) {
                    EarnAccomplishmentForProfile(profile, key, true);
                }
            }
        }
    }
}

void AccomplishmentManager::CheckForCharacterListAccomplishments(
    Symbol s, HamPlayerData *i_pPlayerData, HamProfile *profile
) {
    MILO_ASSERT(i_pPlayerData, 0x48C);
    static Symbol acc_last_fashion("acc_last_fashion");
    static Symbol acc_unlockable_costume("acc_unlockable_costume");
    const AccomplishmentProgress &progress = profile->GetAccomplishmentProgress();
    FOREACH (it, mAccomplishments) {
        Symbol key = it->first;
        if (!progress.IsAccomplished(key)) {
            Accomplishment *pAccomplishment = it->second;
            MILO_ASSERT(pAccomplishment, 0x4A2);
            if (IsAvailable(key) && pAccomplishment->GetType() == 4) {
                AccomplishmentCharacterListConditional *pCharacterListAccomplishment =
                    dynamic_cast<AccomplishmentCharacterListConditional *>(
                        pAccomplishment
                    );
                MILO_ASSERT(pCharacterListAccomplishment, 0x4B1);
                if (pCharacterListAccomplishment->AreCharacterListConditionsMet(
                        s, i_pPlayerData, profile
                    )) {
                    EarnAccomplishmentForProfile(profile, key, true);
                }
                if (key == acc_last_fashion
                    && pCharacterListAccomplishment->AreOldOutfitListConditionsMet()
                    && !TheGameMode->IsGameplayModeBustamove()
                    && !TheGameMode->IsGameplayModeRhythmBattle()) {
                    EarnAccomplishmentForAll(acc_last_fashion, true);
                }
                if (key == acc_unlockable_costume
                    && pCharacterListAccomplishment->AreUnlockableOutfitListConditionsMet(
                        i_pPlayerData, profile
                    )) {
                    EarnAccomplishmentForProfile(profile, acc_unlockable_costume, true);
                }
            }
        }
    }
}

int AccomplishmentManager::GetNumAccomplishments() const {
    int num = 0;
    FOREACH (it, mAccomplishments) {
        if (IsAvailable(it->first)) {
            num++;
        }
    }
    return num;
}

bool AccomplishmentManager::IsCategoryComplete(HamProfile *i_pProfile, Symbol s) const {
    MILO_ASSERT(i_pProfile, 0x797);
    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    int num = GetNumAccomplishmentsInCategory(s);
    return num <= progress.GetNumCompletedInCategory(s);
}

bool AccomplishmentManager::IsGroupComplete(HamProfile *i_pProfile, Symbol s) const {
    MILO_ASSERT(i_pProfile, 0x7A4);
    const AccomplishmentProgress &progress = i_pProfile->GetAccomplishmentProgress();
    int num = GetNumAccomplishmentsInGroup(s);
    return num <= progress.GetNumCompletedInGroup(s);
}

void AccomplishmentManager::HandleSongCompletedForProfile(
    Symbol s, HamPlayerData *hpd, HamProfile *profile
) {
    if (TheGameMode) {
        if (!TheGameMode->Property("update_leaderboards")->Int()) {
            return;
        }
    }
    UpdateMiscellaneousSongDataForUser(s, hpd, profile);
    CheckForOneShotAccomplishments(s, hpd, profile);
    CheckForCharacterListAccomplishments(s, hpd, profile);
    CheckForSpecificModesAccomplishments(s, hpd, profile);
    CheckForCrewsAccomplishments(profile);
}

void AccomplishmentManager::UpdateMiscellaneousSongDataForUser(
    Symbol s, HamPlayerData *hpd, HamProfile *profile
) {
    MetaPerformer *pPerformer = MetaPerformer::Current();
    MILO_ASSERT(pPerformer, 0x5A3);
    AccomplishmentProgress &progress = profile->AccessAccomplishmentProgress();
    int songsPlayed = progress.GetTotalSongsPlayed();
    progress.SetTotalSongsPlayed(songsPlayed + 1);
    if (TheGameMode) {
        if (TheGameMode->InMode("campaign", true)) {
            int campaignSongsPlayed = progress.GetTotalCampaignSongsPlayed();
            progress.SetTotalCampaignSongsPlayed(campaignSongsPlayed + 1);
        }
    }
    static Symbol dance_battle("dance_battle");
    if (TheGameMode && TheGameMode->Mode() == dance_battle) {
        progress.IncrementDanceBattleCount();
    }
    progress.IncrementCharacterUseCount(hpd->Char());
    UpdateConsecutiveDaysPlayed(profile);
    UpdateWeekendWarrior(profile);
    profile->MakeDirty();
}

HardCoreStatus AccomplishmentManager::GetIconHardCoreStatus(int x) const {
    int i;
    for (i = 0; i < 4; i++) {
        if (x < mIconThresholds[i]) {
            return (HardCoreStatus)i;
        }
    }
    return (HardCoreStatus)3;
}

bool AccomplishmentManager::HasCompletedAccomplishment(HamUser *user, Symbol s) const {
    HamProfile *profile = TheProfileMgr.GetProfile(user);
    if (profile) {
        return profile->GetAccomplishmentProgress().IsAccomplished(s);
    } else {
        return false;
    }
}

int AccomplishmentManager::GetNumCompletedAccomplishments(HamUser *user) const {
    int num = 0;
    HamProfile *profile = TheProfileMgr.GetProfile(user);
    if (profile) {
        num = profile->GetAccomplishmentProgress().GetNumCompleted();
    }
    return num;
}

DataNode AccomplishmentManager::OnMsg(const SigninChangedMsg &msg) {
    if ((unsigned int)msg.GetChangedMask() && (unsigned int)msg.GetMask()) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x86D);
            int padnum = pPlayer->PadNum();
            HamProfile *profile = TheProfileMgr.GetProfileFromPad(padnum);
            if (profile) {
                if ((1 << profile->GetPadNum()) & msg.GetChangedMask()) {
                    unk30[padnum] = true;
                }
            }
        }
    }
    return 0;
}

Accomplishment *AccomplishmentManager::FactoryCreateAccomplishment(DataArray *d, int i) {
    static Symbol accomplishment_type("accomplishment_type");
    int type;
    d->FindData(accomplishment_type, type);
    Accomplishment *a = nullptr;
    switch (type) {
    case 0:
        a = new Accomplishment(d, i);
        break;
    case 1:
        a = new AccomplishmentSongListConditional(d, i);
        break;
    case 2:
        a = new AccomplishmentCountConditional(d, i);
        break;
    case 3:
        a = new AccomplishmentOneShot(d, i);
        break;
    case 4:
        a = new AccomplishmentCharacterListConditional(d, i);
        break;
    case 5:
        a = new AccomplishmentDiscSongConditional(d, i);
        break;
    case 6:
        a = new AccomplishmentCampaignConditional(d, i);
        break;
    default:
        MILO_ASSERT(false, 0xde);
        break;
    }
    return a;
}

void AccomplishmentManager::HandleSongCompleted(Symbol song) {
    if (TheHamUserMgr) {
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x41e);
            int padNum = pPlayer->PadNum();
            HamProfile *pProfile = TheProfileMgr.GetProfileFromPad(padNum);
            if (pProfile && pProfile->HasValidSaveData() && pPlayer->IsPlaying()) {
                static Symbol practice("practice");
                if (!unk30[padNum]) {
                    HandleSongCompletedForProfile(song, pPlayer, pProfile);
                }
                if (TheGameMode->InMode(practice, true)) {
                    pProfile->SetUnk388(song);
                    pProfile->SetUnk334(true);
                } else
                    pProfile->SetUnk334(false);
            }
        }
    }
}
