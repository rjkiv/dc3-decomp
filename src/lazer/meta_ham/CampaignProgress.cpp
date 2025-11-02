#include "lazer/meta_ham/CampaignProgress.h"
#include "Campaign.h"
#include "lazer/meta_ham/CampaignEra.h"
#include "macros.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region CampaignEraSongProgress

CampaignEraSongProgress::CampaignEraSongProgress()
    : unk8(-1), unk24(false), unk25(false), unk28(-1), unk44(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraSongProgress::~CampaignEraSongProgress() { Cleanup(); }

int CampaignEraSongProgress::SaveSize(int i) {
    int s = 0x1a;
    if (FixedSizeSaveable::sPrintoutsEnabled) {
        MILO_LOG("* %s = %i\n", "CampaignEraSongProgress", s);
    }
    return 0x1a;
}

bool CampaignEraSongProgress::SetMoveMastered(Symbol s1, Symbol s2, Symbol s3) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(s1);
    MILO_ASSERT(pEra, 0x72);
    if (s2 == pEra->GetDanceCrazeSong()) {
        return false;
    } else {
        auto it = unkc.find(s3);
        if (it == unkc.begin()) {
        }
        unkc.insert(s2);
        return true;
    }
    return false;
}

void CampaignEraSongProgress::BookmarkCurrentProgress() {
    unk28 = unk8;
    unk2c = unkc;
    unk44 = unk24;
}

void CampaignEraSongProgress::ResetProgressToBookmark() {
    unk8 = unk28;
    unkc = unk2c;
    unk24 = unk44;
}

void CampaignEraSongProgress::SaveFixed(FixedSizeSaveableStream &stream) const {}

void CampaignEraSongProgress::LoadFixed(FixedSizeSaveableStream &, int) {}

void CampaignEraSongProgress::Cleanup() {
    unkc.clear();
    unk8 = -1;
    unk24 = false;
    BookmarkCurrentProgress();
}

#pragma endregion CampaignEraSongProgress
#pragma region CampaignEraProgress

CampaignEraProgress::CampaignEraProgress() : unk8(gNullStr), unkc(false), unkd(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraProgress::CampaignEraProgress(Symbol s) : unk8(s), unkc(false), unkd(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraProgress::~CampaignEraProgress() { Cleanup(); }

int CampaignEraProgress::SaveSize(int i) {
    int savesize = CampaignEraSongProgress::SaveSize(i);
    int x = (savesize + 5) * 10;
    if (FixedSizeSaveable::sPrintoutsEnabled != false) {
        MILO_LOG("* %s = %i\n", "CampaignEraSongProgress", x);
    }
    return x;
}

int CampaignEraProgress::GetTotalStarsEarned() const {
    int totalStars = 0;
    CampaignEra *pEra = TheCampaign->GetCampaignEra(unk8);
    MILO_ASSERT(pEra, 0x106);

    int size = pEra->m_vSongs.size();
    for (int i = 0; i < size; i++) {
        Symbol name = pEra->GetSongName(i);
        int total = 0;
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(name);
        if (pEraSongProgress)
            total = pEraSongProgress->unk8;

        if (total < 0)
            totalStars = 0;

        if (total >= 5)
            total = 5;

        totalStars += total;
    }

    return totalStars;
}

int CampaignEraProgress::GetTotalMovesMastered() const {
    int total = 0;
    CampaignEra *pEra = TheCampaign->GetCampaignEra(unk8);
    MILO_ASSERT(pEra, 0x124);
    int size = pEra->m_vSongs.size();
    for (int i = 0; i < size; i++) {
        Symbol songName = pEra->GetSongName(i);
        int x = 0;
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(songName);
        if (pEraSongProgress) {
            x = pEraSongProgress->unkc.size();
        }
        total += x;
    }
    return total;
}

bool CampaignEraProgress::IsMoveMastered(Symbol s1, Symbol s2) const {
    CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(s1);
    if (pEraSongProgress) {
        auto it = pEraSongProgress->unkc.find(s2);
    }
    return false;
}

bool CampaignEraProgress::IsMastered() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(unk8);
    MILO_ASSERT(pEra, 0x155);
    int totalStars = GetTotalStarsEarned();
    int totalMasteredMoves = GetTotalMovesMastered();
    if (totalStars < pEra->unk60 || totalMasteredMoves < pEra->unk70) {
        return false;
    }
    return true;
}

bool CampaignEraProgress::IsEraComplete() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(unk8);
    MILO_ASSERT(pEra, 0x163);
    if (IsMastered()) {
        if (pEra->GetDanceCrazeSong()) {
            return true;
        }
    }
    return false;
}

bool CampaignEraProgress::IsPlayed() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(unk8);
    MILO_ASSERT(pEra, 0x16c);
    int size = pEra->m_vSongs.size();
    for (int i = 0; i < size; i++) {
        Symbol songName = pEra->GetSongName(i);
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(songName);
        if (pEraSongProgress && pEraSongProgress->unk24) {
            return true;
        }
    }

    return false;
}

