#include "meta_ham/CampaignDiffSelectPanel.h"
#include "Campaign.h"
#include "CampaignProgress.h"
#include "HamSongMgr.h"
#include "hamobj/Difficulty.h"
#include "macros.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SaveLoadManager.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIListLabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

#pragma region CampaignDiffProvider

void CampaignDiffProvider::UpdateList(bool b) {
    MILO_ASSERT(TheCampaign, 0x20);
    unk30.clear();
    unk30.push_back(kDifficultyEasy);
    unk30.push_back(kDifficultyMedium);
    unk30.push_back(kDifficultyExpert);
    if (b) {
        unk30.push_back(kDifficultyEasy);
        unk30.push_back(kDifficultyMedium);
        unk30.push_back(kDifficultyExpert);
        unk30.push_back(kDifficultyEasy);
    }
}

Symbol CampaignDiffProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x7c);
    return DifficultyToSym((Difficulty)unk30[i_iData]);
}

void CampaignDiffSelectPanel::Unload() {
    UIPanel::Unload();
    RELEASE(m_pCampaignDiffProvider);
    m_pCampaignDiffProvider = nullptr;
}

void CampaignDiffProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x34);
    Symbol dataSym = DataSymbol(i_iData);
    Difficulty diff = (Difficulty)unk30[i_iData];
    if (uiListLabel->Matches("diff")) {
        uiLabel->SetTextToken(dataSym);
        if (i_iData <= 2)
            return;
        static Symbol tan_easy("tan_easy");
        static Symbol tan_medium("tan_medium");
        static Symbol tan_hard("tan_hard");
        static Symbol mind_control("mind_control");
        switch (i_iData) {
        case 3:
            uiLabel->SetTextToken(tan_easy);
            break;
        case 4:
            uiLabel->SetTextToken(tan_medium);
            break;
        case 5:
            uiLabel->SetTextToken(tan_hard);
            break;
        case 6:
            uiLabel->SetTextToken(mind_control);
            break;
        default:
            return;
        }
    } else if (uiListLabel->Matches("stars")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x51);
        CampaignPerformer *pPerformer =
            dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
        MILO_ASSERT(pPerformer, 0x53);
        const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
        if (!pProgress.IsCampaignIntroCompleted())
            return;
        int stars = 0;
        int maxStars = 0;
        if (pProgress.IsCampaignTanBattleCompleted()) {
            TheHamSongMgr.GetCoreStarsForDifficulty(pProfile, diff, stars, maxStars);
        } else {
            stars = pProgress.GetStars();
            maxStars = TheCampaign->GetMaxStars();
        }
        static Symbol stars_fraction("stars_fraction");
        uiLabel->SetTokenFmt(stars_fraction, stars, maxStars);
    } else if (uiListLabel->Matches("completed")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x6a);
        CampaignPerformer *pPerformer =
            dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
        MILO_ASSERT(pPerformer, 0x6c);
        const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
        if (!pProgress.IsCampaignTanBattleCompleted())
            return;
        uiLabel->SetIcon('M');
        return;
    } else
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
}

BEGIN_HANDLERS(CampaignDiffProvider)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2) != 0))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion CampaignDiffProvider
#pragma region CampaignDiffSelectPanel

CampaignDiffSelectPanel::CampaignDiffSelectPanel() : m_pCampaignDiffProvider(0) {}

void CampaignDiffSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignDiffProvider, 0xca);
    static Message update_diff_provider("update_diff_provider", 0);
    update_diff_provider[0] = m_pCampaignDiffProvider;
    DataNode node = Handle(update_diff_provider, true);
}

void CampaignDiffSelectPanel::FinishLoad() {
    UIPanel::FinishLoad();
    MILO_ASSERT(!m_pCampaignDiffProvider, 0xbd);
    m_pCampaignDiffProvider = new CampaignDiffProvider();
}

void CampaignDiffSelectPanel::SelectDiff() {
    Difficulty diff = GetSelectedDiff();
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0xaf);
    pPerformer->SetDifficulty(diff);
}

Difficulty CampaignDiffSelectPanel::GetSelectedDiff() {
    if (mState == 1) {
        static Message get_selected_diff_index("get_selected_diff_index");
        DataNode node = Handle(get_selected_diff_index, true);
        int i = node.Int();
        int data = m_pCampaignDiffProvider->NumData();
        if (0 < data) {
            Symbol diffSymbol = m_pCampaignDiffProvider->DataSymbol(i);
            return SymToDifficulty(diffSymbol);
        }
    }
    return kDifficultyEasy;
}

void CampaignDiffSelectPanel::CheatWinDiff(int i) {
    Difficulty diff = GetSelectedDiff();
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0xd5);
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0xd7);
    SongStatusMgr *pSongStatusMgr = pProfile->GetSongStatusMgr();
    MILO_ASSERT(pSongStatusMgr, 0xd9);
    const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
    if (i < 1) {
        pPerformer->ResetAllCampaignProgress();
    } else {
        pPerformer->ResetAllCampaignProgress();
        // stuff with m_vEras
    }

    if (TheSaveLoadMgr)
        TheSaveLoadMgr->AutoSave();
    Refresh();
}

BEGIN_HANDLERS(CampaignDiffSelectPanel)
    HANDLE_ACTION(get_selected_diff, GetSelectedDiff())
    HANDLE_ACTION(select_diff, SelectDiff())
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_ACTION(cheat_win_diff, CheatWinDiff(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion CampaignDiffSelectPanel
