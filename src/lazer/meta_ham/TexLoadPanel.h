#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "synth/MoggClip.h"
#include "ui/UIPanel.h"

class DynamicTex {
public:
    virtual ~DynamicTex();

    DynamicTex(const char *, const char *, bool);

    RndTex *mTex; // 0x4
    String mMatName; // 0x8
    RndMat *mMat; // 0x10
    FileLoader *mLoader; // 0x14
};

class DLCTex : public DynamicTex {
public:
    enum State {
        kStarting = 0x0000,
        kMounting = 0x0001,
        kLoading = 0x0002,
        kLoaded = 0x0003
    };

    void StartLoading();

    Symbol mSong; // 0x18
    DLCTex::State mState; // 0x1c
    RndTex *mFailTex; // 0x20
};

class TexLoadPanel : public HamPanel, public ContentMgr::Callback {
public:
    TexLoadPanel();
    // Hmx::Object
    virtual ~TexLoadPanel();
    OBJ_CLASSNAME(TexLoadPanel);
    OBJ_SET_TYPE(TexLoadPanel);
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Load();
    virtual void Exit();
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    // ContentMgr::Callback
    virtual void ContentMounted(const char *, const char *);
    virtual void ContentFailed(const char *);

    NEW_OBJ(TexLoadPanel)

    void FinalizeTexturesChunk();
    DynamicTex *AddTex(const char *, const char *, bool);
    void LoadMoggClip(char const *);

protected:
    void FinalizeTextures();
    DLCTex *NextDLCTex();
    bool RegisterForContent() const;
    bool TexturesLoaded() const;

    std::vector<DynamicTex *> mTexs; // 0x40
    MoggClip *mMoggClip; // 0x4c
    Fader *mFader; // 0x50
};
