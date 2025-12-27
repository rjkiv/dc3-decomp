#include "meta_ham/SongStatusMgr.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/HamSongMgr.h"
#include "meta/SongMgr.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

#pragma region SongStatusData

void SongStatusData::SaveToStream(BinStream &bs) const {
    bs << mScore;
    bs << mPracticeScore;
    bs << mCoopScore;
    bs << mStars;
    bs << mPercentPassed;
    bs << mNumPerfects;
    bs << mNumNices;
    bs << unk10;
    bs << mNoFlashcards;
    bs << mNeedUpload;
}

void SongStatusData::LoadFromStream(BinStream &bs) {
    bs >> mScore;
    bs >> mPracticeScore;
    bs >> mCoopScore;
    bs >> mStars;
    bs >> mPercentPassed;
    bs >> mNumPerfects;
    bs >> mNumNices;
    bs >> unk10;
    bs >> mNoFlashcards;
    bs >> mNeedUpload;
}

#pragma endregion
#pragma region FlauntStatusData

void FlauntStatusData::SaveToStream(BinStream &bs) const {
    bs << mScore;
    bs << (unsigned char)mDiff;
    bs << mNeedUpload;
}

void FlauntStatusData::LoadFromStream(BinStream &bs) {
    bs >> mScore;
    unsigned char uc;
    bs >> uc;
    mDiff = (Difficulty)uc;
    bs >> mNeedUpload;
}

#pragma endregion
#pragma region SongStatus

SongStatus::SongStatus() {
    for (int i = 0; i < 4; i++) {
        mStatusData[i].Clear();
    }
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
    Clear();
}

SongStatus::SongStatus(int songID) {
    for (int i = 0; i < 4; i++) {
        mStatusData[i].Clear();
    }
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
    Clear();
    mSongID = songID;
    for (int i = 0; i < 4; i++) {
        mStatusData[i].mSongID = mSongID;
    }
}

void SongStatus::Clear() {
    mSongID = 0;
    for (int i = 0; i < kNumDifficulties; i++) {
        mStatusData[i].Clear();
        mStatusData[i].mDifficulty = (Difficulty)i;
        mStatusData[i].mSongID = mSongID;
    }
    mLastPlayed = 0;
    unk78 = false;
    unk79 = false;
    unk7c = kNumDifficulties;
    mLastScore = 0;
    unk84 = 0;
    mLastPlayedPractice = 0;
    unk9c = 0;
    unka0 = 4;
    mBattleScore = 0;
    mTotalBattleWins = 0;
    mTotalBattleLosses = 0;
    mWonLastBattle = false;
    unka4 = 0;
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
}

const SongStatusData &SongStatus::GetBestSongStatusData() const {
    Difficulty best = kDifficultyEasy;
    for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        if (0 < mStatusData[d].mScore
            && (3 <= mStatusData[d].mStars
                || mStatusData[d].mStars >= mStatusData[best].mStars)) {
            best = d;
        }
    }
    return mStatusData[best];
}

const SongStatusData &SongStatus::GetBestPracticeSongStatusData() const {
    Difficulty best = kDifficultyEasy;
    for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        if ((50 <= mStatusData[d].mPracticeScore
             || mStatusData[d].mPracticeScore >= mStatusData[best].mPracticeScore)) {
            best = d;
        }
    }
    return mStatusData[best];
}

BinStream &operator<<(BinStream &bs, const SongStatus &stat) {
    bs << stat.mSongID;
    for (int i = 0; i < 4; i++) {
        stat.mStatusData[i].SaveToStream(bs);
    }
    bs << stat.mLastPlayed;
    bs << stat.unk78;
    bs << stat.unk79;
    bs << (unsigned char)stat.unk7c;
    bs << stat.mLastScore;
    bs << stat.unk84;
    bs << stat.mLastPlayedPractice;
    bs << stat.unk9c;
    bs << (unsigned char)stat.unka0;
    bs << stat.mBattleScore;
    bs << stat.mTotalBattleWins;
    bs << stat.mTotalBattleLosses;
    bs << stat.mWonLastBattle;
    bs << stat.unka4;
    stat.mFlauntData.SaveToStream(bs);
    return bs;
}

