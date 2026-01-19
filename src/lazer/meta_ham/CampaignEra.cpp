#include "lazer/meta_ham/CampaignEra.h"
#include "CampaignEra.h"
#include "macros.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <cstring>

#pragma region CampaignEraSongEntry

CampaignEraSongEntry::CampaignEraSongEntry(DataArray *d1, DataArray *d2)
    : m_symSong(gNullStr), unk8(gNullStr), m_iRequiredStars(0) {
    Configure(d1, d2);
}

CampaignEraSongEntry::~CampaignEraSongEntry() {}

bool CampaignEraSongEntry::HasCrazeMove(Symbol crazeMove) const {
    for (int i = 0; i < m_vCrazeMoveNames.size(); i++) {
        Symbol curHamMove = m_vCrazeMoveHamMoveNames[i];
        if (curHamMove == crazeMove) {
            return true;
        }
        String curVariant = m_vCrazeMoveVariantNames[i];
        if (curVariant == crazeMove) {
            return true;
        }
        char buffer[256];
        strcpy(buffer, curHamMove.Str());
        int bufLen = strlen(buffer) - 5;
        if (bufLen >= 0) {
            buffer[bufLen] = '\0';
            if (strstr(crazeMove.Str(), buffer) == crazeMove.Str()
                && crazeMove.Str()[bufLen] == '_' && crazeMove.Str()[bufLen + 2] == '_') {
                return true;
            }
        }
    }
    return false;
}

Symbol CampaignEraSongEntry::GetCrazeMoveName(int i_index) const {
    MILO_ASSERT(i_index>=0 && i_index<m_vCrazeMoveNames.size(), 0xc4);
    return m_vCrazeMoveNames[i_index];
}

Symbol CampaignEraSongEntry::GetCrazeMoveVariantName(int i_index) const {
    MILO_ASSERT(i_index>=0 && i_index<m_vCrazeMoveVariantNames.size(), 0xca);
    return m_vCrazeMoveVariantNames[i_index];
}

Symbol CampaignEraSongEntry::GetCrazeMoveHamMoveName(int i_index) const {
    MILO_ASSERT(i_index>=0 && i_index<m_vCrazeMoveHamMoveNames.size(), 0xd0);
    return m_vCrazeMoveHamMoveNames[i_index];
}

Symbol CampaignEraSongEntry::GetVariantFromHamMoveName(Symbol hamMoveName) const {
    for (int i = 0; i < m_vCrazeMoveNames.size(); i++) {
        if (hamMoveName == m_vCrazeMoveHamMoveNames[i]
            || hamMoveName == m_vCrazeMoveVariantNames[i]) {
            return m_vCrazeMoveVariantNames[i];
        }
    }
    return gNullStr;
}

Symbol CampaignEraSongEntry::GetHamMoveNameFromVariant(Symbol variant) const {
    for (int i = 0; i < m_vCrazeMoveVariantNames.size(); i++) {
        if (variant == m_vCrazeMoveVariantNames[i]) {
            return m_vCrazeMoveHamMoveNames[i];
        }
    }
    return gNullStr;
}

void CampaignEraSongEntry::Configure(DataArray *pSongEntry, DataArray *pSongEntry2) {
    MILO_ASSERT(pSongEntry, 0x59);
    MILO_ASSERT(m_vCrazeMoveNames.empty(), 0x5a);
    MILO_ASSERT(m_vCrazeMoveVariantNames.empty(), 0x5b);
    MILO_ASSERT(m_vCrazeMoveHamMoveNames.empty(), 0x5c);
    MILO_ASSERT(pSongEntry->Size() >= 4, 0x5d);

    m_symSong = pSongEntry->Sym(0);
    unk8 = pSongEntry->Sym(1);
    m_iRequiredStars = pSongEntry->Int(2);
    Symbol sym_songNameLookup = pSongEntry2->Sym(0);
    MILO_ASSERT(sym_songNameLookup == m_symSong, 0x65);
    DataArray *songEntryArray2 = pSongEntry2->Array(1);
    Symbol s1 = gNullStr;
    Symbol s2 = gNullStr;
    Symbol s3 = gNullStr;
    DataArray *songEntryArray = pSongEntry->Array(3);
    for (int i = 0; i < songEntryArray->Size(); i++) {
        s1 = songEntryArray->Sym(i);
        for (int j = 0; j < songEntryArray2->Size(); j++) {
            DataArray *curArr2 = songEntryArray2->Array(j);
            Symbol curSym1 = curArr2->Sym(0);
            Symbol curSym2 = curArr2->Sym(1);
            Symbol curSym3 = curArr2->Sym(2);
            if (curSym1 == s1) {
                s2 = curSym2;
                s3 = curSym3;
                break;
            }
        }
        m_vCrazeMoveNames.push_back(s1);
        m_vCrazeMoveVariantNames.push_back(s2);
        m_vCrazeMoveHamMoveNames.push_back(s3);
    }
}

