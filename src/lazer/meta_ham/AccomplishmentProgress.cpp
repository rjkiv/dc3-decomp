#include "meta_ham/AccomplishmentProgress.h"
#include "Accomplishment.h"
#include "HamProfile.h"
#include "hamobj/HamGameData.h"
#include "hamobj/ScoreUtl.h"
#include "macros.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/AccomplishmentCategory.h"
#include "meta_ham/AccomplishmentGroup.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Award.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>

#pragma region GamerAwardStatus

GamerAwardStatus::GamerAwardStatus()
    : mID(-1), mType(kGamerAwardInvalid), mWaiting(false) {
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
    mSaveSizeMethod = SaveSize;
}

GamerAwardStatus::GamerAwardStatus(int i, GamerAwardType type)
    : mID(i), mType(type), mWaiting(false) {
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
    mSaveSizeMethod = SaveSize;
}

GamerAwardStatus::~GamerAwardStatus() {
    if (mOverlapped.InternalLow == 0x3e5) {
        XCancelOverlapped(&mOverlapped);
    }
}

void GamerAwardStatus::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << mID;
    fs << mType;
}

void GamerAwardStatus::LoadFixed(FixedSizeSaveableStream &fs, int) {
    fs >> mID;
    int type = 0;
    fs >> type;
    mType = (GamerAwardType)type;
}

int GamerAwardStatus::SaveSize(int) { REPORT_SIZE("GamerpicAward", 8); }

#pragma endregion
#pragma region AccomplishmentProgress

AccomplishmentProgress::AccomplishmentProgress(HamProfile *prof) : mParentProfile(prof) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

AccomplishmentProgress::~AccomplishmentProgress() {
    FOREACH (it, mGamerAwardStatusList) {
        delete *it;
    }
    mGamerAwardStatusList.clear();
}

BEGIN_HANDLERS(AccomplishmentProgress)
    HANDLE_EXPR(
        get_icon_hardcore_status,
        TheAccomplishmentMgr->GetIconHardCoreStatus(mAccomplishments.size())
    )
    HANDLE_ACTION(set_freestyle_photo_count, mFreestylePictureCount = _msg->Int(2))
    HANDLE_EXPR(get_freestyle_photo_count, mFreestylePictureCount)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void AccomplishmentProgress::SaveFixed(FixedSizeSaveableStream &fs) const {
    FixedSizeSaveable::SaveStd(fs, mAccomplishments, 200);
    FixedSizeSaveable::SaveStd(fs, mNewlyAcquiredAccomplishments, 200);
    FixedSizeSaveable::SaveStd(fs, mAwards, 100);
    FixedSizeSaveable::SaveStd(fs, mAccomplishmentCountTracking, 200, 8);
    fs << mTotalSongsPlayed;
    fs << mTotalCampaignSongsPlayed;
    fs << mFreestylePictureCount;
    fs << mRandomCharacterCount;
    fs << mFlawlessMoveCount;
    fs << mNiceMoveCount;
    fs << mPlayDaysCount;
    fs << mLastPlayDate;
    fs << mPlayWeekendsCount;
    fs << mLastWeekendPlayDate;
    FixedSizeSaveable::SaveStdPtr(
        fs, mGamerAwardStatusList, 50, GamerAwardStatus::SaveSize(0x5C)
    );
    FixedSizeSaveable::SaveStd(fs, mCharacterUseMap, 20, 8);
}

void AccomplishmentProgress::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    FixedSizeSaveable::LoadStd(fs, mAccomplishments, 200);
    FixedSizeSaveable::LoadStd(fs, mNewlyAcquiredAccomplishments, 200);
    FixedSizeSaveable::LoadStd(fs, mAwards, 100);
    FixedSizeSaveable::LoadStd(fs, mAccomplishmentCountTracking, 200, 8);
    fs >> mTotalSongsPlayed;
    fs >> mTotalCampaignSongsPlayed;
    fs >> mFreestylePictureCount;
    fs >> mRandomCharacterCount;
    fs >> mFlawlessMoveCount;
    fs >> mNiceMoveCount;
    fs >> mPlayDaysCount;
    fs >> mLastPlayDate;
    fs >> mPlayWeekendsCount;
    fs >> mLastWeekendPlayDate;
    FixedSizeSaveable::LoadStdPtr(
        fs, mGamerAwardStatusList, 50, GamerAwardStatus::SaveSize(i2)
    );
    FixedSizeSaveable::LoadStd(fs, mCharacterUseMap, 20, 8);
}

int AccomplishmentProgress::GetNiceMoveCount() const { return mNiceMoveCount; }
void AccomplishmentProgress::IncrementDanceBattleCount() { mDanceBattleCount++; }
void AccomplishmentProgress::ClearAllPerfectMoves() { mAllPerfectMoves = true; }
bool AccomplishmentProgress::HasNewAwards() const { return !mNewAwards.empty(); }

