#include "meta_ham/CampaignProgress.h"
#include "Campaign.h"
#include "flow/PropertyEventProvider.h"
#include "meta_ham/CampaignEra.h"
#include "macros.h"
#include "meta_ham/HamProfile.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region CampaignEraSongProgress

CampaignEraSongProgress::CampaignEraSongProgress()
    : mStarsEarned(-1), mSongPlayed(false), unk25(false), mBookmarkedStarsEarned(-1),
      mBookmarkedSongPlayed(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraSongProgress::~CampaignEraSongProgress() { Cleanup(); }

void CampaignEraSongProgress::SaveFixed(FixedSizeSaveableStream &stream) const {
    FixedSizeSaveable::SaveStd(stream, m_MovesMastered, 4);
    stream << mStarsEarned;
    stream << mSongPlayed;
    stream << unk25;
}

void CampaignEraSongProgress::LoadFixed(FixedSizeSaveableStream &stream, int) {
    FixedSizeSaveable::LoadStd(stream, m_MovesMastered, 4);
    stream >> mStarsEarned;
    stream >> mSongPlayed;
    stream >> unk25;
    BookmarkCurrentProgress();
}

int CampaignEraSongProgress::SaveSize(int) {
    REPORT_SIZE("CampaignEraSongProgress", 0x1A);
}

bool CampaignEraSongProgress::SetMoveMastered(Symbol era, Symbol song, Symbol move) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x72);
    if (song == pEra->GetDanceCrazeSong()) {
        return false;
    } else {
        bool inserted = false;
        if (m_MovesMastered.find(move) == m_MovesMastered.end()) {
            MILO_ASSERT(m_MovesMastered.size() < kMaxSymbols_CampaignMovesPerSong, 0x7E);
            m_MovesMastered.insert(move);
            inserted = true;
        }
        return inserted;
    }
}

void CampaignEraSongProgress::BookmarkCurrentProgress() {
    mBookmarkedStarsEarned = mStarsEarned;
    mBookmarkedMovesMastered = m_MovesMastered;
    mBookmarkedSongPlayed = mSongPlayed;
}

void CampaignEraSongProgress::ResetProgressToBookmark() {
    mStarsEarned = mBookmarkedStarsEarned;
    m_MovesMastered = mBookmarkedMovesMastered;
    mSongPlayed = mBookmarkedSongPlayed;
}

void CampaignEraSongProgress::Cleanup() {
    m_MovesMastered.clear();
    mStarsEarned = -1;
    mSongPlayed = false;
    BookmarkCurrentProgress();
}

#pragma endregion
#pragma region CampaignEraProgress

CampaignEraProgress::CampaignEraProgress()
    : mEra(gNullStr), mIntroMoviePlayed(false), unkd(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraProgress::CampaignEraProgress(Symbol era)
    : mEra(era), mIntroMoviePlayed(false), unkd(false) {
    mSaveSizeMethod = &SaveSize;
}

CampaignEraProgress::~CampaignEraProgress() { Cleanup(); }

void CampaignEraProgress::SaveFixed(FixedSizeSaveableStream &stream) const {
    FixedSizeSaveable::SaveSymbolID(stream, mEra);
    stream << mIntroMoviePlayed;
    stream << unkd;
    FixedSizeSaveable::SaveStdPtr(
        stream, m_mapCampaignEraSongProgress, 10, CampaignEraSongProgress::SaveSize(92) + 4
    );
}

void CampaignEraProgress::LoadFixed(FixedSizeSaveableStream &stream, int i) {
    FixedSizeSaveable::LoadSymbolFromID(stream, mEra);
    stream >> mIntroMoviePlayed;
    stream >> unkd;
    FixedSizeSaveable::LoadStdPtr(
        stream, m_mapCampaignEraSongProgress, 10, CampaignEraSongProgress::SaveSize(i) + 4
    );
}

int CampaignEraProgress::SaveSize(int i) {
    int savesize = CampaignEraSongProgress::SaveSize(i);
    REPORT_SIZE("CampaignEraSongProgress", (savesize + 5) * 10);
}

int CampaignEraProgress::GetTotalStarsEarned() const {
    int totalStars = 0;
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x106);

    int size = pEra->GetNumSongs();
    for (int i = 0; i < size; i++) {
        Symbol name = pEra->GetSongName(i);
        int total = 0;
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(name);
        if (pEraSongProgress) {
            total = pEraSongProgress->GetStarsEarned();
        }
        total = Max(total, 0);
        if (total >= 5) {
            total = 5;
        }
        totalStars += total;
    }
    return totalStars;
}

int CampaignEraProgress::GetTotalMovesMastered() const {
    int total = 0;
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x124);
    int size = pEra->GetNumSongs();
    for (int i = 0; i < size; i++) {
        Symbol songName = pEra->GetSongName(i);
        int x = 0;
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(songName);
        if (pEraSongProgress) {
            x = pEraSongProgress->NumMovesMastered();
        }
        total += x;
    }
    return total;
}