#pragma endregion
#pragma region CampaignEra

CampaignEra::CampaignEra(DataArray *d1, DataArray *d2)
    : mEra(gNullStr), mCrew(gNullStr), mVenue(gNullStr), mEraSongUnlockedToken(gNullStr),
      mEraSongCompleteToken(gNullStr), mEraIntroMovie(gNullStr), unk50(false),
      mCompletionAccomplishment(gNullStr), unk58(0), mCrazeSong(gNullStr),
      mStarsRequiredForMastery(0), mMovesRequiredForMastery(0),
      mStarsRequiredForOutfits(0), mOutfitAward(gNullStr) {
    Configure(d1, d2);
}

CampaignEra::~CampaignEra() { Cleanup(); }

Symbol CampaignEra::GetDanceCrazeSong() const { return mCrazeSong; }

bool CampaignEra::IsTanBattleEra() const {
    static Symbol era_tan_battle("era_tan_battle");
    return mEra == era_tan_battle;
}

int CampaignEra::GetMaxStars() const {
    int size = m_vSongs.size();
    int maxStars = 0;
    if (size > 0) {
        maxStars = size * 5;
    }
    return maxStars;
}

CampaignEraSongEntry *CampaignEra::GetSongEntry(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vSongs.size(), 0x1BB);
    return m_vSongs[i_iIndex];
}

CampaignEraSongEntry *CampaignEra::GetSongEntry(Symbol song) const {
    auto it = unk8.find(song);
    if (it != unk8.end()) {
        return GetSongEntry(it->second);
    } else
        return nullptr;
}

Symbol CampaignEra::GetSongName(int i_iIndex) const {
    CampaignEraSongEntry *pEntry = GetSongEntry(i_iIndex);
    MILO_ASSERT(pEntry, 0x1cb);
    return pEntry->GetSongName();
}

int CampaignEra::GetSongIndex(Symbol song) const {
    auto it = unk8.find(song);
    return it != unk8.end() ? it->second : -1;
}

int CampaignEra::GetNumSongCrazeMoves(Symbol song) const {
    CampaignEraSongEntry *pEntry = GetSongEntry(song);
    return pEntry != nullptr ? pEntry->GetNumSongCrazeMoves() : 0;
}

int CampaignEra::GetSongRequiredStars(Symbol song) {
    CampaignEraSongEntry *pEntry = GetSongEntry(song);
    MILO_ASSERT(pEntry, 0x1f6);
    return pEntry->GetSongRequiredStars();
}

bool CampaignEra::HasCrazeMove(Symbol song, Symbol crazeMove) const {
    CampaignEraSongEntry *pSongEntry = GetSongEntry(song);
    MILO_ASSERT(pSongEntry, 0x220);
    if (pSongEntry)
        return pSongEntry->HasCrazeMove(crazeMove);
    else
        return false;
}

Symbol CampaignEra::GetMoveVariantName(Symbol song, Symbol hamMoveName) const {
    CampaignEraSongEntry *songEntry = GetSongEntry(song);
    if (songEntry == nullptr) {
        MILO_FAIL(
            "Failed to GetSongEntry for song '%s' in current era '%s'",
            song.Str(),
            mEra.Str()
        );
        return gNullStr;
    }
    return songEntry->GetVariantFromHamMoveName(hamMoveName);
}

