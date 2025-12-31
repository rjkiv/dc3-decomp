#include "meta_ham/CampaignSongSelectPanel.h"
#include "HamPanel.h"
#include "TexLoadPanel.h"
#include "hamobj/Difficulty.h"
#include "meta/SongMgr.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/CampaignEra.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/CampaignProgress.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamStarsDisplay.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SaveLoadManager.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "ui/UIListCustom.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include <cstring>

#pragma region CampaignSongProvider

CampaignSongProvider::CampaignSongProvider() : unk40(0) {}

void CampaignSongProvider::Custom(
    int, int i_iData, UIListCustom *uiListCustom, Hmx::Object *o
) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x88);
    Symbol dataSym = DataSymbol(i_iData);
    if (uiListCustom->Matches("stars")) {
        HamStarsDisplay *pStarDisplay = dynamic_cast<HamStarsDisplay *>(o);
        MILO_ASSERT(pStarDisplay, 0x97);
        if (!IsSongPlayed(dataSym)) {
            pStarDisplay->SetShowing(false);
        } else {
            pStarDisplay->SetShowing(true);
            pStarDisplay->SetSongCampaign(TheSongMgr.GetSongIDFromShortName(dataSym));
        }
    }
}

int CampaignSongProvider::NumData() const { return unk44.size(); }

bool CampaignSongProvider::IsSongAvailable(Symbol song) const {
    if (MetaPanel::sUnlockAll) {
        return true;
    } else {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0xf2);
        CampaignPerformer *pPerformer =
            dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
        MILO_ASSERT(pPerformer, 0xf4);
        return pPerformer->CanSelectEraSong(song);
    }
}

bool CampaignSongProvider::IsSongPlayed(Symbol song) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0xe1);
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0xe3);
    const CampaignProgress &pProgress =
        pProfile->GetCampaignProgress(pPerformer->GetDifficulty());
    return pProgress.IsSongPlayed(pPerformer->Era(), song);
}

bool CampaignSongProvider::IsCrazeSong(Symbol song) const {
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0xfb);
    CampaignEra *pEra = TheCampaign->GetCampaignEra(pPerformer->Era());
    MILO_ASSERT(pEra, 0xfd);
    return song == pEra->GetDanceCrazeSong();
}

void CampaignSongProvider::UpdateList() {
    MILO_ASSERT(TheCampaign, 0x38);
    unk44.clear();
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0x3d);
    CampaignEra *pEra = TheCampaign->GetCampaignEra(pPerformer->Era());
    MILO_ASSERT(pEra, 0x41);
    for (int i = 0; i < pEra->GetNumSongs(); i++) {
        unk44.push_back(pEra->GetSongName(i));
    }
}

Symbol CampaignSongProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x104);
    return unk44[i_iData];
}

#pragma endregion CampaignSongProvider
#pragma region CampaignSongSelectPanel

CampaignSongSelectPanel::CampaignSongSelectPanel()
    : m_pCampaignSongProvider(), mPreviewDelayFinished(false), m_pCurCampaignEra(),
      m_pCurCampaignProgress() {}

CampaignSongSelectPanel::~CampaignSongSelectPanel() {}

void CampaignSongSelectPanel::Load() {
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0x199);
    unk60 = pPerformer->Era();
    m_pCurCampaignEra = TheCampaign->GetCampaignEra(unk60);
    MILO_ASSERT(m_pCurCampaignEra, 0x19c);
    mDifficulty = pPerformer->GetDifficulty();
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x19f);
    m_pCurCampaignProgress = &pProfile->AccessCampaignProgress(mDifficulty);
    MILO_ASSERT(m_pCurCampaignProgress, 0x1a1);
    TexLoadPanel::Load();
}

void CampaignSongSelectPanel::Enter() {
    GetDateAndTime(unk58);
    mPreviewDelayFinished = false;
    HamPanel::Enter();
}

void CampaignSongSelectPanel::Poll() {
    TexLoadPanel::Poll();
    if (!mPreviewDelayFinished && GetTimeSinceEnter() >= 3)
        mPreviewDelayFinished = true;
}

void CampaignSongSelectPanel::Unload() {
    TexLoadPanel::Unload();
    RELEASE(m_pCampaignSongProvider);
}

void CampaignSongSelectPanel::FinishLoad() {
    TexLoadPanel::FinishLoad();
    MILO_ASSERT(!m_pCampaignSongProvider, 0x190);
    m_pCampaignSongProvider = new CampaignSongProvider();
    m_pCampaignSongProvider->SetPanelDir(LoadedDir());
}

bool CampaignSongSelectPanel::CanSelectSong(int i_iData) {
    MILO_ASSERT(m_pCampaignSongProvider, 0x153);
    Symbol song = gNullStr;
    if (0 < m_pCampaignSongProvider->NumData()) {
        song = m_pCampaignSongProvider->DataSymbol(i_iData);
    }
    if (song != gNullStr)
        return m_pCampaignSongProvider->IsSongAvailable(song);
    else
        return false;
}

int CampaignSongSelectPanel::GetSongIndex(Symbol song) {
    MILO_ASSERT(m_pCampaignSongProvider, 0x184);
    if (0 < m_pCampaignSongProvider->NumData())
        return m_pCampaignSongProvider->SymbolIndex(song);
    else
        return -1;
}

int CampaignSongSelectPanel::GetStarsRequiredForEraSong() const {
    MILO_ASSERT(m_pCurCampaignProgress, 0x1ec);
    return m_pCurCampaignProgress->GetRequiredStarsForDanceCrazeSong(unk60);
}

