#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/PostProc.h"

class OverlayPanel : public HamPanel {
public:
    // Hmx::Object
    virtual ~OverlayPanel();
    OBJ_CLASSNAME(OverlayPanel)
    OBJ_SET_TYPE(OverlayPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Dismiss();

    OverlayPanel();

    RndPostProc *mPostProc; // 0x3c
};