void AccomplishmentProgress::ClearPerfectStreak() {
    mCurrentPerfectStreak = 0;
    mBestPerfectStreak = 0;
}

void AccomplishmentProgress::SetTotalSongsPlayed(int songs) {
    mTotalSongsPlayed = songs;
    MILO_ASSERT(mParentProfile, 0x2fc);
    mParentProfile->MakeDirty();
}

void AccomplishmentProgress::SetTotalCampaignSongsPlayed(int songs) {
    mTotalCampaignSongsPlayed = songs;
    MILO_ASSERT(mParentProfile, 0x30b);
    mParentProfile->MakeDirty();
}

void AccomplishmentProgress::MovePassed(Symbol gameplayMode, int ratingIndex) {
    static Symbol move_perfect("move_perfect");
    static Symbol move_awesome("move_awesome");
    Symbol rating = RatingState(ratingIndex);
    if (rating == move_perfect) {
        mFlawlessMoveCount++;
    } else if (rating == move_awesome) {
        mNiceMoveCount++;
    }
    if (rating != move_perfect) {
        mAllPerfectMoves = false;
    }
}

int AccomplishmentProgress::GetCharacterUseCount(Symbol s) const {
    auto it = mCharacterUseMap.find(s);
    if (it == mCharacterUseMap.end()) {
        return 0;
    }
    return it->second;
}

int AccomplishmentProgress::GetCount(Symbol s) const {
    int count = 0;
    auto it = mAccomplishmentCountTracking.find(s);
    if (it != mAccomplishmentCountTracking.end()) {
        count = it->second;
    }
    return count;
}

void AccomplishmentProgress::GiveGamerpic(Accomplishment *a) {
    int reward = a->GetGamerpicReward();
    GamerAwardStatus *gStatus = new GamerAwardStatus(reward, kGamerAwardGamerpic);
    mGamerAwardStatusList.push_back(gStatus);
    DWORD res = XUserAwardGamerPicture(
        mParentProfile->GetPadNum(), reward, 0, &gStatus->mOverlapped
    );
    if (res == ERROR_IO_PENDING) {
        gStatus->mWaiting = true;
    }
}

void AccomplishmentProgress::ClearFirstNewAward() {
    MILO_ASSERT(HasNewAwards(), 0x110);
    mNewAwards.pop_front();
}

Symbol AccomplishmentProgress::GetFirstNewAward() const {
    MILO_ASSERT(HasNewAwards(), 0xfe);
    return mNewAwards.front().first;
}

Symbol AccomplishmentProgress::GetFirstNewAwardReason() const {
    MILO_ASSERT(HasNewAwards(), 0x107);
    return mNewAwards.front().second;
}

bool AccomplishmentProgress::IsAccomplished(Symbol s) const {
    return mAccomplishments.count(s);
}

void AccomplishmentProgress::Clear() {
    mAccomplishments.clear();
    mNewlyAcquiredAccomplishments.clear();
    mAccomplishmentsPendingUpdate.clear();
    mAwards.clear();
    mAccomplishmentCountTracking.clear();
    mTotalSongsPlayed = 0;
    mTotalCampaignSongsPlayed = 0;
    mDanceBattleCount = 0;
    mFreestylePictureCount = 0;
    mAllPerfectMoves = true;
    mRandomCharacterCount = 0;
    mCurrentPerfectStreak = 0;
    mBestPerfectStreak = 0;
    mCharacterUseMap.clear();
    mFlawlessMoveCount = 0;
    mNiceMoveCount = 0;
    mPlayDaysCount = 0;
    mLastPlayDate = 0;
    mPlayWeekendsCount = 0;
    mLastWeekendPlayDate = -1;
}

void AccomplishmentProgress::Poll() {
    for (auto it = mGamerAwardStatusList.begin(); it != mGamerAwardStatusList.end();) {
        GamerAwardStatus *curStatus = *it;
        if (curStatus->mWaiting) {
            DWORD dw;
            DWORD res = XGetOverlappedResult(&curStatus->mOverlapped, &dw, false);
            if (res == ERROR_SUCCESS) {
                mGamerAwardStatusList.erase(it++);
                mParentProfile->MakeDirty();
                continue;
            }
            if (res != ERROR_IO_INCOMPLETE) {
                curStatus->mWaiting = false;
            }
        }
        ++it;
    }
}

int AccomplishmentProgress::SaveSize(int i1) {
    REPORT_SIZE("AccomplishmentProgress", GamerAwardStatus::SaveSize(i1) * 0x32 + 0xEF0);
}

