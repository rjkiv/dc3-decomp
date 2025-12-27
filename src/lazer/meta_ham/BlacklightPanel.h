#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"

class BlacklightPanel : public HamPanel {
public:
    // Hmx::Object
    virtual ~BlacklightPanel();
    OBJ_CLASSNAME(BlacklightPanel)
    OBJ_SET_TYPE(BlacklightPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Enter();

    NEW_OBJ(BlacklightPanel)

    BlacklightPanel();
    DataNode OnEnterBlacklightMode(DataArray const *);
    DataNode OnExitBlacklightMode(DataArray const *);

    static BlacklightPanel *sInstance;
};
