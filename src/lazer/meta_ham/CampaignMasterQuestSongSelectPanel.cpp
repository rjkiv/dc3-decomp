#include "meta_ham/CampaignMasterQuestSongSelectPanel.h"
#include "MQSongSortMgr.h"
#include "TexLoadPanel.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

CampaignMasterQuestSongSelectPanel::CampaignMasterQuestSongSelectPanel()
    : m_pCampaignSongProvider(), unk5e(false), unk60() {}

CampaignMasterQuestSongSelectPanel::~CampaignMasterQuestSongSelectPanel() {}

void CampaignMasterQuestSongSelectPanel::Poll() {
    TexLoadPanel::Poll();
    if (!unk5e && 3 <= GetTimeSinceEnter())
        unk5e = true;
}

void CampaignMasterQuestSongSelectPanel::Unload() { TexLoadPanel::Unload(); }

bool CampaignMasterQuestSongSelectPanel::CanSelectSong(int i) {
    MILO_ASSERT(m_pCampaignSongProvider, 0xb2);
    Symbol s = gNullStr;
    if (0 < m_pCampaignSongProvider->NumData()) {
        s = m_pCampaignSongProvider->DataSymbol(i);
    }
    return (s == gNullStr) == 0;
}

int CampaignMasterQuestSongSelectPanel::GetTimeSinceEnter() const {
    DateTime dt;
    GetDateAndTime(dt);
    int iNow = dt.ToCode();
    int iEnter = mDateTime.ToCode();
    MILO_ASSERT(iNow >= iEnter, 0x133);
    return iNow - iEnter;
}

Symbol CampaignMasterQuestSongSelectPanel::GetSong(int i) {
    MILO_ASSERT(m_pCampaignSongProvider, 0xd6);
    if (m_pCampaignSongProvider->NumData() > 0)
        return m_pCampaignSongProvider->DataSymbol(i);
    else
        return gNullStr;
}

Symbol CampaignMasterQuestSongSelectPanel::GetSelectedSong() {
    if (mState != 1) {
        return gNullStr;
    } else {
        static Symbol focus_song_index("focus_song_index");
        return GetSong(Property(focus_song_index, true)->Int());
    }
}

BEGIN_HANDLERS(CampaignMasterQuestSongSelectPanel)
    HANDLE_EXPR(get_song, GetSong(_msg->Int(2)))
    HANDLE_EXPR(get_selected_song, GetSelectedSong())
    HANDLE_EXPR(can_select_current_song, CanSelectCurrentSong())
    HANDLE_EXPR(can_select_song, CanSelectSong(_msg->Int(2)))
    HANDLE_ACTION(select_song, MetaPerformer::Current()->SetSong(GetSelectedSong()))
    HANDLE_EXPR(is_preview_delay_finished, unk5e)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_EXPR(is_current_selection_character, IsCurrentSelectionSong() == 0)
    HANDLE_EXPR(is_current_selection_song, IsCurrentSelectionSong())
    HANDLE_ACTION(on_highlight_song, OnHighlightSong())
    HANDLE_ACTION(on_highlight_header, OnHighlightHeader())
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS
