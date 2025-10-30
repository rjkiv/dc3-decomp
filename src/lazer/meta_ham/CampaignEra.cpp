#include "lazer/meta_ham/CampaignEra.h"
#include "CampaignEra.h"
#include "macros.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region CampaignEraSongEntry

CampaignEraSongEntry::CampaignEraSongEntry(DataArray *d1, DataArray *d2)
    : m_symSong(gNullStr), unk8(gNullStr), unkc(0) {
    Configure(d1, d2);
}

CampaignEraSongEntry::~CampaignEraSongEntry() {}

bool CampaignEraSongEntry::HasCrazeMove(Symbol crazeMove) const {
    for (int i = 0; i < m_vCrazeMoveNames.size(); i++) {
        if (m_vCrazeMoveNames[i] == crazeMove) {
            return true;
        }

        if (m_vCrazeMoveVariantNames[i] == crazeMove) {
            return true;
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
    unkc = pSongEntry->Node(2).Int(pSongEntry);
    Symbol sym_songNameLookup = pSongEntry2->Sym(0);
    MILO_ASSERT(sym_songNameLookup == m_symSong, 0x65);
}

#pragma endregion CampaignEraSongEntry
#pragma region CampaignEra

CampaignEra::CampaignEra(DataArray *d1, DataArray *d2)
    : mEra(gNullStr), mCrew(gNullStr), mVenue(gNullStr),
      mEraSong_Unlocked_Token(gNullStr), mEraSong_Complete_Token(gNullStr),
      mEra_Intro_Movie(gNullStr), unk50(false), mCompletetion_Accomplishment(gNullStr),
      unk58(0), mCraze_Song(gNullStr), unk60(0), unk70(0), unk74(0), unk78(gNullStr) {
    Configure(d1, d2);
}

CampaignEra::~CampaignEra() { Cleanup(); }

Symbol CampaignEra::GetDanceCrazeSong() const { return mCraze_Song; }

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
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (m_vSongs.size()), 0x1bb);
    return m_vSongs[i_iIndex];
}

CampaignEraSongEntry *CampaignEra::GetSongEntry(Symbol song) const {
    auto it = unk8.find(song);
    if (it == unk8.end()) {
        return GetSongEntry(it->second);
    } else
        return nullptr;
}

Symbol CampaignEra::GetSongName(int i_iIndex) const {
    CampaignEraSongEntry *pEntry = GetSongEntry(i_iIndex);
    MILO_ASSERT(pEntry, 0x1cb);
    return pEntry->m_symSong;
}

int CampaignEra::GetSongIndex(Symbol song) const {
    auto it = unk8.find(song);
    return it != unk8.end() ? it->second : -1;
}

int CampaignEra::GetNumSongCrazeMoves(Symbol song) const {
    CampaignEraSongEntry *pEntry = GetSongEntry(song);
    return pEntry != nullptr ? pEntry->m_vCrazeMoveNames.size() : 0;
}

int CampaignEra::GetSongRequiredStars(Symbol song) {
    CampaignEraSongEntry *pEntry = GetSongEntry(song);
    MILO_ASSERT(pEntry, 0x1f6);
    return pEntry->unkc;
}

bool CampaignEra::HasCrazeMove(Symbol song, Symbol crazeMove) const {
    CampaignEraSongEntry *pSongEntry = GetSongEntry(song);
    MILO_ASSERT(pSongEntry, 0x220);
    return pSongEntry->HasCrazeMove(crazeMove);
}

Symbol CampaignEra::GetMoveVariantName(Symbol song, Symbol hamMoveName) const {
    CampaignEraSongEntry *songEntry = GetSongEntry(song);
    if (songEntry == nullptr) {
        MILO_FAIL(
            "Failed to GetSongEntry for song \'%s\' in current era \'%s\'",
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
    return pSongEntry->GetHamMoveNameFromVariant(variant);
}

Symbol CampaignEra::GetIntroMovie() const { return mEra_Intro_Movie; }

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
    i_pConfig->FindData(erasong_unlocked_token, mEraSong_Unlocked_Token);

    static Symbol era_complete_token("era_complete_token");
    i_pConfig->FindData(era_complete_token, mEraSong_Complete_Token);

    static Symbol era_intro_movie("era_intro_movie");
    i_pConfig->FindData(era_intro_movie, mEra_Intro_Movie, false);

    static Symbol completion_accomplishment("completion_accomplishment");
    i_pConfig->FindData(completion_accomplishment, mCompletetion_Accomplishment);

    static Symbol mastery_stars("mastery_stars");
    DataArray *pMasteryArray = i_pConfig->FindArray(mastery_stars, false);
    if (pMasteryArray) {
        MILO_ASSERT(pMasteryArray->Size() >= 5, 0x160);
        mMastery_Stars[0] = pMasteryArray->Sym(2);
        mMastery_Stars[1] = pMasteryArray->Sym(3);
        mMastery_Stars[2] = pMasteryArray->Sym(4);
    }

    MILO_ASSERT(m_vSongs.empty(), 0x168);
    unk70 = 0;
    unk60 = 0;

    static Symbol songs("songs");
    DataArray *pSongArray = i_pConfig->FindArray(songs);
    MILO_ASSERT(pSongArray, 0x16f);
    MILO_ASSERT(pSongArray->Size() > 1, 0x170);

    DataArray *pSongLookupData = d2->FindArray(songs);
    MILO_ASSERT(pSongLookupData, 0x174);
    MILO_ASSERT(pSongLookupData->Size() == pSongArray->Size(), 0x175);

    if (10 < pSongArray->Size()) {
        MILO_NOTIFY("Too many campaign era songs! Era = %s", mEra);
    }
}

#pragma endregion CampaignEra
