#pragma once
#include "game/SongDB.h"
#include "hamobj/HamMaster.h"
#include "meta/HAQManager.h"
#include "meta/MetaMusicManager.h"
#include "meta/SongPreview.h"
#include "meta_ham/Campaign.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

DECLARE_MESSAGE(XMPStateChangedMsg, "xmp_state_changed") // check type
END_MESSAGE

class MetaPanel : public UIPanel {
public:
    // Hmx::Object
    virtual ~MetaPanel();
    OBJ_CLASSNAME(MetaPanel)
    OBJ_SET_TYPE(MetaPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual void Exit();
    virtual bool Exiting() const;
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    static Hmx::Object *NewObject();
    static DataNode ToggleUnlockAll(DataArray *);
    static DataNode ToggleMotdCheat(DataArray *);
    static void CycleVenuePreference();
    static void Init();

    static SongDB *sSongDB; // DAT_821189a0
    static HamMaster *sHamMaster; // DAT_8311899c
    static bool sUnlockAll;
    static bool sMotdCheat;

    MetaPanel();
    void UnlockClassicOutfit(Symbol);

protected:
    int PickLoopIndex(int);
    DataNode OnMsg(XMPStateChangedMsg const &);

    std::vector<int> unk38;
    int unk44;
    int unk48;
    SongPreview unk4c;
    bool unkdc;
    MetaMusicManager *unke0;
    Campaign *unke4;
    HAQManager *unke8;
};
