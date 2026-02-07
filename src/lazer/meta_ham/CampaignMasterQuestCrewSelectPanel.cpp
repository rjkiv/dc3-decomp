#include "meta_ham/CampaignMasterQuestCrewSelectPanel.h"
#include "HamProfile.h"
#include "ProfileMgr.h"
#include "TexLoadPanel.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/HamSongMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "ui/PanelDir.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

#pragma region CampaignMqCrewProvider

CampaignMqCrewProvider::CampaignMqCrewProvider() : mPanelDir(0) {}

void CampaignMqCrewProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x58);
    Symbol crewSym = DataSymbol(i_iData);
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x5c);
    if (uiListLabel->Matches("crew")) {
        uiLabel->SetTextToken(crewSym);
    } else if (uiListLabel->Matches("lock")) {
        uiLabel->SetTextToken(gNullStr);
    } else if (uiListLabel->Matches("stars")) {
        int y = 0;
        int x = 0;
        static DataNode &mq_difficulty = DataVariable("mq_difficulty");
        Difficulty mqDiff = (Difficulty)mq_difficulty.Int();
        TheHamSongMgr.GetCrewStarsForDifficulty(pProfile, crewSym, mqDiff, x, y);
        static Symbol stars_fraction("stars_fraction");
        uiLabel->SetTokenFmt(stars_fraction, x, y);
    } else {
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
    }
}

Symbol CampaignMqCrewProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0,NumData(), 0x7d);
    return mMQCrews[i_iData];
}

void CampaignMqCrewProvider::UpdateList() {
    MILO_ASSERT(TheCampaign, 0x36);
    mMQCrews.clear();
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x3A);
    static DataNode &mq_difficulty = DataVariable("mq_difficulty");
    Difficulty mqDiff = (Difficulty)mq_difficulty.Int();
    DataArray *crewsArr = SystemConfig()->FindArray("selectable_crews", false);
    if (crewsArr) {
        for (int i = 1; i < crewsArr->Size(); i++) {
            Symbol curSym = crewsArr->Sym(i);
            int y = 0;
            int x = 0;
            TheHamSongMgr.GetCrewStarsForDifficulty(pProfile, curSym, mqDiff, x, y);
            TheCampaign->SetUnkC0At(curSym, y - x == 0);
            if (y) {
                mMQCrews.push_back(curSym);
            }
        }
    }
}

#pragma endregion
#pragma region CampaignMasterQuestCrewSelectPanel

CampaignMasterQuestCrewSelectPanel::CampaignMasterQuestCrewSelectPanel()
    : m_pCampaignMqCrewProvider(), mPreviewDelayFinished(false) {}

BEGIN_HANDLERS(CampaignMasterQuestCrewSelectPanel)
    HANDLE_EXPR(get_selected_crew, GetSelectedCrew())
    HANDLE_ACTION(update_crew_mesh, UpdateCrewMesh(_msg->Sym(2)))
    HANDLE_EXPR(is_preview_delay_finished, mPreviewDelayFinished)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS

void CampaignMasterQuestCrewSelectPanel::Poll() {
    TexLoadPanel::Poll();
    if (!mPreviewDelayFinished && 3 <= GetTimeSinceEnter())
        mPreviewDelayFinished = true;
}

void CampaignMasterQuestCrewSelectPanel::Load() { TexLoadPanel::Load(); }

void CampaignMasterQuestCrewSelectPanel::FinishLoad() {
    TexLoadPanel::FinishLoad();
    MILO_ASSERT(!m_pCampaignMqCrewProvider, 0x94);
    m_pCampaignMqCrewProvider = new CampaignMqCrewProvider();
    m_pCampaignMqCrewProvider->SetPanelDir(LoadedDir());
}

int CampaignMasterQuestCrewSelectPanel::GetTimeSinceEnter() const {
    DateTime dt;
    GetDateAndTime(dt);
    unsigned int iNow = dt.ToCode();
    unsigned int iEnter = mDateTime.ToCode();
    MILO_ASSERT(iNow >= iEnter, 0xd6);
    return iNow - iEnter;
}

Symbol CampaignMasterQuestCrewSelectPanel::GetSelectedCrew() {
    static Message cGetSelectedCrewMsg("get_selected_crew_index");
    DataNode selectedCrewIndexNode = Handle(cGetSelectedCrewMsg, true);
    int node = selectedCrewIndexNode.Int();
    if (m_pCampaignMqCrewProvider->NumData() > 0)
        return m_pCampaignMqCrewProvider->DataSymbol(node);

    return "";
}

void CampaignMasterQuestCrewSelectPanel::UpdateCrewMesh(Symbol s) {
    String matName = MakeString("crew_portrait.mat");
    RndMat *pMat = LoadedDir()->Find<RndMat>(matName.c_str(), false);
    MILO_ASSERT(pMat, 0xea);
    String meshName = MakeString("crew_portrait.mesh");
    RndMesh *pMesh = LoadedDir()->Find<RndMesh>(meshName.c_str(), false);
    MILO_ASSERT(pMesh, 0xed);
    String texName;
    texName = MakeString("%s.tex", s.Str());
    RndTex *tex = LoadedDir()->Find<RndTex>(texName.c_str(), false);
    if (tex) {
        pMat->SetDiffuseTex(tex);
        pMesh->SetMat(pMat);
    }
}

void CampaignMasterQuestCrewSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignMqCrewProvider, 0xde);
    m_pCampaignMqCrewProvider->UpdateList();
    static Message cUpdateProviderMsg("update_crew_provider", 0);
    cUpdateProviderMsg[0] = m_pCampaignMqCrewProvider;
    Handle(cUpdateProviderMsg, true);
}

#pragma endregion
