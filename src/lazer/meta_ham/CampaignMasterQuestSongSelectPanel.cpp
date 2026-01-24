#include "meta_ham/CampaignMasterQuestSongSelectPanel.h"
#include "TexLoadPanel.h"
#include "hamobj/HamNavList.h"
#include "macros.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MQSongSortMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include <cstdio>

void CampaignMasterQuestSongSelectPanel::Impl::Init(
    CampaignMasterQuestSongSelectPanel *panel
) {
    ObjectDir *dir = panel->DataDir();
    mContextualTitleLabel = dir->Find<UILabel>("contextual_title.lbl");
    mContextualInstructionsLabel = dir->Find<UILabel>("contextual_instructions.lbl");
    mContextualStarsLabel = dir->Find<UILabel>("contextual_stars.lbl");
}

CampaignMasterQuestSongSelectPanel::CampaignMasterQuestSongSelectPanel()
    : m_pCampaignSongProvider(), mPreviewDelayFinished(false), mImpl(0) {}

CampaignMasterQuestSongSelectPanel::~CampaignMasterQuestSongSelectPanel() {
    RELEASE(mImpl);
}

BEGIN_HANDLERS(CampaignMasterQuestSongSelectPanel)
    HANDLE_EXPR(get_song, GetSong(_msg->Int(2)))
    HANDLE_EXPR(get_selected_song, GetSelectedSong())
    HANDLE_EXPR(can_select_current_song, CanSelectCurrentSong())
    HANDLE_EXPR(can_select_song, CanSelectSong(_msg->Int(2)))
    HANDLE_ACTION(select_song, SelectSong())
    HANDLE_EXPR(is_preview_delay_finished, mPreviewDelayFinished)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_EXPR(is_current_selection_character, IsCurrentSelectionSong() == 0)
    HANDLE_EXPR(is_current_selection_song, IsCurrentSelectionSong())
    HANDLE_ACTION(on_highlight_song, OnHighlightSong())
    HANDLE_ACTION(on_highlight_header, OnHighlightHeader())
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS

void CampaignMasterQuestSongSelectPanel::Enter() {
    if (!mImpl) {
        mImpl = new Impl();
    }
    mImpl->Init(this);
    GetDateAndTime(mEnterTime);
    mPreviewDelayFinished = false;
    HamPanel::Enter();
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (pProfile) {
        Symbol crew = TheCampaign->GetMQCrew();
        static DataNode &mq_difficulty = DataVariable("mq_difficulty");
        Difficulty mqDiff = (Difficulty)mq_difficulty.Int();
        int y, x;
        TheHamSongMgr.GetCrewStarsForDifficulty(pProfile, crew, mqDiff, x, y);
        char buffer[64];
        sprintf(buffer, x == y ? "QR_%s.tex" : "crewLogo_%s.tex", crew.Str());
        RndMat *logo = LoadedDir()->Find<RndMat>("logo.mat");
        RndTex *tex = LoadedDir()->Find<RndTex>(buffer);
        logo->SetDiffuseTex(tex);
    }
}

void CampaignMasterQuestSongSelectPanel::Poll() {
    TexLoadPanel::Poll();
    if (!mPreviewDelayFinished && 3 <= GetTimeSinceEnter())
        mPreviewDelayFinished = true;
}

void CampaignMasterQuestSongSelectPanel::Unload() { TexLoadPanel::Unload(); }

void CampaignMasterQuestSongSelectPanel::FinishLoad() {
    TexLoadPanel::FinishLoad();
    m_pCampaignSongProvider = TheMQSongSortMgr;
}

bool CampaignMasterQuestSongSelectPanel::CanSelectSong(int i) {
    MILO_ASSERT(m_pCampaignSongProvider, 0xb2);
    Symbol s = gNullStr;
    if (0 < m_pCampaignSongProvider->NumData()) {
        s = m_pCampaignSongProvider->DataSymbol(i);
    }
    return s != gNullStr;
}

int CampaignMasterQuestSongSelectPanel::GetTimeSinceEnter() const {
    DateTime dt;
    GetDateAndTime(dt);
    unsigned int iNow = dt.ToCode();
    unsigned int iEnter = mEnterTime.ToCode();
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
    if (GetState() != kUp) {
        return gNullStr;
    } else {
        static Symbol focus_song_index("focus_song_index");
        return GetSong(Property(focus_song_index, true)->Int());
    }
}

void CampaignMasterQuestSongSelectPanel::SelectSong() {
    Symbol getSelSong = GetSelectedSong();
    MetaPerformer::Current()->SetSong(getSelSong);
}

void CampaignMasterQuestSongSelectPanel::OnHighlightSong() {
    Symbol song = GetSelectedSong();
    int songID = TheHamSongMgr.GetSongIDFromShortName(song);
    Symbol defaultChar = TheHamSongMgr.Data(songID)->DefaultCharacter();
    Symbol token = MakeString("%s%s", defaultChar.Str(), "_title");
    mImpl->mContextualTitleLabel->SetTextToken(token);
    mImpl->mContextualInstructionsLabel->SetPrelocalizedString(String());
    static DataNode &mq_difficulty = DataVariable("mq_difficulty");
    Difficulty mqDiff = (Difficulty)mq_difficulty.Int();
    bool bref;
    int stars =
        TheProfileMgr.GetActiveProfile(true)->GetSongStatusMgr()->GetStarsForDifficulty(
            songID, mqDiff, bref
        );
    if (stars >= 5) {
        stars = 5;
    }
    mImpl->mContextualStarsLabel->SetPrelocalizedString(
        String(MakeString("%d / %d", stars, 5))
    );
}

bool CampaignMasterQuestSongSelectPanel::CanSelectCurrentSong() {
    Symbol song = GetSelectedSong();
    if (song != gNullStr) {
        MILO_ASSERT(m_pCampaignSongProvider, 0x94);
        return m_pCampaignSongProvider->IsSong(song);
    } else {
        return false;
    }
}

bool CampaignMasterQuestSongSelectPanel::IsCurrentSelectionSong() {
    bool ret = false;
    Symbol song = GetSelectedSong();
    if (song != gNullStr) {
        MILO_ASSERT(m_pCampaignSongProvider, 0xA3);
        ret = m_pCampaignSongProvider->IsSong(song);
    }
    return ret;
}

void CampaignMasterQuestSongSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignSongProvider, 0x13B);
    m_pCampaignSongProvider->OnEnter();
    static Message cUpdateProviderMsg("update_song_provider", 0);
    cUpdateProviderMsg[0] = m_pCampaignSongProvider;
    Handle(cUpdateProviderMsg, true);
    HamNavList *list = DataDir()->Find<HamNavList>("right_hand.hnl", false);
    SetNavList(list);
    list->ClearBigElements();
    FOREACH (it, m_pCampaignSongProvider->GetUnk90()) {
        if (m_pCampaignSongProvider->IsCharacter(*it)) {
            mNavList->PushBackBigElement(*it);
        }
    }
}
