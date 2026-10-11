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
#include "os/PlatformMgr.h"
#include "synth/MetaMusic.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include <vector>

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

    static bool sUnlockAll;
    static bool sMotdCheat;

    MetaPanel();
    void UnlockClassicOutfit(Symbol);
    void UnlockAll();

protected:
    int PickLoopIndex(int);
    DataNode OnMsg(XMPStateChangedMsg const &);

    std::vector<int> mRecentIndices; // 0x38
    int mCurrentSlot; // 0x44
    MetaMusic *mMusic; // 0x48
    SongPreview mSongPreview; // 0x4c
    bool mXMPPlaying; // 0xdc
    MetaMusicManager *mMetaMusicMgr; // 0xe0
    Campaign *mCampaign; // 0xe4
    HAQManager *mHAQMgr; // 0xe8
};
