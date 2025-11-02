#include "lazer/meta_ham/Campaign.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "lazer/meta_ham/CampaignEra.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Loader.h"
#include "utl/SongInfoCopy.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Campaign *TheCampaign;
DataArray *s_pReloadedCampaignData;

Campaign::Campaign(DataArray *d)
    : unk30(state1), unk34(false), unk9c(false), m_pCurLoader(0), unkbc(gNullStr),
      unkd8(0) {
    MILO_ASSERT(!TheCampaign, 0x41);
    TheCampaign = this;
    SetName("campaign", ObjectDir::Main());
    ConfigureCampaignData(d);
}

Campaign::~Campaign() {
    TheCampaign = nullptr;
    if (s_pReloadedCampaignData) {
        s_pReloadedCampaignData->Release();
    }
    Cleanup();
    RELEASE(unkd8);
}

void Campaign::FinishLoading(Loader *l) {
    DirLoader *dl = dynamic_cast<DirLoader *>(l);
    MILO_ASSERT(dl == m_pCurLoader, 0x315);
    MILO_ASSERT(m_pCurLoader->IsLoaded(), 0x316);
    unkd8 = m_pCurLoader->GetDir();
    FOREACH (it, unka8) {
    }
    RELEASE(m_pCurLoader);
}

void Campaign::FailedLoading(Loader *l) {
    DirLoader *dl = dynamic_cast<DirLoader *>(l);
    MILO_ASSERT(dl == m_pCurLoader, 0x337);
    RELEASE(m_pCurLoader);
    MILO_NOTIFY("Loading Campaign Move Data: move data not found");
    FOREACH (it, unka8) {
        CampaignMove *move = *it;
        move->unkc = 2;
        move->unk10 = 0;
    }
}

BEGIN_PROPSYNCS(Campaign)
    SYNC_PROP(work_it_active, unk34);
    SYNC_SUPERCLASS(Hmx::Object);
END_PROPSYNCS

Symbol Campaign::GetIntroVenue() const { return mIntroVenue; }

Symbol Campaign::GetIntroCrew() const { return mIntroCrew; }

void Campaign::SetCurState(CampaignState state) { unk30 = state; }

int Campaign::GetMaxStars() const {
    int stars = 0;
    FOREACH (it, m_vEras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x236);
        stars += pEra->GetMaxStars();
    }
    return stars;
}

int Campaign::GetNumIntroSongs() const { return m_vIntroSongs.size(); }

int Campaign::GetNumOutroSongs() const { return m_vOutroSongs.size(); }

void Campaign::ResetOutroStarsEarnedStartingAtIndex(int iIndex) {
    for (int i = iIndex; i < m_vOutroSongs.size(); i++) {
        m_vOutroSongs[i]->mStarsEarned = 0;
    }
}

Symbol Campaign::GetCampaignWinInstructions(int iIndex) const {
    iIndex--;
    MILO_ASSERT((0) <= (iIndex) && (iIndex) < (m_vInstructions.size()), 0x248);
    return unk68[iIndex];
}

Symbol Campaign::GetIntroSong(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x259);
    return m_vIntroSongs[i_iIndex]->mIntroSong;
}

Symbol Campaign::GetIntroSongCharacter(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x25f);
    return m_vIntroSongs[i_iIndex]->mCharacter;
}

int Campaign::GetIntroSongStarsRequired(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x265);
    return m_vIntroSongs[i_iIndex]->mStarsRequired;
}

Symbol Campaign::GetOutroSong(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x279);
    return m_vOutroSongs[i_iIndex]->mOutroSong;
}

Symbol Campaign::GetOutroSongCharacter(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x27f);
    return m_vOutroSongs[i_iIndex]->mCharacter;
}

int Campaign::GetOutroSongStarsRequired(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x285);
    return m_vOutroSongs[i_iIndex]->mStarsRequired;
}

Symbol Campaign::GetOutroSongGameplayMode(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x28b);
    return m_vOutroSongs[i_iIndex]->mGameplayMode;
}

int Campaign::GetOutroSongFailRestartIndex(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x291);
    return m_vOutroSongs[i_iIndex]->mFailRestartIndex;
}

bool Campaign::GetOutroSongRehearseAllowed(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x297);
    return m_vOutroSongs[i_iIndex]->mRehearseAllowed;
}

bool Campaign::GetOutroSongShortened(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x29d);
    return m_vOutroSongs[i_iIndex]->mSongShortened;
}

bool Campaign::GetOutroSongFreestyleEnabled(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x2a3);
    return m_vOutroSongs[i_iIndex]->mFreestyleEnabled;
}

int Campaign::GetOutroStarsEarned(int iIndex) const {
    if (0 <= iIndex && iIndex < m_vOutroSongs.size())
        return m_vOutroSongs[iIndex]->mStarsEarned;
    return 0;
}

