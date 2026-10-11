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
    bs << mMovesPercent;
    bs << mNumPerfect;
    bs << mNumNice;
    bs << mStarsNoFlashcards;
    bs << mScoreNoFlashcards;
    bs << mNeedUpload;
}

void SongStatusData::LoadFromStream(BinStream &bs) {
    bs >> mScore;
    bs >> mPracticeScore;
    bs >> mCoopScore;
    bs >> mStars;
    bs >> mMovesPercent;
    bs >> mNumPerfect;
    bs >> mNumNice;
    bs >> mStarsNoFlashcards;
    bs >> mScoreNoFlashcards;
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
        mSongData[i].Clear();
    }
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
    Clear();
}

SongStatus::SongStatus(int songID) {
    for (int i = 0; i < 4; i++) {
        mSongData[i].Clear();
    }
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
    Clear();
    mSongID = songID;
    for (int i = 0; i < 4; i++) {
        mSongData[i].mSongID = mSongID;
    }
}

void SongStatus::Clear() {
    mSongID = 0;
    for (int i = 0; i < kNumDifficulties; i++) {
        mSongData[i].Clear();
        mSongData[i].mDiff = (Difficulty)i;
        mSongData[i].mSongID = mSongID;
    }
    mLastPlayed = 0;
    mLastStars = false;
    mLastStarsNoFlashcards = false;
    mLastDiff = kNumDifficulties;
    mLastScore = 0;
    mLastCoopScore = 0;
    mLastPlayedPractice = 0;
    mLastScorePractice = 0;
    mLastDiffPractice = kNumDifficulties;
    mBattleScore = 0;
    mBattleWins = 0;
    mBattleLosses = 0;
    mWonLastDanceBattle = false;
    mFlags = 0;
    mFlauntData.mScore = 0;
    mFlauntData.mDiff = DefaultDifficulty();
    mFlauntData.mNeedUpload = false;
}

const SongStatusData &SongStatus::GetBestSongStatusData() const {
    Difficulty best = kDifficultyEasy;
    for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        if (0 < mSongData[d].mScore
            && (3 <= mSongData[d].mStars
                || mSongData[d].mStars >= mSongData[best].mStars)) {
            best = d;
        }
    }
    return mSongData[best];
}

const SongStatusData &SongStatus::GetBestPracticeSongStatusData() const {
    Difficulty best = kDifficultyEasy;
    for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        if ((50 <= mSongData[d].mPracticeScore
             || mSongData[d].mPracticeScore >= mSongData[best].mPracticeScore)) {
            best = d;
        }
    }
    return mSongData[best];
}

BinStream &operator<<(BinStream &bs, const SongStatus &stat) {
    bs << stat.mSongID;
    for (int i = 0; i < 4; i++) {
        stat.mSongData[i].SaveToStream(bs);
    }
    bs << stat.mLastPlayed;
    bs << stat.mLastStars;
    bs << stat.mLastStarsNoFlashcards;
    bs << (unsigned char)stat.mLastDiff;
    bs << stat.mLastScore;
    bs << stat.mLastCoopScore;
    bs << stat.mLastPlayedPractice;
    bs << stat.mLastScorePractice;
    bs << (unsigned char)stat.mLastDiffPractice;
    bs << stat.mBattleScore;
    bs << stat.mBattleWins;
    bs << stat.mBattleLosses;
    bs << stat.mWonLastDanceBattle;
    bs << stat.mFlags;
    stat.mFlauntData.SaveToStream(bs);
    return bs;
}