bool CampaignEraProgress::IsMoveMastered(Symbol song, Symbol move) const {
    bool ret = false;
    CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(song);
    if (pEraSongProgress) {
        ret = pEraSongProgress->IsMoveMastered(move);
    }
    return ret;
}

bool CampaignEraProgress::IsMastered() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x155);
    int totalStars = GetTotalStarsEarned();
    int starsRequired = pEra->StarsRequiredForMastery();
    int movesRequired = pEra->MovesRequiredForMastery();
    int totalMasteredMoves = GetTotalMovesMastered();
    return totalStars >= starsRequired && totalMasteredMoves >= movesRequired;
}

bool CampaignEraProgress::IsEraComplete() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x163);
    Symbol song = pEra->GetDanceCrazeSong();
    if (IsMastered()) {
        bool ret;
        CampaignEraSongProgress *progress = GetEraSongProgress(song);
        ret = progress ? progress->IsSongPlayed() : false;
        if (ret) {
            return true;
        }
    }
    return false;
}

bool CampaignEraProgress::IsPlayed() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x16c);
    int size = pEra->GetNumSongs();
    for (int i = 0; i < size; i++) {
        Symbol songName = pEra->GetSongName(i);
        CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(songName);
        if (pEraSongProgress && pEraSongProgress->IsSongPlayed()) {
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

void CampaignEraProgress::SetSongPlayed(Symbol song, bool played) {
    CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(song);
    if (!pEraSongProgress) {
        pEraSongProgress = new CampaignEraSongProgress();
        m_mapCampaignEraSongProgress[song] = pEraSongProgress;
        MILO_ASSERT(m_mapCampaignEraSongProgress.size() <= kMaxCampaignEraSongs, 0xD8);
    }
    pEraSongProgress->SetSongPlayed(played);
}

void CampaignEraProgress::UpdateSong(Symbol song, int stars) {
    CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(song);
    if (!pEraSongProgress) {
        pEraSongProgress = new CampaignEraSongProgress();
        m_mapCampaignEraSongProgress[song] = pEraSongProgress;
        MILO_ASSERT(m_mapCampaignEraSongProgress.size() <= kMaxCampaignEraSongs, 0x190);
    }
    if (stars >= pEraSongProgress->GetStarsEarned()) {
        pEraSongProgress->SetStarsEarned(stars);
    }
}

bool CampaignEraProgress::UpdateSongMoveMastered(Symbol song, Symbol move) {
    CampaignEraSongProgress *pEraSongProgress = GetEraSongProgress(song);
    if (!pEraSongProgress) {
        pEraSongProgress = new CampaignEraSongProgress();
        m_mapCampaignEraSongProgress[song] = pEraSongProgress;
        MILO_ASSERT(m_mapCampaignEraSongProgress.size() <= kMaxCampaignEraSongs, 0x19F);
    }
    return pEraSongProgress->SetMoveMastered(mEra, song, move);
}

CampaignEraSongProgress *CampaignEraProgress::GetEraSongProgress(Symbol song) const {
    auto it = m_mapCampaignEraSongProgress.find(song);
    return (it != m_mapCampaignEraSongProgress.end()) ? it->second : nullptr;
}

void CampaignEraProgress::Cleanup() {
    FOREACH (it, m_mapCampaignEraSongProgress) {
        RELEASE(it->second);
    }
    m_mapCampaignEraSongProgress.clear();
}

#pragma endregion
#pragma region CampaignProgress

CampaignProgress::CampaignProgress()
    : mProfile(nullptr), mIntroCompleted(false), mMindControlCompleted(false),
      mTanBattleCompleted(false), mNum5StarredMQSongs(0) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

CampaignProgress::~CampaignProgress() { Clear(); }

void CampaignProgress::SaveFixed(FixedSizeSaveableStream &stream) const {
    FixedSizeSaveable::SaveStdPtr<CampaignEraProgress>(
        stream, m_mapCampaignEraProgress, 10, CampaignEraProgress::SaveSize(92) + 4
    );
    stream << mIntroCompleted;
    stream << mMindControlCompleted;
    stream << mTanBattleCompleted;
    stream << mNum5StarredMQSongs;
}

void CampaignProgress::LoadFixed(FixedSizeSaveableStream &stream, int i) {
    FixedSizeSaveable::LoadStdPtr<CampaignEraProgress>(
        stream, m_mapCampaignEraProgress, 10, CampaignEraProgress::SaveSize(i) + 4
    );
    stream >> mIntroCompleted;
    stream >> mMindControlCompleted;
    stream >> mTanBattleCompleted;
    stream >> mNum5StarredMQSongs;
}

void CampaignProgress::SetCampaignIntroCompleted(bool complete) {
    mIntroCompleted = complete;
    mProfile->MakeDirty();
}

void CampaignProgress::SetCampaignMindControlCompleted(bool complete) {
    mMindControlCompleted = complete;
    mProfile->MakeDirty();
}

bool CampaignProgress::IsCampaignTanBattleCompleted() const {
    return mTanBattleCompleted;
}

void CampaignProgress::SetCampaignTanBattleCompleted(bool complete) {
    mTanBattleCompleted = complete;
    mProfile->MakeDirty();
}

int CampaignProgress::SaveSize(int ver) {
    REPORT_SIZE("CampaignProgress", CampaignEraProgress::SaveSize(ver) * 10 + 51);
}

int CampaignProgress::GetRequiredStarsForDanceCrazeSong(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x310);
    Symbol song = pEra->GetDanceCrazeSong();
    return pEra->GetSongRequiredStars(song);
}

bool CampaignProgress::IsEraPlayed(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x287);
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        return pEraProgress->IsPlayed();
    }
    return false;
}

