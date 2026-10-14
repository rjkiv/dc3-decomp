#pragma once

#include "obj/Object.h"
#include "obj/Task.h"
#include "ui/UIPanel.h"
#include "utl/DeJitter.h"

class DeJitterPanel : public UIPanel {
public:
    DeJitterPanel();
    // Hmx::Object
    virtual ~DeJitterPanel();
    OBJ_CLASSNAME(DeJitterPanel)
    OBJ_SET_TYPE(DeJitterPanel);

    // UIPanel
    virtual void Enter();
    virtual void Poll();

    NEW_OBJ(DeJitterPanel)

private:
    Timer mTimer; // 0x38
    DeJitter mJitter; // 0x68
    bool mFirstPoll; // 0xf8
};

class DeJitterSetter {
public:
    DeJitterSetter(DeJitter &, Timer *);
    ~DeJitterSetter();

    float oldSec; // 0x0
    float oldDt; // 0x4
};
