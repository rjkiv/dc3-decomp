#pragma once
#include "HamPanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamLabel.h"
#include "meta_ham/MainMenuProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "rndobj/Tex.h"
#include "utl/NetCacheLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

class MainMenuPanel : public HamPanel, public ContentMgr::Callback {
public:
    struct MotdData {
    public:
        MotdData();
        MotdData(const MotdData &);

        Symbol type; // 0x0
        String text; // 0x4
        float length; // 0xc
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
    MainMenuProvider *GetMainMenuProvider();

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

    HamLabel *mMsgLabel; // 0x40
    MainMenuProvider mProvider; // 0x44
    bool mNeedToLoadNetCacheMgr; // 0x80
    bool mNeedToCleanupNetCacheMgr; // 0x81 - not in pdb
    std::list<NetCacheLoader *> mLoaders; // 0x84
    RndTex *mDlcArt; // 0x8c
    RndTex *mUtilityArt; // 0x90 - not in pdb
    bool mIsDlcArtLoaded; // 0x94
    bool mIsUtilityArtLoaded; // 0x95
    bool mIsMiscArtLoaded; // 0x96 - not in pdb
    std::map<Symbol, std::list<String> > mMessages; // 0x98
    bool mMotdUpdatedNeeded; // 0xb0
    std::list<MotdData> mMotdData; // 0xb4
    int mMotdFreq; // 0xbc
    int mCurrMotdFreq; // 0xc0
    int mStatsDisplayLimit; // 0xc4
    int mCurrStatsAccumulated; // 0xc8
    int mCommunityDisplayLimit; // 0xcc
    int mCurrCommunityAccumulated; // 0xd0
    Symbol prevMotd; // 0xd4
    PropertyEventProvider *mSettings; // 0xd8
};
