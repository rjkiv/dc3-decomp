#pragma once
#include "hamobj/HamNavList.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "ui/UIComponent.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

class HamPanel : public UIPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(HamPanel);
    OBJ_SET_TYPE(HamPanel);
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual bool Exiting() const;
    virtual void Poll();
    virtual UIComponent *FocusComponent();

    // HamPanel
    virtual bool ShouldUseLocalNavlist() const { return true; }
    virtual bool HasNavList() const { return mNavList != nullptr; }

    NEW_OBJ(HamPanel)

    HamNavList *mNavList;

    HamPanel();
};