BinStream &operator>>(BinStream &bs, SongStatus &stat) {
    bs >> stat.mSongID;
    for (int i = 0; i < 4; i++) {
        stat.mSongData[i].LoadFromStream(bs);
        stat.mSongData[i].mSongID = stat.mSongID;
    }
    bs >> stat.mLastPlayed;
    bs >> stat.mLastStars;
    bs >> stat.mLastStarsNoFlashcards;
    unsigned char uc;
    bs >> uc;
    stat.mLastDiff = (Difficulty)uc;
    bs >> stat.mLastScore;
    bs >> stat.mLastCoopScore;
    bs >> stat.mLastPlayedPractice;
    bs >> stat.mLastScorePractice;
    bs >> uc;
    stat.mLastDiffPractice = (Difficulty)uc;
    bs >> stat.mBattleScore;
    bs >> stat.mBattleWins;
    bs >> stat.mBattleLosses;
    bs >> stat.mWonLastDanceBattle;
    bs >> stat.mFlags;
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
        AccessSongStatus(songID).mSongData[d].mNeedUpload = false;
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
        for (int i = 0; i < 4; i++) {
            if (cur.mSongData[i].mNeedUpload) {
                cur.mSongData[i].mDiff = (Difficulty)i;
                data.push_back(cur.mSongData[i]);
            }
        }
    }
}

void SongStatusMgr::GetFlauntsToUpload(std::list<FlauntStatusData> &data) {
    FOREACH (it, mSongStatusMap) {
        SongStatus cur = it->second;
        if (cur.mFlauntData.mNeedUpload) {
            cur.mFlauntData.mSongID = cur.mSongID;
            data.push_back(cur.mFlauntData);
        }
    }
}

Difficulty SongStatusMgr::GetDifficulty(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestSongStatusData().mDiff;
    } else {
        return DefaultDifficulty();
    }
}

int SongStatusMgr::GetPercentForDifficulty(int songID, Difficulty d) const {
    int pct = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        pct = status.mSongData[d].mMovesPercent;
    }
    return pct;
}

int SongStatusMgr::GetNumPerfectForDifficulty(int songID, Difficulty d) const {
    int num = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        num = status.mSongData[d].mNumPerfect;
    }
    return num;
}

int SongStatusMgr::GetNumNiceForDifficulty(int songID, Difficulty d) const {
    int num = 0;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        num = status.mSongData[d].mNumNice;
    }
    return num;
}

Difficulty __cdecl SongStatusMgr::GetPracticeDifficulty(int songID) const {
    if (HasSongStatus(songID)) {
        return GetSongStatus(songID).GetBestPracticeSongStatusData().mDiff;
    } else {
        return DefaultDifficulty();
    }
}

