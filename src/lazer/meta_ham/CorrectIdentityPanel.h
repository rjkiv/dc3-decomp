#pragma once
#include "meta_ham/OverlayPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CorrectIdentityPanel : public OverlayPanel, public UIListProvider {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CorrectIdentityPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Dismiss();

    CorrectIdentityPanel();
    void SetAsOverlay();
    bool IdentitySelected(int);

    std::vector<int> mIdentityList; // 0x44

private:
    void UpdateIdentityList();
};