Symbol CampaignEra::GetHamMoveNameFromVariant(Symbol song, Symbol variant) const {
    CampaignEraSongEntry *pSongEntry = GetSongEntry(song);
    MILO_ASSERT(pSongEntry, 600);
    if (pSongEntry)
        return pSongEntry->GetHamMoveNameFromVariant(variant);
    else
        return gNullStr;
}

Symbol CampaignEra::GetIntroMovie() const { return mEraIntroMovie; }

void CampaignEra::Cleanup() {
    FOREACH (it, m_vSongs) {
        RELEASE(*it);
    }
    m_vSongs.clear();
    unk8.clear();
}

void CampaignEra::Configure(DataArray *i_pConfig, DataArray *d2) {
    MILO_ASSERT(i_pConfig, 0x13d);
    mEra = i_pConfig->Sym(0);
    static Symbol crew("crew");
    i_pConfig->FindData(crew, mCrew, false);
    static Symbol venue("venue");
    i_pConfig->FindData(venue, mVenue, false);
    static Symbol erasong_unlocked_token("erasong_unlocked_token");
    i_pConfig->FindData(erasong_unlocked_token, mEraSongUnlockedToken);
    static Symbol era_complete_token("era_complete_token");
    i_pConfig->FindData(era_complete_token, mEraSongCompleteToken);
    static Symbol era_intro_movie("era_intro_movie");
    i_pConfig->FindData(era_intro_movie, mEraIntroMovie, false);
    static Symbol completion_accomplishment("completion_accomplishment");
    i_pConfig->FindData(completion_accomplishment, mCompletionAccomplishment);
    static Symbol mastery_stars("mastery_stars");
    DataArray *pMasteryArray = i_pConfig->FindArray(mastery_stars, false);
    if (pMasteryArray) {
        MILO_ASSERT(pMasteryArray->Size() >= 5, 0x160);
        mMasteryStars[0] = pMasteryArray->Sym(2);
        mMasteryStars[1] = pMasteryArray->Sym(3);
        mMasteryStars[2] = pMasteryArray->Sym(4);
    }
    MILO_ASSERT(m_vSongs.empty(), 0x168);
    mMovesRequiredForMastery = 0;
    mStarsRequiredForMastery = 0;
    static Symbol songs("songs");
    DataArray *pSongArray = i_pConfig->FindArray(songs);
    MILO_ASSERT(pSongArray, 0x16f);
    MILO_ASSERT(pSongArray->Size() > 1, 0x170);
    DataArray *pSongLookupData = d2->FindArray(songs);
    MILO_ASSERT(pSongLookupData, 0x174);
    MILO_ASSERT(pSongLookupData->Size() == pSongArray->Size(), 0x175);
    int numSongs = pSongArray->Size() - 1;
    if (numSongs > 10) {
        MILO_NOTIFY("Too many campaign era songs! Era = %s", mEra.Str());
        numSongs = 10;
    }
    for (int i = 0; i < numSongs; i++) {
        DataArray *pSongEntryArray = pSongArray->Array(i + 1);
        DataArray *pSongLookupDataArray = pSongLookupData->Array(i + 1);
        CampaignEraSongEntry *pSongEntry =
            new CampaignEraSongEntry(pSongEntryArray, pSongLookupDataArray);
        m_vSongs.push_back(pSongEntry);
        unk8[pSongEntry->GetSongName()] = m_vSongs.size() - 1;
        if (pSongEntry->GetSongRequiredStars() != 0) {
            mCrazeSong = pSongEntry->GetSongName();
            mStarsRequiredForMastery = pSongEntry->GetSongRequiredStars();
        } else {
            mMovesRequiredForMastery += pSongEntry->GetNumSongCrazeMoves();
        }
    }
    if (mCrazeSong == gNullStr) {
        mCrazeSong = GetSongName(numSongs - 1);
    }
}

#pragma endregion
