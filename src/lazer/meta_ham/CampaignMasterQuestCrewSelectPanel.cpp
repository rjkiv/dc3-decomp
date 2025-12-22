#include "meta_ham/CampaignMasterQuestCrewSelectPanel.h"
#include "TexLoadPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

#pragma region CampaignMqCrewProvider

CampaignMqCrewProvider::CampaignMqCrewProvider() : unk40() {}

Symbol CampaignMqCrewProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0,NumData(), 0x7d);
    return unk44[i_iData];
}

#pragma endregion CampaignMqCrewProvider
#pragma region CampaignMasterQuestCrewSelectPanel

CampaignMasterQuestCrewSelectPanel::CampaignMasterQuestCrewSelectPanel()
    : m_pCampaignMqCrewProvider(), mPreviewDelayFinished(false) {}

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
    m_pCampaignMqCrewProvider->unk40 = LoadedDir();
}

int CampaignMasterQuestCrewSelectPanel::GetTimeSinceEnter() const {
    DateTime dt;
    GetDateAndTime(dt);
    int iNow = dt.ToCode();
    int iEnter = mDateTime.ToCode();
    MILO_ASSERT(iNow >= iEnter, 0xd6);
    return iNow - iEnter;
}

Symbol CampaignMasterQuestCrewSelectPanel::GetSelectedCrew() {
    static Message get_selected_crew_index("get_selected_crew_index");
    DataNode selectedCrewIndexNode = Handle(get_selected_crew_index, true);
    int node = selectedCrewIndexNode.Int();
    if (m_pCampaignMqCrewProvider->NumData() > 0)
        return m_pCampaignMqCrewProvider->DataSymbol(node);
    else
        return "";
}

void CampaignMasterQuestCrewSelectPanel::UpdateCrewMesh(Symbol s) {
    RndMat *pMat =
        DataDir()->Find<RndMat>(FormatString("crew_portrait.mat").Str(), false);
    MILO_ASSERT(pMat, 0xea);
    RndMesh *pMesh =
        DataDir()->Find<RndMesh>(FormatString("crew_portrait.mesh").Str(), false);
    MILO_ASSERT(pMesh, 0xed);
}

void CampaignMasterQuestCrewSelectPanel::Refresh() {
    MILO_ASSERT(m_pCampaignMqCrewProvider, 0xde);
    m_pCampaignMqCrewProvider->UpdateList();
    static Message update_crew_provider("update_crew_provider");
    Handle(update_crew_provider, true);
}

BEGIN_HANDLERS(CampaignMasterQuestCrewSelectPanel)
    HANDLE_EXPR(get_selected_crew, GetSelectedCrew())
    HANDLE_ACTION(update_crew_mesh, UpdateCrewMesh(_msg->Sym(2)))
    HANDLE_EXPR(is_preview_delay_finished, mPreviewDelayFinished)
    HANDLE_ACTION(refresh, Refresh())
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS

#pragma endregion CampaignMasterQuestCrewSelectPanel