void Campaign::SetOutroStarsEarned(int i1, int i2) {
    if (i1 < 0)
        return;

    if (i1 >= m_vOutroSongs.size()) {
        return;
    }

    if (m_vOutroSongs[i1]->mStarsEarned >= i2) {
        return;
    }

    m_vOutroSongs[i1]->mStarsEarned = i2;
}

CampaignEra *Campaign::GetCampaignEra(Symbol s) const {
    auto it = unk38.find(s);
    if (it != unk38.end()) {
        return GetCampaignEra(it->second);
    }
    return nullptr;
}

bool Campaign::UpdateEraSongUnlockInstructions(Symbol, HamLabel *) { return false; }

void Campaign::LoadCampaignDanceMoves(Symbol) {}

void Campaign::CheatReloadData() {}

CampaignEra *Campaign::GetCampaignEra(int i_iIndex) const {
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (m_vEras.size()), 0x106);
    return m_vEras[i_iIndex];
}

void Campaign::LoadHamMoves(Symbol s) {
    MILO_ASSERT(m_pCurLoader==NULL, 0x347);
    unkb8 = false;
    RELEASE(unkd8);
}

HamMove *Campaign::GetHamMove(Symbol, int) { return nullptr; }

Symbol Campaign::GetMoveName(Symbol s, int i) { return 0; }

void Campaign::GatherMoveData(Symbol sym) {
    CampaignEra *pEra = GetCampaignEra(sym);
    MILO_ASSERT(pEra, 0x2f0);
    Symbol danceCrazeSong = pEra->GetDanceCrazeSong();

    for (int i = 0; i < pEra->m_vSongs.size(); i++) {
        CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(i);
        MILO_ASSERT(pSongEntry, 0x2f8);
        if (pSongEntry->m_symSong != danceCrazeSong) {
            for (int j = 0; j < pSongEntry->m_vCrazeMoveNames.size(); i++) {
            }
        }
    }
}

void Campaign::Cleanup() {
    FOREACH (it, m_vEras) {
        RELEASE(*it);
    }

    FOREACH (it, m_vIntroSongs) {
        RELEASE(*it);
    }

    FOREACH (it, m_vOutroSongs) {
        RELEASE(*it);
    }
}

void Campaign::ConfigureCampaignData(DataArray *i_pConfig) {
    static Symbol venue("venue");
    static Symbol crew("crew");
    static Symbol songs("songs");

    mIntroVenue = venue;
    mIntroCrew = crew;

    static Symbol campaign_intro("campaign_intro");
    DataArray *pIntroArray = i_pConfig->FindArray(campaign_intro);
    MILO_ASSERT(pIntroArray, 0x94);

    pIntroArray->FindData(venue, mIntroVenue, false);
    pIntroArray->FindData(crew, mIntroCrew, false);

    DataArray *pSongArray = i_pConfig->FindArray(songs);
    MILO_ASSERT(pSongArray, 0x99);
    MILO_ASSERT(pSongArray->Size(), 0x9a);

    for (int i = 0; i < pSongArray->Size(); i++) {
    }

    static Symbol eras("eras");
    DataArray *pEraArray = i_pConfig->FindArray(eras);
    MILO_ASSERT(pEraArray, 0xc6);

    int iNumEras = pEraArray->Size();
    if (10 < pEraArray->Size()) {
        MILO_FAIL("Too many campaign eras! ");
        iNumEras = 10;
    }

    static Symbol era_move_names_lookup("era_move_names_lookup");
    DataArray *pLookupEraArray = i_pConfig->FindArray(era_move_names_lookup);
    MILO_ASSERT(pLookupEraArray, 0xd1);
    MILO_ASSERT(iNumEras == pLookupEraArray->Size(), 0xd2);

    for (int i = 0; i < iNumEras; i++) {
        DataArray *d1 = pLookupEraArray->Node(i).Array(pLookupEraArray);
        DataArray *d2 = pEraArray->Node(i).Array(pEraArray);
        CampaignEra *pCampaignEra = new CampaignEra(d1, d2);
        MILO_ASSERT(pCampaignEra, 0xd8);
    }

    static Symbol instructions("instructions");
    DataArray *pInstructionArray = i_pConfig->FindArray(instructions);
    MILO_ASSERT(pInstructionArray, 0xf0);
    for (int i = 1; i < pInstructionArray->Size(); i++) {
        Symbol s = pInstructionArray->Sym(i);
        m_vInstructions.push_back(s);
    }

    static Symbol win_instructions("win_instructions");
    DataArray *pWinInstructionArray = i_pConfig->FindArray(win_instructions);
    MILO_ASSERT(pWinInstructionArray, 0xfb);
    for (int i = 1; i < pWinInstructionArray->Size(); i++) {
        Symbol s = pWinInstructionArray->Sym(i);
        unk68.push_back(s);
    }
}

BEGIN_HANDLERS(Campaign)
END_HANDLERS
