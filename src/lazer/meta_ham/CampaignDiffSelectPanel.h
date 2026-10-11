#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include <vector>

class CampaignDiffProvider : public UIListProvider, public Hmx::Object {
public:
    virtual ~CampaignDiffProvider() {}
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return m_vDiffs.size(); }
    OBJ_CLASSNAME(CampaignDiffProvider)
    OBJ_SET_TYPE(CampaignDiffProvider)
    virtual DataNode Handle(DataArray *, bool);

    NEW_OBJ(CampaignDiffProvider)

    void UpdateList(bool);

private:
    std::vector<int> m_vDiffs; // 0x30
};

class CampaignDiffSelectPanel : public HamPanel {
public:
    CampaignDiffSelectPanel();
    OBJ_CLASSNAME(CampaignDiffSelectPanel)
    OBJ_SET_TYPE(CampaignDiffSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Unload();
    virtual void FinishLoad();

    NEW_OBJ(CampaignDiffSelectPanel)

    Difficulty GetSelectedDiff();
    void SelectDiff();
    void Refresh();
    void CheatWinDiff(int);

private:
    CampaignDiffProvider *m_pCampaignDiffProvider; // 0x3c
};
