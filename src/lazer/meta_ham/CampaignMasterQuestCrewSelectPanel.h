#pragma once
#include "TexLoadPanel.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "utl/Symbol.h"

class CampaignMqCrewProvider : public HamNavProvider {
public:
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;

    CampaignMqCrewProvider();
    void UpdateList();

    PanelDir *unk40;
    std::vector<Symbol> unk44;
};

class CampaignMasterQuestCrewSelectPanel : public TexLoadPanel {
public:
    OBJ_CLASSNAME(CampaignMasterQuestCrewSelectPanel)
    OBJ_SET_TYPE(CampaignMasterQuestCrewSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();
    virtual void Load();
    virtual void FinishLoad();

    CampaignMasterQuestCrewSelectPanel();
    int GetTimeSinceEnter() const;
    Symbol GetSelectedCrew();
    void UpdateCrewMesh(Symbol);
    void Refresh();

protected:
    CampaignMqCrewProvider *m_pCampaignMqCrewProvider; // 0x54
    DateTime mDateTime; // 0x58
    bool mPreviewDelayFinished; // 0x5e
};