BinStream &operator>>(BinStream &bs, SongStatus &stat) {
    bs >> stat.mSongID;
    for (int i = 0; i < 4; i++) {
        stat.mStatusData[i].LoadFromStream(bs);
        stat.mStatusData[i].mSongID = stat.mSongID;
    }
    bs >> stat.mLastPlayed;
    bs >> stat.unk78;
    bs >> stat.unk79;
    unsigned char uc;
    bs >> uc;
    stat.unk7c = (Difficulty)uc;
    bs >> stat.mLastScore;
    bs >> stat.unk84;
    bs >> stat.mLastPlayedPractice;
    bs >> stat.unk9c;
    bs >> uc;
    stat.unka0 = uc;
    bs >> stat.mBattleScore;
    bs >> stat.mTotalBattleWins;
    bs >> stat.mTotalBattleLosses;
    bs >> stat.mWonLastBattle;
    bs >> stat.unka4;
    stat.mFlauntData.LoadFromStream(bs);
    return bs;
}

#pragma endregion
#pragma region SongStatusMgr

namespace {
    DataNode OnToggleFakeLeaderboardUploadFailure(DataArray *a) {
        SongStatusMgr::sFakeLeaderboardUploadFailure =
            !SongStatusMgr::sFakeLeaderboardUploadFailure;
        Hmx::Object *cheatDisplay = ObjectDir::Main()->Find<Hmx::Object>("cheat_display");
        if (cheatDisplay) {
            static Message msg("show_bool", "Fake leaderboard upload failure", 0);
            msg[1] = SongStatusMgr::sFakeLeaderboardUploadFailure;
            cheatDisplay->Handle(msg, false);
        }
        return 0;
    }
}

SongStatusMgr::SongStatusMgr(HamSongMgr *h) : mSongMgr(h) {
    mSaveSizeMethod = &SaveSize;
    TheContentMgr.RegisterCallback(this, false);
}

BEGIN_HANDLERS(SongStatusMgr)
END_HANDLERS

void SongStatusMgr::Init() {
    DataRegisterFunc(
        "toggle_fake_leaderboard_upload_failure", OnToggleFakeLeaderboardUploadFailure
    );
}

int SongStatusMgr::SaveSize(int) { return 0x6cbdc; }

SongStatus &SongStatusMgr::AccessSongStatus(int songID) {
    auto iterSongStatus = mSongStatusMap.find(songID);
    MILO_ASSERT(iterSongStatus != mSongStatusMap.end(), 0x1B5);
    return iterSongStatus->second;
}

const SongStatus &SongStatusMgr::GetSongStatus(int songID) const {
    auto iterSongStatus = mSongStatusMap.find(songID);
    MILO_ASSERT(iterSongStatus != mSongStatusMap.end(), 0x1BD);
    return iterSongStatus->second;
}

void SongStatusMgr::Clear() { mSongStatusMap.clear(); }

void SongStatusMgr::ClearNeedUpload(int songID, Difficulty d) {
    if (HasSongStatus(songID)) {
        AccessSongStatus(songID).mStatusData[d].mNeedUpload = false;
    }
}

void SongStatusMgr::ClearFlauntsNeedUpload(int songID) {
    if (HasSongStatus(songID)) {
        AccessSongStatus(songID).mFlauntData.mNeedUpload = false;
    }
}

bool SongStatusMgr::HasSongStatus(int songID) const {
    return mSongStatusMap.find(songID) != mSongStatusMap.end();
}

bool SongStatusMgr::IsSongPlayed(int songID) const { return HasSongStatus(songID); }

void SongStatusMgr::GetScoresToUpload(std::list<SongStatusData> &data) {
    FOREACH (it, mSongStatusMap) {
        SongStatus cur = it->second;
        for (int i = 0; i < kNumDifficulties; i++) {
            if (cur.mStatusData[i].unk10) {
                data.push_back(cur.mStatusData[i]);
            }
        }
    }
}

void SongStatusMgr::GetFlauntsToUpload(std::list<FlauntStatusData> &data) {
    FOREACH (it, mSongStatusMap) {
        SongStatus cur = it->second;
        if (cur.unk78) {
            data.push_back(cur.mFlauntData);
        }
    }
}

Difficulty __cdecl SongStatusMgr::GetDifficulty(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestSongStatusData().mDifficulty;
    } else {
        return DefaultDifficulty();
    }
}

int SongStatusMgr::GetPercentForDifficulty(int songID, Difficulty d) const {
    int pct = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        pct = status.mStatusData[d].mPercentPassed;
    }
    return pct;
}

int SongStatusMgr::GetNumPerfectForDifficulty(int songID, Difficulty d) const {
    int num = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        num = status.mStatusData[d].mNumPerfects;
    }
    return num;
}

int SongStatusMgr::GetNumNiceForDifficulty(int songID, Difficulty d) const {
    int num = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        num = status.mStatusData[d].mNumNices;
    }
    return num;
}

