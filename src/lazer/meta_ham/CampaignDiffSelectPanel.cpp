#include "meta_ham/CampaignDiffSelectPanel.h"
#include "Campaign.h"
#include "CampaignProgress.h"
#include "HamSongMgr.h"
#include "hamobj/Difficulty.h"
#include "macros.h"
#include "meta/SongMgr.h"
#include "meta_ham/CampaignEra.h"
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
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region CampaignDiffProvider

void CampaignDiffProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x34);
    Symbol diffSym = DataSymbol(i_iData);
    Difficulty diff = (Difficulty)mDifficulties[i_iData];
    if (uiListLabel->Matches("diff")) {
        uiLabel->SetTextToken(diffSym);
        if (i_iData > 2) {
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
                break;
            }
        }
    } else if (uiListLabel->Matches("stars")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x51);
        CampaignPerformer *pPerformer =
            dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
        MILO_ASSERT(pPerformer, 0x53);
        const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
        if (pProgress.IsCampaignIntroCompleted()) {
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
        }
    } else if (uiListLabel->Matches("completed")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x6a);
        CampaignPerformer *pPerformer =
            dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
        MILO_ASSERT(pPerformer, 0x6c);
        const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
        if (pProgress.IsCampaignTanBattleCompleted()) {
            uiLabel->SetIcon('M');
        }
    } else
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
}

Symbol CampaignDiffProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x7c);
    return DifficultyToSym((Difficulty)mDifficulties[i_iData]);
}

BEGIN_HANDLERS(CampaignDiffProvider)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2) != 0))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void CampaignDiffProvider::UpdateList(bool b) {
    MILO_ASSERT(TheCampaign, 0x20);
    mDifficulties.clear();
    mDifficulties.push_back(kDifficultyEasy);
    mDifficulties.push_back(kDifficultyMedium);
    mDifficulties.push_back(kDifficultyExpert);
    if (b) {
        mDifficulties.push_back(kDifficultyEasy);
        mDifficulties.push_back(kDifficultyMedium);
        mDifficulties.push_back(kDifficultyExpert);
        mDifficulties.push_back(kDifficultyEasy);
    }
}

#pragma endregion
#pragma region CampaignDiffSelectPanel

CampaignDiffSelectPanel::CampaignDiffSelectPanel() : m_pCampaignDiffProvider(0) {}

BEGIN_HANDLERS(CampaignDiffSelectPanel)
    HANDLE_EXPR(get_selected_diff, GetSelectedDiff())
    HANDLE_ACTION(select_diff, SelectDiff())
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_ACTION(cheat_win_diff, CheatWinDiff(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

void CampaignDiffSelectPanel::Unload() {
    UIPanel::Unload();
    RELEASE(m_pCampaignDiffProvider);
    m_pCampaignDiffProvider = nullptr;
}

void CampaignDiffSelectPanel::FinishLoad() {
    UIPanel::FinishLoad();
    MILO_ASSERT(!m_pCampaignDiffProvider, 0xbd);
    m_pCampaignDiffProvider = new CampaignDiffProvider();
}

void CampaignDiffSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignDiffProvider, 0xca);
    static Message update_diff_provider("update_diff_provider", 0);
    update_diff_provider[0] = m_pCampaignDiffProvider;
    Handle(update_diff_provider, true);
}

void CampaignDiffSelectPanel::SelectDiff() {
    Difficulty diff = GetSelectedDiff();
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    MILO_ASSERT(pPerformer, 0xaf);
    pPerformer->SetDifficulty(diff);
}

Difficulty CampaignDiffSelectPanel::GetSelectedDiff() {
    if (mState == kUp) {
        static Message cGetSelectedDiffMsg("get_selected_diff_index");
        DataNode node = Handle(cGetSelectedDiffMsg, true);
        int i = node.Int() % kNumDifficultiesDC2;
        if (0 < m_pCampaignDiffProvider->NumData()) {
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
    CampaignProgress &pProgress = pProfile->AccessCampaignProgress(diff);
    if (i > 0) {
        pPerformer->ResetAllCampaignProgress();
        int count = 0;
        FOREACH (it, TheCampaign->Eras()) {
            count++;
            if (i <= count) {
                CampaignEra *pEra = *it;
                MILO_ASSERT(pEra, 0xe6);
                for (int j = 0; j < pEra->GetNumSongs(); j++) {
                    Symbol songName = pEra->GetSongName(i);
                    pPerformer->UpdateEraSong(diff, pEra->GetName(), songName, 5);
                    int songID = TheSongMgr.GetSongIDFromShortName(songName);
                    pSongStatusMgr->UpdateSong(
                        songID, 0x29a, 0x457, diff, 1, 5, 6, 0, 0, 0, 0, 1
                    ); // idk the values
                    pPerformer->UnlockAllMoves(pEra->GetName(), songName, 5);
                }
            }
        }
        pProgress.SetCampaignIntroCompleted(true);
        if (6 <= i) {
            pProgress.SetCampaignTanBattleCompleted(true);
        }
    } else {
        pPerformer->ResetAllCampaignProgress();
    }
    pProfile->MakeDirty();
    if (TheSaveLoadMgr)
        TheSaveLoadMgr->AutoSave();
    Refresh();
}

#pragma endregion