int CampaignSongSelectPanel::GetTimeSinceEnter() const {
    DateTime dt;
    GetDateAndTime(dt);
    int iNow = dt.ToCode();
    int iEnter = unk58.ToCode();
    MILO_ASSERT(iNow >= iEnter, 0x1d2);
    return iNow - iEnter;
}

void CampaignSongSelectPanel::SelectSong() {
    Symbol selSong = GetSelectedSong();
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0x12d);
    pPerformer->SetSong(selSong);
}

Symbol CampaignSongSelectPanel::GetSelectedSong() {
    if (mState != 1)
        return gNullStr;
    else {
        static Message get_selected_song_index("get_selected_song_index");
        DataNode node = Handle(get_selected_song_index, true);
        int i = node.Int();
        if (m_pCampaignSongProvider->NumData() > 0)
            return m_pCampaignSongProvider->DataSymbol(i);
        else
            return "";
    }
}

void CampaignSongSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignSongProvider, 0x1da);
    m_pCampaignSongProvider->UpdateList();
    static Message update_song_provider("update_song_provider", 0);
    update_song_provider[0] = m_pCampaignSongProvider;
    Handle(update_song_provider, true);
}

Symbol CampaignSongSelectPanel::GetSong(int i_iData) {
    MILO_ASSERT(m_pCampaignSongProvider, 0x179);
    if (m_pCampaignSongProvider->NumData() > 0)
        return m_pCampaignSongProvider->DataSymbol(i_iData);
    return "";
}

int CampaignSongSelectPanel::GetEraStars() const {
    MILO_ASSERT(m_pCurCampaignProgress, 0x1f2);
    int starsEarned = m_pCurCampaignProgress->GetEraStarsEarned(unk60);
    int starsRequired = GetStarsRequiredForEraSong();
    if (starsRequired >= starsEarned)
        return starsEarned;
    return starsRequired;
}

int CampaignSongSelectPanel::GetEraSongStars() const {
    int eraStars = GetEraStars();
    int starsRequired = GetStarsRequiredForEraSong();
    if (eraStars >= starsRequired)
        eraStars = starsRequired;
    return eraStars;
}

bool CampaignSongSelectPanel::IsWaitingForEraSongUnlock() {
    Symbol selectedSong = GetSelectedSong();
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x135);
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0x137);
    Symbol danceCrazeSong = m_pCurCampaignEra->GetDanceCrazeSong();
    if (danceCrazeSong == selectedSong && pPerformer->JustUnlockedEraSong())
        return true;
    return false;
}

bool CampaignSongSelectPanel::CanSelectCurrentSong() {
    Symbol selectedSong = GetSelectedSong();
    if (selectedSong != gNullStr) {
        MILO_ASSERT(m_pCampaignSongProvider, 0x149);
        return m_pCampaignSongProvider->IsSongAvailable(selectedSong);
    } else
        return false;
}

void CampaignSongSelectPanel::CheatTransitionPending() {
    MILO_LOG(
        "CampaignSongSelectPanel::CheatTransitionPending() - handle pending NavList refresh\n"
    );
    m_pCampaignSongProvider->UpdateList();
}

void CampaignSongSelectPanel::CheatWinEraSong(Symbol s, int i) {
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0x20b);
    if (i > 0) {
        pPerformer->UpdateEraSong(pPerformer->GetDifficulty(), unk60, s, i);
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x213);
        SongStatusMgr *pSongStatusMgr = pProfile->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0x215);
        int songID = TheSongMgr.GetSongIDFromShortName(s);
        pSongStatusMgr->UpdateSong(
            songID, 0x29a, 0x457, pPerformer->GetDifficulty(), i, 0, 0, 0, 0, 0, 0, 0
        ); // idk what these vals are
        pPerformer->UnlockAllMoves(unk60, s, i);
    } else {
        pPerformer->ClearSongProgress(unk60, s);
    }

    static Message refresh_flashcard_dock("refresh_flashcard_dock");
    Handle(refresh_flashcard_dock, true);
    static Message update_era_meter("update_era_meter");
    DataNode updateEraMeterNode = Handle(update_era_meter, true);
    if (TheSaveLoadMgr)
        TheSaveLoadMgr->AutoSave();
    Refresh();
}

BEGIN_HANDLERS(CampaignSongSelectPanel)
    HANDLE_EXPR(get_song, GetSong(_msg->Int(2)))
    HANDLE_EXPR(get_song_index, GetSongIndex(_msg->Sym(2)))
    HANDLE_EXPR(get_selected_song, GetSelectedSong())
    HANDLE_EXPR(can_select_current_song, CanSelectCurrentSong())
    HANDLE_EXPR(can_select_song, CanSelectSong(_msg->Int(2)))
    HANDLE_ACTION(select_song, SelectSong())
    HANDLE_EXPR(is_waiting_for_era_song_unlock, IsWaitingForEraSongUnlock())
    HANDLE_EXPR(is_preview_delay_finished, mPreviewDelayFinished)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_EXPR(get_era_stars, GetEraStars())
    HANDLE_EXPR(get_required_erasong_stars, GetStarsRequiredForEraSong())
    HANDLE_EXPR(get_erasong_stars, GetEraSongStars())
    HANDLE_ACTION(cheat_win_era_song, CheatWinEraSong(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(cheat_transition_pending, CheatTransitionPending())
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS

#pragma endregion CampaignSongSelectPanel
