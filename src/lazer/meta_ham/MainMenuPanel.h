#pragma once
#include "HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamLabel.h"
#include "meta_ham/MainMenuProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "rndobj/Tex.h"
#include "stl/_map.h"
#include "stl/_pair.h"
#include "stl/_vector.h"
#include "utl/NetCacheLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

class MainMenuPanel : public HamPanel, public ContentMgr::Callback {
public:
    struct MotdData {
    public:
        MotdData();
        MotdData(MotdData const &);

        Symbol unk0;
        String unk4;
        float unkc;
    };

    // Hmx::Object
    virtual ~MainMenuPanel();
    OBJ_CLASSNAME(MainMenuPanel)
    OBJ_SET_TYPE(MainMenuPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual void Exit();
    virtual bool Unloading() const;
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    // ContentMgr::Callback
    virtual void ContentDone();

    NEW_OBJ(MainMenuPanel)

    MainMenuPanel();
    MainMenuProvider GetMainMenuProvider() const { return unk44; }

protected:
    HamLabel *mMsgLabel; // 0x40
    MainMenuProvider unk44;
    bool unk80;
    bool unk81;
    std::list<NetCacheLoader *> unk84;
    RndTex *unk8c;
    RndTex *unk90;
    bool unk94;
    bool unk95;
    bool unk96;
    std::map<Symbol, std::list<String> > unk98;
    bool unkb0;
    std::list<MotdData> unkb4;
    int unkbc;
    int unkc0;
    int unkc4;
    int unkc8;
    int unkcc;
    int unkd0;
    Symbol unkd4;
    PropertyEventProvider *unkd8;

private:
    void DeleteDownloadedArts();
    void DownloadMotdArt();
    void HandleNetCacheMgrFailure();
    void HandleNetCacheLoaderFailure(int);
    void UpdateIconState(Symbol);
    void CleanupNetCacheRelated();
    void MotdHandleTextScrolledIn(int);
    void LoadArt(String);
    void UpdateArtLoaders();
    float MotdPickNextText();
    void MotdHandleTextScrolledOut(int);
    void MotdInitializeTexts();
    void MotdSetup(HamLabel *);
};