void AccomplishmentProgress::NotifyPlayerOfAccomplishment(Symbol s, const char *cc) {
    Accomplishment *pAccomplishment = TheAccomplishmentMgr->GetAccomplishment(s);
    MILO_ASSERT(pAccomplishment, 0x23D);
    MILO_LOG("You got accomplishment %s\n", s.Str());
}

void AccomplishmentProgress::IncrementCount(Symbol s, int i2) {
    auto it = mAccomplishmentCountTracking.find(s);
    if (it != mAccomplishmentCountTracking.end()) {
        it->second += i2;
    } else {
        mAccomplishmentCountTracking[s] = i2;
    }
}

void AccomplishmentProgress::IncrementCharacterUseCount(Symbol s) {
    auto it = mCharacterUseMap.find(s);
    if (it == mCharacterUseMap.end()) {
        mCharacterUseMap[s] = 1;
    } else {
        it->second++;
    }
}

void AccomplishmentProgress::GiveAvatarAsset(Accomplishment *acc) {
    int reward = acc->GetAvatarAssetReward();
    GamerAwardStatus *status = new GamerAwardStatus(reward, (GamerAwardType)2);
    mGamerAwardStatusList.push_back(status);
    status->mAvatarAward.dwUserIndex = mParentProfile->GetPadNum();
    status->mAvatarAward.dwAwardId = reward;
    DWORD res = XUserAwardAvatarAssets(1, &status->mAvatarAward, &status->mOverlapped);
    if (res == ERROR_IO_PENDING) {
        status->mWaiting = true;
    }
}

int AccomplishmentProgress::GetNumCompletedInCategory(Symbol s) const {
    int num = 0;
    std::set<Symbol> *set = TheAccomplishmentMgr->GetAccomplishmentSetForCategory(s);
    if (set) {
        FOREACH_PTR (it, set) {
            if (IsAccomplished(*it)) {
                num++;
            }
        }
    }
    return num;
}

int AccomplishmentProgress::GetNumCompletedInGroup(Symbol group) const {
    MILO_ASSERT(group != gNullStr, 0x2C5);
    std::list<Symbol> *pCategoryList =
        TheAccomplishmentMgr->GetCategoryListForGroup(group);
    MILO_ASSERT(pCategoryList, 0x2C8);
    int num = 0;
    FOREACH_PTR (it, pCategoryList) {
        num += GetNumCompletedInCategory(*it);
    }
    return num;
}

bool AccomplishmentProgress::AddAward(Symbol award, Symbol reason) {
    if (!HasAward(award)) {
        mAwards.insert(award);
        Award *pAward = TheAccomplishmentMgr->GetAward(award);
        MILO_ASSERT(pAward, 0x11D);
        if (!pAward->IsSilent()) {
            mNewAwards.push_back(std::make_pair(award, reason));
        }
        pAward->GrantAwards(mParentProfile);
        return true;
    } else {
        return false;
    }
}

bool AccomplishmentProgress::AddAccomplishment(Symbol s) {
    if (!IsAccomplished(s)) {
        Accomplishment *pAcc = TheAccomplishmentMgr->GetAccomplishment(s);
        if (!pAcc) {
            MILO_NOTIFY("No Accomplishment for %s", s.Str());
            return false;
        }
        NotifyPlayerOfAccomplishment(s, pAcc->GetIconArt());
        TheAccomplishmentMgr->AddGoalAcquisitionInfo(
            s, ThePlatformMgr.GetName(mParentProfile->GetPadNum()), TheGameData->GetSong()
        );
        if (pAcc->HasAward()) {
            Symbol award = pAcc->GetAward();
            AddAward(award, s);
        }
        mAccomplishments.insert(s);
        mNewlyAcquiredAccomplishments.insert(s);
        Symbol category = pAcc->GetCategory();
        AccomplishmentCategory *pCategory =
            TheAccomplishmentMgr->GetAccomplishmentCategory(category);
        MILO_ASSERT(pCategory, 0xBB);
        if (TheAccomplishmentMgr->IsCategoryComplete(mParentProfile, category)
            && pCategory->HasAward()) {
            AddAward(pCategory->GetAward(), category);
        }
        Symbol group = pCategory->GetGroup();
        AccomplishmentGroup *pGroup = TheAccomplishmentMgr->GetAccomplishmentGroup(group);
        MILO_ASSERT(pGroup, 0xCA);
        if (TheAccomplishmentMgr->IsGroupComplete(mParentProfile, group)
            && pGroup->HasAward()) {
            AddAward(pGroup->GetAward(), group);
        }
        if (pAcc->HasGamerpicReward()) {
            GiveGamerpic(pAcc);
        }
        if (pAcc->HasAvatarAssetReward()) {
            GiveAvatarAsset(pAcc);
        }
        MILO_ASSERT(mParentProfile, 0xE4);
        mParentProfile->MakeDirty();
        return true;
    } else {
        return false;
    }
}

#pragma endregion
