#pragma once

#include "SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "stl/_vector.h"
#include "types.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

class PreloadPanel : public UIPanel, public ContentMgr::Callback {
public:
    enum PreloadResult {
        kPreloadInProgress = 0,
        kPreloadSuccess = 1,
        kPreloadFailure = 2
    };

    // Hmx::Object
    virtual ~PreloadPanel();
    OBJ_CLASSNAME(PreloadPanel);
    OBJ_SET_TYPE(PreloadPanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual void Load();
    virtual void SetTypeDef(DataArray *);

    // UIPanel
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void PollForLoading();
    virtual void FinishLoad();

    // ContentMgr::Callback
    virtual void ContentMounted(char const *, char const *);
    virtual void ContentFailed(char const *);

    PreloadPanel();

    PreloadResult unk3c;
    std::vector<String> unk40;
    bool unk4c;
    std::vector<Symbol> unk50;
    Hmx::Object *unk5c;
    bool unk60;
    String unk64;
    u32 unk68;
    bool unk6c;

protected:
    Symbol CurrentSong() const;

private:
    void CheckTypeDef(Symbol);
    bool CheckFileCached(char const *);
    SongMgr *FindSongMgr() const;
    DataNode OnMsg(ContentReadFailureMsg const &);
    DataNode OnMsg(class UITransitionCompleteMsg const &);
    void OnContentMountedOrFailed(char const *);
    void StartCache();
};
