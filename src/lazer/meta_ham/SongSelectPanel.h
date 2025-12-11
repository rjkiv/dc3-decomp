#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "utl/Symbol.h"

class SongSelectPanel : public HamPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(SongSelectPanel)
    OBJ_SET_TYPE(SongSelectPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Load();
    virtual void Exit();
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void FinishLoad();

    SongSelectPanel();
    RndTex *GetTexForCharacter(Symbol);
    DataNode OnEnterBlacklightMode(DataArray const *);
    DataNode OnExitBlacklightMode(DataArray const *);
};