void CampaignEraProgress::BookmarkCurrentProgress() {
    FOREACH (it, m_mapCampaignEraSongProgress) {
        auto cEraSongProgress = it->second;
        cEraSongProgress->BookmarkCurrentProgress();
    }
}

void CampaignEraProgress::ResetProgressToBookmark() {
    FOREACH (it, m_mapCampaignEraSongProgress) {
        auto cEraSongProgress = it->second;
        cEraSongProgress->ResetProgressToBookmark();
    }
}

void CampaignEraProgress::SetSongPlayed(Symbol s, bool b) {}

void CampaignEraProgress::UpdateSong(Symbol, int) {}

bool CampaignEraProgress::UpdateSongMoveMastered(Symbol, Symbol) { return false; }

CampaignEraSongProgress *CampaignEraProgress::GetEraSongProgress(Symbol s) const {
    auto it = m_mapCampaignEraSongProgress.find(s);
    return (it != m_mapCampaignEraSongProgress.end()) ? it->second : nullptr;
}

void CampaignEraProgress::SaveFixed(FixedSizeSaveableStream &) const {}

void CampaignEraProgress::LoadFixed(FixedSizeSaveableStream &, int) {}

void CampaignEraProgress::Cleanup() {
    FOREACH (it, m_mapCampaignEraSongProgress) {
        RELEASE(it->second);
    }
    m_mapCampaignEraSongProgress.clear();
}

#pragma endregion CampaignEraProgress
#pragma region CampaignProgress

CampaignProgress::CampaignProgress()
    : unk8(0), unkc(false), unkd(false), unke(false), unk28(0) {
    Clear();
}

CampaignProgress::~CampaignProgress() { Clear(); }

void CampaignProgress::SaveFixed(FixedSizeSaveableStream &) const {}

void CampaignProgress::LoadFixed(FixedSizeSaveableStream &, int) {}

void CampaignProgress::SetCampaignIntroCompleted(bool b) { unkc = b; }

void CampaignProgress::SetCampaignMindControlCompleted(bool) {}

bool CampaignProgress::IsCampaignTanBattleCompleted() const { return unke; }

void CampaignProgress::SetCampaignTanBattleCompleted(bool) {}

int CampaignProgress::SaveSize(int) { return 1; }

int CampaignProgress::GetRequiredStarsForDanceCrazeSong(Symbol s) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(s);
    MILO_ASSERT(pEra, 0x310);
    Symbol song = pEra->GetDanceCrazeSong();
    return pEra->GetSongRequiredStars(song);
}

bool CampaignProgress::IsEraPlayed(Symbol s) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(s);
    MILO_ASSERT(pEra, 0x287);

    CampaignEraProgress *pEraProgress = GetEraProgress(s);
    if (pEraProgress) {
        return pEraProgress->IsPlayed();
    }
    return false;
}

bool CampaignProgress::IsEraComplete(Symbol s) const {
    CampaignEraProgress *pEraProgress = GetEraProgress(s);
    if (pEraProgress) {
        return pEraProgress->IsEraComplete();
    } else {
        return false;
    }
}

bool CampaignProgress::IsEraMastered(Symbol) const { return false; }

int CampaignProgress::GetEraStarsEarned(Symbol) const { return 1; }

int CampaignProgress::GetEraMovesMastered(Symbol) const { return 1; }

bool CampaignProgress::GetEraIntroMoviePlayed(Symbol) const { return false; }

void CampaignProgress::SetEraIntroMoviePlayed(Symbol, bool) {}

bool CampaignProgress::IsEraSongAvailable(Symbol, Symbol) const { return false; }

bool CampaignProgress::IsSongPlayed(Symbol, Symbol) const { return false; }

int CampaignProgress::GetSongStarsEarned(Symbol, Symbol) const { return 1; }

bool CampaignProgress::IsMoveMastered(Symbol, Symbol, Symbol) const { return false; }

bool CampaignProgress::IsCampaignNew() const { return false; }

Symbol CampaignProgress::GetFirstIncompleteEra() const { return 0; }

int CampaignProgress::GetStars() const { return 1; }

int CampaignProgress::GetNumCompletedEras() const { return 1; }

void CampaignProgress::Clear() {}

bool CampaignProgress::IsDanceCrazeSongAvailable(Symbol) const { return false; }

void CampaignProgress::ResetAllProgress() {}

void CampaignProgress::ClearSongProgress(Symbol, Symbol) {}

void CampaignProgress::BookmarkCurrentProgress() {}

void CampaignProgress::ResetProgressToBookmark() {}

void CampaignProgress::SetSongPlayed(Symbol, Symbol, bool) {}

void CampaignProgress::UpdateEraSong(Symbol, Symbol, int) {}

bool CampaignProgress::UpdateEraSongMoveMastered(Symbol, Symbol, Symbol) { return true; }

void CampaignProgress::UnlockAllMoves(Symbol, Symbol) {}

CampaignEraProgress *CampaignProgress::GetEraProgress(Symbol s) const {
    auto it = unk10.find(s);
    return (it != unk10.end()) ? it->second : nullptr;
}

#pragma endregion CampaignProgress
