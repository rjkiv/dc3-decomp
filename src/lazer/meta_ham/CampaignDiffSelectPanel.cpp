#include "meta_ham/CampaignDiffSelectPanel.h"
#include "Campaign.h"
#include "hamobj/Difficulty.h"
#include "macros.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
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

BEGIN_HANDLERS(CampaignDiffProvider)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2) != 0))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion CampaignDiffProvider
#pragma region CampaignDiffSelectPanel

CampaignDiffSelectPanel::CampaignDiffSelectPanel() : m_pCampaignDiffProvider(0) {}

void CampaignDiffSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignDiffProvider, 0xca);
    static Message update_diff_provider("update_diff_provider");
    Property(update_diff_provider, true);
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
        if (0 < i) {
            Symbol diffSymbol = m_pCampaignDiffProvider->DataSymbol(i);
            return SymToDifficulty(diffSymbol);
        }
    }
    return kDifficultyEasy;
}

BEGIN_HANDLERS(CampaignDiffSelectPanel)
    HANDLE_ACTION(get_selected_diff, GetSelectedDiff())
    HANDLE_ACTION(select_diff, SelectDiff())
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_ACTION(cheat_win_diff, CheatWinDiff(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion CampaignDiffSelectPanel
