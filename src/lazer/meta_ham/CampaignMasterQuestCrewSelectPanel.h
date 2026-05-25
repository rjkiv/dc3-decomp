#pragma once
#include "TexLoadPanel.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "ui/PanelDir.h"
#include "utl/Symbol.h"

class CampaignMqCrewProvider : public HamNavProvider {
public:
    CampaignMqCrewProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return mMQCrews.size(); }

    void UpdateList();
    void SetPanelDir(PanelDir *p) { mPanelDir = p; }

private:
    PanelDir *mPanelDir; // 0x40
    std::vector<Symbol> mMQCrews; // 0x44
};

class CampaignMasterQuestCrewSelectPanel : public TexLoadPanel {
public:
    CampaignMasterQuestCrewSelectPanel();
    OBJ_CLASSNAME(CampaignMasterQuestCrewSelectPanel)
    OBJ_SET_TYPE(CampaignMasterQuestCrewSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();
    virtual void Load();
    virtual void FinishLoad();

    NEW_OBJ(CampaignMasterQuestCrewSelectPanel)

    int GetTimeSinceEnter() const;
    Symbol GetSelectedCrew();
    void UpdateCrewMesh(Symbol);
    void Refresh();

protected:
    CampaignMqCrewProvider *m_pCampaignMqCrewProvider; // 0x54
    DateTime mDateTime; // 0x58
    bool mPreviewDelayFinished; // 0x5e
};
