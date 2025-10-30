#pragma once
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

    RndAnimatable *unk38;

    HamPanel();
};