Difficulty __cdecl SongStatusMgr::GetPracticeDifficulty(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestPracticeSongStatusData().mDifficulty;
    } else {
        return DefaultDifficulty();
    }
}

int SongStatusMgr::GetScore(int songID, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatusData &data = GetSongStatus(songID).GetBestSongStatusData();
        bref = data.mNoFlashcards;
        return data.mScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetCoopScore(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestSongStatusData().mCoopScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetScoreForDifficulty(int songID, Difficulty d, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        bref = status.mStatusData[d].mNoFlashcards;
        return status.mStatusData[d].mScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetBestScore(int songID, bool &bref, Difficulty d) const {
    int bestScore = 0;
    bref = false;
    if (HasSongStatus(songID) && d != kNumDifficulties) {
        for (; d != kNumDifficulties; d = DifficultyOneHarder(d)) {
            const SongStatus &status = GetSongStatus(songID);
            int score = status.mStatusData[d].mScore;
            if (score > bestScore) {
                bref = status.mStatusData[d].mNoFlashcards;
                bestScore = score;
            }
        }
    }
    return bestScore;
}

int SongStatusMgr::GetStars(int songID, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatusData &data = GetSongStatus(songID).GetBestSongStatusData();
        bref = data.unk10;
        return data.mStars;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetStarsForDifficulty(int songID, Difficulty d, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        bref = status.mStatusData[d].unk10;
        return status.mStatusData[d].mStars;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetBestStars(int songID, bool &bref, Difficulty d) const {
    int bestStars = 0;
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        for (; d != kNumDifficulties; d = DifficultyOneHarder(d)) {
            int curStars = status.mStatusData[d].mStars;
            if (curStars >= bestStars) {
                bref = status.mStatusData[d].unk10;
                bestStars = curStars;
            }
        }
    }
    return bestStars;
}

int SongStatusMgr::GetBestBattleScore(int songID) const {
    int bestScore = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        for (int i = 0; i < kNumDifficulties; i++) {
            if (status.mBattleScore >= bestScore) {
                bestScore = status.mBattleScore;
            }
        }
    }
    return bestScore;
}

int SongStatusMgr::GetTotalBattleWins(int songID) const {
    int total = 0;
    if (HasSongStatus(songID)) {
        total = GetSongStatus(songID).mTotalBattleWins;
    }
    return total;
}

int SongStatusMgr::GetTotalBattleLosses(int songID) const {
    int total = 0;
    if (HasSongStatus(songID)) {
        total = GetSongStatus(songID).mTotalBattleLosses;
    }
    return total;
}

bool SongStatusMgr::GetLastBattleResult(int songID) const {
    bool res = false;
    if (HasSongStatus(songID)) {
        res = GetSongStatus(songID).mWonLastBattle;
    }
    return res;
}

unsigned int SongStatusMgr::GetLastPlayed(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).mLastPlayed;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetLastScore(int songID, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        bref = status.unk79;
        return status.mLastScore;
    } else {
        return 0;
    }
}

unsigned int SongStatusMgr::GetLastPlayedPractice(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).mLastPlayedPractice;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetPracticeScore(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestPracticeSongStatusData().mPracticeScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetPracticeScore(int songID, Difficulty d) const {
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        return status.mStatusData[d].mPracticeScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::CalculateTotalScore(Symbol game_origin) const {
    int total = 0;
    FOREACH (it, mSongStatusMap) {
        int curSongID = it->first;
        if (mSongMgr->HasSong(curSongID)) {
            const HamSongMetadata *metaData = mSongMgr->Data(curSongID);
            MILO_ASSERT(metaData, 0x19C);
            if (game_origin == gNullStr || game_origin == metaData->GameOrigin()) {
                bool bref;
                total += GetBestScore(curSongID, bref, kDifficultyBeginner);
                if (total > 2000000000) {
                    return 2000000000;
                }
            }
        }
    }
    return total;
}

bool SongStatusMgr::UpdateFlaunt(int songID, int score, Difficulty d, bool b3) {
    MILO_ASSERT(songID != kSongID_Invalid && songID != kSongID_Any && songID != kSongID_Random, 0x14A);
    if (HasSongStatus(songID)) {
        SongStatus &status = AccessSongStatus(songID);
        status.mFlauntData.mScore = score;
        status.mFlauntData.mDiff = d;
        status.mFlauntData.mNeedUpload = !b3;
    } else {
        SongStatus status(songID);
        status.mFlauntData.mNeedUpload = !b3;
        status.mFlauntData.mScore = score;
        status.mFlauntData.mDiff = d;
        mSongStatusMap[songID] = status;
    }
    return true;
}

bool SongStatusMgr::UpdateBattleSong(int songID, int score, bool won) {
    MILO_ASSERT(songID != kSongID_Invalid && songID != kSongID_Any && songID != kSongID_Random, 0xF5);
    static Symbol dance_battle("dance_battle");
    if (!TheGameMode->InMode(dance_battle, true)) {
        return false;
    } else {
        if (HasSongStatus(songID)) {
            SongStatus &status = AccessSongStatus(songID);
            status.mWonLastBattle = won;
            if (won) {
                status.mTotalBattleWins++;
            } else {
                status.mTotalBattleLosses++;
            }
            if (status.mBattleScore <= score) {
                status.mBattleScore = score;
            }
        } else {
            SongStatus status(songID);
            status.mWonLastBattle = won;
            if (won) {
                status.mTotalBattleWins++;
            } else {
                status.mTotalBattleLosses++;
            }
            status.mBattleScore = score;
            mSongStatusMap[songID] = status;
        }
        return true;
    }
}

bool SongStatusMgr::UpdateSong(
    int songID,
    int score,
    int i3,
    Difficulty difficulty,
    int i5,
    int stars,
    int numNices,
    int numPerfects,
    int percentPassed,
    bool b10,
    bool b11,
    bool b12
) {
    MILO_ASSERT(songID != kSongID_Invalid && songID != kSongID_Any && songID != kSongID_Random, 0x90);
    static Symbol practice("practice");
    static Symbol perform("perform");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol dance_battle("dance_battle");
    if (HasSongStatus(songID)) {
        bool ret = false;
        SongStatus &status = AccessSongStatus(songID);
        if (TheGameMode->Property(gameplay_mode)->Sym() == perform || b12) {
            DateTime dt;
            GetDateAndTime(dt);
            status.mLastPlayed = dt.ToCode();
            status.unk78 = stars;
            if (stars >= 5) {
                status.unk79 = b11;
            }
            status.unk7c = difficulty;
            status.mLastScore = score;
            status.unk84 = i3;
            if (status.mStatusData[difficulty].mScore <= score) {
                status.mStatusData[difficulty].mScore = score;
                status.mStatusData[difficulty].mNoFlashcards = b11;
                status.mStatusData[difficulty].mNeedUpload = !b10;
            }
            if (status.mStatusData[difficulty].mCoopScore <= i3) {
                status.mStatusData[difficulty].mCoopScore = i3;
                status.mStatusData[difficulty].mNeedUpload = !b10;
            }
            if (status.mStatusData[difficulty].mStars <= stars) {
                status.mStatusData[difficulty].mStars = stars;
                if (b11 && 5 <= stars) {
                    status.mStatusData[difficulty].unk10 = b11;
                }
            }
            if (status.mStatusData[difficulty].mNumNices <= numNices) {
                status.mStatusData[difficulty].mNumNices = numNices;
            }
            if (status.mStatusData[difficulty].mNumPerfects <= numPerfects) {
                status.mStatusData[difficulty].mNumPerfects = numPerfects;
            }
            if (status.mStatusData[difficulty].mPercentPassed <= percentPassed) {
                status.mStatusData[difficulty].mPercentPassed = percentPassed;
            }
            ret = true;
        }
        return ret;
    } else {
        SongStatus status(songID);
        if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
            status.mStatusData[difficulty].mScore = score;
            status.mStatusData[difficulty].mCoopScore = i3;
            status.mStatusData[difficulty].mStars = stars;
            status.mStatusData[difficulty].mNoFlashcards = b11;
            if (5 <= stars) {
                status.mStatusData[difficulty].unk10 = b11;
            }
            DateTime dt;
            status.mStatusData[difficulty].mNeedUpload = !b10;
            GetDateAndTime(dt);
            status.mLastPlayed = dt.ToCode();
            status.unk78 = stars;
            if (5 <= stars) {
                status.unk79 = b11;
            }
            status.unk7c = difficulty;
            status.mLastScore = score;
            status.unk84 = i3;
            status.mStatusData[difficulty].mNumNices = numNices;
            status.mStatusData[difficulty].mNumPerfects = numPerfects;
            status.mStatusData[difficulty].mPercentPassed = percentPassed;
        }
        mSongStatusMap[songID] = status;
        return true;
    }
}

#pragma endregion
