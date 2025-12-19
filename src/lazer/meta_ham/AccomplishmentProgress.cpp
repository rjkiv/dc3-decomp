#include "meta_ham/AccomplishmentProgress.h"
#include "Accomplishment.h"
#include "HamProfile.h"
#include "hamobj/ScoreUtl.h"
#include "macros.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>
#pragma region GamerAwardStatus

GamerAwardStatus::GamerAwardStatus() : unk8(-1), unkc(type0), unk10(false) {
    memset(&unk1c, 0, sizeof(XOVERLAPPED));
    mSaveSizeMethod = SaveSize;
}

GamerAwardStatus::GamerAwardStatus(int i, GamerAwardType type)
    : unk8(i), unkc(type), unk10(false) {
    memset(&unk1c, 0, sizeof(XOVERLAPPED));
    mSaveSizeMethod = SaveSize;
}

GamerAwardStatus::~GamerAwardStatus() {
    if (unk1c.InternalLow == 0x3e5) {
        XCancelOverlapped(&unk1c);
    }
}

int GamerAwardStatus::SaveSize(int) {
    if (FixedSizeSaveable::sPrintoutsEnabled) {
        MILO_LOG("* %s = %i\n", "GamerpicAward", 8);
    }
    return 8;
}

void GamerAwardStatus::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << unk8;
    fs << unkc;
}

void GamerAwardStatus::LoadFixed(FixedSizeSaveableStream &fs, int i) { fs >> unk8; }

#pragma endregion GamerAwardStatus
#pragma region AccomplishmentProgress

AccomplishmentProgress::AccomplishmentProgress(HamProfile *prof) : mParentProfile(prof) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

AccomplishmentProgress::~AccomplishmentProgress() {
    FOREACH (it, unk50) {
        RELEASE(*it);
    }
    unk50.clear();
}

int AccomplishmentProgress::GetNiceMoveCount() const { return mNiceMoveCount; }

void AccomplishmentProgress::IncrementDanceBattleCount() { mDanceBattleCount++; }

void AccomplishmentProgress::ClearAllPerfectMoves() { mPerfectMovesCleared = true; }

void AccomplishmentProgress::ClearPerfectStreak() {
    unke4 = 0;
    unke8 = 0;
}

bool AccomplishmentProgress::HasNewAwards() const { return !unkac.empty(); }

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

void AccomplishmentProgress::MovePassed(Symbol s, int i) {
    static Symbol move_perfect("move_perfect");
    static Symbol move_awesome("move_awesome");
    RatingState(i);
    if (s == move_perfect) {
        unk104++;
    } else if (s == move_awesome) {
        mNiceMoveCount++;
    }
    if (s != move_perfect) {
        mPerfectMovesCleared = false;
    }
}

int AccomplishmentProgress::GetCharacterUseCount(Symbol s) const {
    auto it = unkec.find(s);
    if (it == unkec.end()) {
        return 0;
    }
    return it->second;
}

int AccomplishmentProgress::GetCount(Symbol s) const {
    int i = 0;
    auto it = unkbc.find(s);
    if (it != unkbc.end())
        return it->second;
}

void AccomplishmentProgress::GiveGamerpic(Accomplishment *a) {
    int i = a->GetGamerpicReward();
    GamerAwardStatus *gStatus = new GamerAwardStatus(i, type1);
    unk50.push_back(gStatus);
}

void AccomplishmentProgress::ClearFirstNewAward() {
    MILO_ASSERT(HasNewAwards(), 0x110);
    unkac.pop_front();
}

Symbol AccomplishmentProgress::GetFirstNewAward() const {
    MILO_ASSERT(HasNewAwards(), 0xfe);
    return unkac.front().first;
}

Symbol AccomplishmentProgress::GetFirstNewAwardReason() const {
    MILO_ASSERT(HasNewAwards(), 0x107);
    return unkac.front().second;
}

bool AccomplishmentProgress::IsAccomplished(Symbol s) const { return unk58.count(s); }

void AccomplishmentProgress::Clear() {
    unk58.clear();
    unk70.clear();
    unk88.clear();
    unk94.clear();
    unkbc.clear();
    mTotalSongsPlayed = 0;
    mTotalCampaignSongsPlayed = 0;
    mDanceBattleCount = 0;
    mFreestylePhotoCount = 0;
    mPerfectMovesCleared = true;
    unke0 = 0;
    unke4 = 0;
    unke8 = 0;
    unkec.clear();
    unk104 = 0;
    mNiceMoveCount = 0;
    unk114 = 0;
    unk118 = 0;
    unk11c = 0;
    unk120 = -1;
}

BEGIN_HANDLERS(AccomplishmentProgress)
    // need AccomplishmentMgr
    HANDLE_ACTION(set_freestyle_photo_count, mFreestylePhotoCount = _msg->Int(2))
    HANDLE_EXPR(get_freestyle_photo_count, mFreestylePhotoCount)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion AccomplishmentProgress
