#pragma once
#include "game/SongDB.h"
#include "hamobj/HamMaster.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIPanel.h"
#include "utl/BeatMap.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"

class LoadingPanel : public UIPanel {
public:
    // Hmx::Object
    virtual ~LoadingPanel();
    OBJ_CLASSNAME(LoadingPanel)
    OBJ_SET_TYPE(LoadingPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    NEW_OBJ(LoadingPanel)

    LoadingPanel();

    static HamMaster *sLoadingMaster; // DAT_8311A440
    static SongDB *sSongDB; // DAT_8311A444 i think, def a SongDB

    int unk38;
    TempoMap *unk3c;
    BeatMap *unk40;

protected:
    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual bool Exiting();
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();

    char const *GetLoadingScreen(Symbol);
    Symbol ChooseLoadingScreen();
    void PlayLoadingMusic();
};

void ResetLoadingMusic();