bool CampaignProgress::IsEraComplete(Symbol era) const {
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        return pEraProgress->IsEraComplete();
    } else {
        return false;
    }
}

bool CampaignProgress::IsEraMastered(Symbol era) const {
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        return pEraProgress->IsMastered();
    } else {
        return false;
    }
}

int CampaignProgress::GetEraStarsEarned(Symbol era) const {
    int stars = 0;
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        stars = pEraProgress->GetTotalStarsEarned();
    }
    return stars;
}

int CampaignProgress::GetEraMovesMastered(Symbol era) const {
    int master_ct = 0;
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        master_ct = pEraProgress->GetTotalMovesMastered();
    }
    return master_ct; // why
}

bool CampaignProgress::GetEraIntroMoviePlayed(Symbol era) const {
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        return pEraProgress->GetIntroMoviePlayed();
    } else {
        return false;
    }
}

void CampaignProgress::SetEraIntroMoviePlayed(Symbol era, bool played) {
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        pEraProgress->SetIntroMoviePlayed(played);
        mProfile->MakeDirty();
    }
}

bool CampaignProgress::IsEraSongAvailable(Symbol era, Symbol song) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 810);
    Symbol craze = pEra->GetDanceCrazeSong();
    if (song == craze) {
        CampaignEraProgress *pEraProgress = GetEraProgress(era);
        if (pEraProgress) {
            return pEraProgress->IsMastered();
        } else {
            return false;
        }
    } else {
        int iReqStars = pEra->GetSongRequiredStars(song), iStarCount = 0;
        CampaignEraProgress *pEraProgress = GetEraProgress(era);
        if (pEraProgress) {
            iStarCount = pEraProgress->GetTotalStarsEarned();
        }
        return iStarCount >= iReqStars;
    }
}

bool CampaignProgress::IsSongPlayed(Symbol era, Symbol song) const {
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        CampaignEraSongProgress *pEraSongProgress =
            pEraProgress->GetEraSongProgress(song);
        if (pEraSongProgress) {
            return pEraSongProgress->IsSongPlayed();
        }
    }
    return false;
}

int CampaignProgress::GetSongStarsEarned(Symbol era, Symbol song) const {
    int stars = 0;
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        CampaignEraSongProgress *pEraSongProgress =
            pEraProgress->GetEraSongProgress(song);
        if (pEraSongProgress) {
            stars = pEraSongProgress->GetStarsEarned();
        }
        stars = Max(stars, 0);
    }
    return stars;
}

bool CampaignProgress::IsMoveMastered(Symbol era, Symbol song, Symbol move) const {
    bool b1 = false;
    CampaignEraProgress *pEraProgress = GetEraProgress(era);
    if (pEraProgress) {
        b1 = pEraProgress->IsMoveMastered(song, move);
    }
    return b1;
}

CampaignEraProgress *CampaignProgress::GetEraProgress(Symbol era) const {
    auto it = m_mapCampaignEraProgress.find(era);
    return (it != m_mapCampaignEraProgress.end()) ? it->second : nullptr;
}

bool CampaignProgress::IsCampaignNew() const {
    auto &eras = TheCampaign->Eras();
    FOREACH (it, eras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x3BC);
        if (IsEraPlayed(pEra->GetName())) {
            return false;
        }
    }
    return true;
}

Symbol CampaignProgress::GetFirstIncompleteEra() const {
    auto &eras = TheCampaign->Eras();
    Symbol era;
    FOREACH (it, eras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x3E6);
        era = pEra->GetName();
        CampaignEraProgress *progress = GetEraProgress(era);
        bool canContinue;
        if (progress) {
            canContinue = progress->IsEraComplete();
        } else {
            canContinue = false;
        }
        if (!canContinue)
            break;
        if (pEra->GetUnk50()) {
            static Symbol era_tan_battle("era_tan_battle");
            return era_tan_battle;
        }
    }
    return era;
}