int SongStatusMgr::GetScore(int songID, bool &noFlashcards) const {
    noFlashcards = false;
    if (HasSongStatus(songID)) {
        const SongStatusData &data = GetSongStatus(songID).GetBestSongStatusData();
        noFlashcards = data.mScoreNoFlashcards;
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

int SongStatusMgr::GetScoreForDifficulty(
    int songID, Difficulty d, bool &noFlashcards
) const {
    noFlashcards = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        noFlashcards = status.mSongData[d].mScoreNoFlashcards;
        return status.mSongData[d].mScore;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetBestScore(int songID, bool &noFlashcards, Difficulty d) const {
    int bestScore = 0;
    noFlashcards = false;
    if (HasSongStatus(songID)) {
        for (Difficulty loopdiff = d; loopdiff != kNumDifficulties;
             loopdiff = DifficultyOneHarder(loopdiff)) {
            const SongStatus &status = GetSongStatus(songID);
            int score = status.mSongData[loopdiff].mScore;
            if (score > bestScore) {
                noFlashcards = status.mSongData[loopdiff].mScoreNoFlashcards;
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
        bref = data.mStarsNoFlashcards;
        return data.mStars;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetStarsForDifficulty(int songID, Difficulty d, bool &bref) const {
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        bref = status.mSongData[d].mStarsNoFlashcards;
        return status.mSongData[d].mStars;
    } else {
        return 0;
    }
}

int SongStatusMgr::GetBestStars(int songID, bool &bref, Difficulty d) const {
    int bestStars = 0;
    bref = false;
    if (HasSongStatus(songID)) {
        const SongStatus &status = GetSongStatus(songID);
        for (Difficulty loopDiff = d; loopDiff != kNumDifficulties;
             loopDiff = DifficultyOneHarder(loopDiff)) {
            int curStars = status.mSongData[loopDiff].mStars;
            if (curStars >= bestStars) {
                bref = status.mSongData[loopDiff].mStarsNoFlashcards;
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
        total = GetSongStatus(songID).mBattleWins;
    }
    return total;
}

int SongStatusMgr::GetTotalBattleLosses(int songID) const {
    int total = 0;
    if (HasSongStatus(songID)) {
        total = GetSongStatus(songID).mBattleLosses;
    }
    return total;
}

bool SongStatusMgr::GetLastBattleResult(int songID) const {
    bool res = false;
    if (HasSongStatus(songID)) {
        res = GetSongStatus(songID).mWonLastDanceBattle;
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
        bref = status.mLastStarsNoFlashcards;
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
        return status.mSongData[d].mPracticeScore;
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
            status.mWonLastDanceBattle = won;
            if (won) {
                status.mBattleWins++;
            } else {
                status.mBattleLosses++;
            }
            if (status.mBattleScore <= score) {
                status.mBattleScore = score;
            }
        } else {
            SongStatus status(songID);
            status.mWonLastDanceBattle = won;
            if (won) {
                status.mBattleWins++;
            } else {
                status.mBattleLosses++;
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
            status.mLastStars = stars;
            if (stars >= 5) {
                status.mLastStarsNoFlashcards = b11;
            }
            status.mLastDiff = difficulty;
            status.mLastScore = score;
            status.mLastCoopScore = i3;
            if (status.mSongData[difficulty].mScore <= score) {
                status.mSongData[difficulty].mScore = score;
                status.mSongData[difficulty].mScoreNoFlashcards = b11;
                status.mSongData[difficulty].mNeedUpload = !b10;
            }
            if (status.mSongData[difficulty].mCoopScore <= i3) {
                status.mSongData[difficulty].mCoopScore = i3;
                status.mSongData[difficulty].mNeedUpload = !b10;
            }
            if (status.mSongData[difficulty].mStars <= stars) {
                status.mSongData[difficulty].mStars = stars;
                if (b11 && 5 <= stars) {
                    status.mSongData[difficulty].mStarsNoFlashcards = b11;
                }
            }
            if (status.mSongData[difficulty].mNumNice <= numNices) {
                status.mSongData[difficulty].mNumNice = numNices;
            }
            if (status.mSongData[difficulty].mNumPerfect <= numPerfects) {
                status.mSongData[difficulty].mNumPerfect = numPerfects;
            }
            if (status.mSongData[difficulty].mMovesPercent <= percentPassed) {
                status.mSongData[difficulty].mMovesPercent = percentPassed;
            }
            ret = true;
        }
        return ret;
    } else {
        SongStatus status(songID);
        if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
            status.mSongData[difficulty].mScore = score;
            status.mSongData[difficulty].mCoopScore = i3;
            status.mSongData[difficulty].mStars = stars;
            status.mSongData[difficulty].mScoreNoFlashcards = b11;
            if (5 <= stars) {
                status.mSongData[difficulty].mStarsNoFlashcards = b11;
            }
            DateTime dt;
            status.mSongData[difficulty].mNeedUpload = !b10;
            GetDateAndTime(dt);
            status.mLastPlayed = dt.ToCode();
            status.mLastStars = stars;
            if (5 <= stars) {
                status.mLastStarsNoFlashcards = b11;
            }
            status.mLastDiff = difficulty;
            status.mLastScore = score;
            status.mLastCoopScore = i3;
            status.mSongData[difficulty].mNumNice = numNices;
            status.mSongData[difficulty].mNumPerfect = numPerfects;
            status.mSongData[difficulty].mMovesPercent = percentPassed;
        }
        mSongStatusMap[songID] = status;
        return true;
    }
}

#pragma endregion
