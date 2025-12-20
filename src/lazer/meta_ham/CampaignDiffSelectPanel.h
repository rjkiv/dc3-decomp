#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"

class CampaignDiffProvider : public UIListProvider, public Hmx::Object {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual Symbol DataSymbol(int) const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

    CampaignDiffProvider();
    void UpdateList(bool);

    std::vector<int> unk30;
};

class CampaignDiffSelectPanel : public HamPanel {
public:
    OBJ_CLASSNAME(CampaignDiffSelectPanel)
    OBJ_SET_TYPE(CampaignDiffSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Unload();
    virtual void FinishLoad();

    CampaignDiffSelectPanel();
    Difficulty GetSelectedDiff();
    void SelectDiff();
    void Refresh();
    void CheatWinDiff(int);

    CampaignDiffProvider *m_pCampaignDiffProvider; // 0x3c
};