int CampaignProgress::GetStars() const {
    auto &eras = TheCampaign->Eras();
    int stars = 0;
    FOREACH (it, eras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x412);
        int i4 = 0;
        CampaignEraProgress *progress = GetEraProgress(pEra->GetName());
        if (progress) {
            i4 = progress->GetTotalStarsEarned();
        }
        stars += i4;
    }
    return stars;
}

int CampaignProgress::GetNumCompletedEras() const {
    auto &eras = TheCampaign->Eras();
    int numCompleted = 0;
    FOREACH (it, eras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x423);
        CampaignEraProgress *progress = GetEraProgress(pEra->GetName());
        bool b4;
        if (progress) {
            b4 = progress->IsEraComplete();
        } else {
            b4 = false;
        }
        if (b4) {
            numCompleted++;
        }
    }
    return numCompleted;
}

void CampaignProgress::Clear() {
    FOREACH (it, m_mapCampaignEraProgress) {
        RELEASE(it->second);
    }
    m_mapCampaignEraProgress.clear();
    mIntroCompleted = false;
    mMindControlCompleted = false;
    mTanBattleCompleted = false;
}

bool CampaignProgress::IsDanceCrazeSongAvailable(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x31B);
    Symbol song = pEra->GetDanceCrazeSong();
    if (song != gNullStr) {
        return IsEraSongAvailable(era, song);
    } else {
        return false;
    }
}

void CampaignProgress::ResetAllProgress() {
    Clear();
    mProfile->MakeDirty();
}

void CampaignProgress::ClearSongProgress(Symbol era, Symbol song) {
    CampaignEraProgress *p = GetEraProgress(era);
    if (p) {
        CampaignEraSongProgress *sp = p->GetEraSongProgress(song);
        if (sp) {
            sp->SetStarsEarned(0);
            sp->ClearMovesMastered();
            sp->SetSongPlayed(false);
        }
    }
    mProfile->MakeDirty();
}

void CampaignProgress::BookmarkCurrentProgress() {
    FOREACH (it, m_mapCampaignEraProgress) {
        it->second->BookmarkCurrentProgress();
    }
}

void CampaignProgress::ResetProgressToBookmark() {
    FOREACH (it, m_mapCampaignEraProgress) {
        it->second->ResetProgressToBookmark();
    }
}

void CampaignProgress::SetSongPlayed(Symbol era, Symbol song, bool played) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x34E);
    CampaignEraProgress *progress = GetEraProgress(era);
    if (!progress) {
        progress = new CampaignEraProgress(era);
        m_mapCampaignEraProgress[era] = progress;
        MILO_ASSERT(m_mapCampaignEraProgress.size() <= kMaxSymbols_CampaignEras, 0x355);
    }
    progress->SetSongPlayed(song, played);
}

void CampaignProgress::UpdateEraSong(Symbol era, Symbol song, int stars) {
    CampaignEraProgress *progress = GetEraProgress(era);
    if (!progress) {
        progress = new CampaignEraProgress(era);
        m_mapCampaignEraProgress[era] = progress;
        MILO_ASSERT(m_mapCampaignEraProgress.size() <= kMaxSymbols_CampaignEras, 0x387);
    }
    progress->UpdateSong(song, stars);
    mProfile->MakeDirty();
}

bool CampaignProgress::UpdateEraSongMoveMastered(Symbol era, Symbol song, Symbol move) {
    static Symbol metamode("metamode");
    static Symbol campaign_holla_back("campaign_holla_back");
    if (TheHamProvider->Property(metamode)->Sym() == campaign_holla_back) {
        return false;
    } else {
        CampaignEraProgress *progress = GetEraProgress(era);
        if (!progress) {
            progress = new CampaignEraProgress(era);
            m_mapCampaignEraProgress[era] = progress;
            MILO_ASSERT(m_mapCampaignEraProgress.size() <= kMaxSymbols_CampaignEras, 0x3AC);
        }
        bool ret = progress->UpdateSongMoveMastered(song, move);
        mProfile->MakeDirty();
        return ret;
    }
}

void CampaignProgress::UnlockAllMoves(Symbol era, Symbol song) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x43A);
    CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(song);
    MILO_ASSERT(pSongEntry, 0x43C);
    if (song != pEra->GetDanceCrazeSong()) {
        int numSongs = pSongEntry->GetNumSongCrazeMoves();
        for (int i = 0; i < numSongs; i++) {
            Symbol move = pSongEntry->GetCrazeMoveHamMoveName(i);
            UpdateEraSongMoveMastered(era, song, move);
        }
    }
    SetSongPlayed(era, song, true);
    mProfile->MakeDirty();
}

#pragma endregion
