#pragma once

#include "obj/Object.h"
#include "obj/Task.h"
#include "ui/UIPanel.h"
#include "utl/DeJitter.h"

class DeJitterPanel : public UIPanel {
public:
    // Hmx::Object
    virtual ~DeJitterPanel();
    OBJ_CLASSNAME(DeJitterPanel)
    OBJ_SET_TYPE(DeJitterPanel);

    // UIPanel
    virtual void Enter();
    virtual void Poll();

    DeJitterPanel();

    Timer unk38;
    DeJitter unk68;
    bool unkf8;
};

class DeJitterSetter {
public:
    DeJitterSetter(DeJitter &, Timer *);
    ~DeJitterSetter();

    float secs; // 0x0
    float delta_secs; // 0x4
};
