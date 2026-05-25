#pragma once
#include "meta_ham/OverlayPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CorrectIdentityPanel : public OverlayPanel, public UIListProvider {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CorrectIdentityPanel)
    OBJ_SET_TYPE(CorrectIdentityPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const { return mIdentityList.size(); }
    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Dismiss();

    void SetAsOverlay();
    bool IdentitySelected(int);

    NEW_OBJ(CorrectIdentityPanel)

private:
    void UpdateIdentityList();

    std::vector<int> mIdentityList; // 0x44
};
