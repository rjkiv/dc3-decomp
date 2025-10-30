#include "lazer/meta_ham/SongStatusMgr.h"
#include "SongStatusMgr.h"
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/HamSongMgr.h"
#include "macros.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

namespace {};

#pragma region SongStatusData

void SongStatusData::SaveToStream(BinStream &) const {}

void SongStatusData::LoadFromStream(BinStream &) {}

#pragma endregion SongStatusData
#pragma region FlauntStatusData

void FlauntStatusData::SaveToStream(BinStream &) const {}

void FlauntStatusData::LoadFromStream(BinStream &) {}

#pragma endregion FlauntStatusData
#pragma region SongStatus

SongStatus::SongStatus() {}

SongStatus::SongStatus(int) {}

void SongStatus::Clear() {}

#pragma endregion SongStatus
#pragma region SongStatusMgr

SongStatusMgr::SongStatusMgr(HamSongMgr *h) : unk34(), unk38(h) {
    mSaveSizeMethod = &SaveSize;
    TheContentMgr.RegisterCallback(nullptr, false);
}

SongStatusMgr::~SongStatusMgr() { unk3c.clear(); }

int SongStatusMgr::SaveSize(int) { return 0x6cbdc; }

void SongStatusMgr::Init() {}

void SongStatusMgr::GetScoresToUpload(std::list<SongStatusData> &) {}

void SongStatusMgr::GetFlauntsToUpload(std::list<FlauntStatusData> &) {}

bool SongStatusMgr::HasSongStatus(int) const { return false; }

Difficulty SongStatusMgr::GetDifficulty(int) const { return kDifficultyExpert; }

int SongStatusMgr::GetScore(int, bool &) const { return 1; }

bool SongStatusMgr::IsSongPlayed(int) const { return false; }

int SongStatusMgr::GetCoopScore(int) const { return 1; }

int SongStatusMgr::GetScoreForDifficulty(int, Difficulty, bool &) const { return 1; }

int SongStatusMgr::GetBestScore(int, bool &, Difficulty) const { return 1; }

int SongStatusMgr::GetStars(int, bool &) const { return 1; }

int SongStatusMgr::GetStarsForDifficulty(int, Difficulty, bool &) const { return 1; }

int SongStatusMgr::GetBestStars(int, bool &, Difficulty) const { return 1; }

int SongStatusMgr::GetPercentForDifficulty(int, Difficulty) const { return 1; }

int SongStatusMgr::GetNumPerfectForDifficulty(int, Difficulty) const { return 1; }

int SongStatusMgr::GetNumNiceForDifficulty(int, Difficulty) const { return 1; }

int SongStatusMgr::GetBestBattleScore(int) const { return 1; }

int SongStatusMgr::GetTotalBattleWins(int) const { return 1; }

int SongStatusMgr::GetTotalBattleLosses(int) const { return 1; }

bool SongStatusMgr::GetLastBattleResult(int) const { return false; }

unsigned int SongStatusMgr::GetLastPlayed(int) const { return 0; }

int SongStatusMgr::GetLastScore(int, bool &) const { return 1; }

unsigned int SongStatusMgr::GetLastPlayedPractice(int) const { return 0; }

int SongStatusMgr::GetPracticeScore(int) const { return 1; }

int SongStatusMgr::GetPracticeScore(int, Difficulty) const { return 1; }

Difficulty SongStatusMgr::GetPracticeDifficulty(int) const { return kDifficultyEasy; }

void SongStatusMgr::Clear() {}

void SongStatusMgr::ClearNeedUpload(int, Difficulty) {}

void SongStatusMgr::ClearFlauntsNeedUpload(int) {}

int SongStatusMgr::CalculateTotalScore(Symbol) const { return 1; }

bool SongStatusMgr::UpdateSong(
    int, int, int, Difficulty, int, int, int, int, int, bool, bool, bool
) {
    return false;
}

bool SongStatusMgr::UpdateBattleSong(int, int, bool) { return false; }

bool SongStatusMgr::UpdateFlaunt(int, int, Difficulty, bool) { return false; }

void SongStatusMgr::SaveFixed(FixedSizeSaveableStream &) const {}

void SongStatusMgr::LoadFixed(FixedSizeSaveableStream &, int) {}

#pragma endregion SongStatusMgr
