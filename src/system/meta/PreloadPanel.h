#pragma once
#include "SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/FileCache.h"
#include "ui/UIPanel.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"
#include <vector>

class PreloadPanel : public UIPanel, public ContentMgr::Callback {
public:
    enum PreloadResult {
        kPreloadInProgress = 0,
        kPreloadSuccess = 1,
        kPreloadFailure = 2
    };

    PreloadPanel();
    // Hmx::Object
    virtual ~PreloadPanel();
    OBJ_CLASSNAME(PreloadPanel);
    OBJ_SET_TYPE(PreloadPanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetTypeDef(DataArray *);

    // UIPanel
    virtual void Load();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void PollForLoading();
    virtual void FinishLoad();

    // ContentMgr::Callback
    virtual void ContentMounted(char const *, char const *);
    virtual void ContentFailed(char const *);

    NEW_OBJ(PreloadPanel)
    static FileCache *sCache;

protected:
    Symbol CurrentSong() const;

    PreloadResult mPreloadResult; // 0x3c

private:
    void CheckTypeDef(Symbol);
    bool CheckFileCached(char const *);
    SongMgr *FindSongMgr() const;
    DataNode OnMsg(const ContentReadFailureMsg &);
    DataNode OnMsg(const UITransitionCompleteMsg &);
    void OnContentMountedOrFailed(char const *);
    void StartCache();

    std::vector<String> mPreloadedFiles; // 0x40
    bool mMounted; // 0x4c
    std::vector<Symbol> mContentNames; // 0x50
    Hmx::Object *mAppReadFailureHandler; // 0x5c
    bool mContentReadFailureMsgCorrupt; // 0x60
    String mContentReadFailureMsgPackage; // 0x64
    bool mFailedToFindSong; // 0x6c
    int mMaxCacheSize; // 0x70
};
