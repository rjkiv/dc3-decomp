#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "ui/UIPanel.h"

class ContentLoadingPanel : public UIPanel, public ContentMgr::Callback {
public:
    // Hmx::Object
    virtual ~ContentLoadingPanel();
    OBJ_CLASSNAME(ContentLoadingPanel)
    OBJ_SET_TYPE(ContentLoadingPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Poll();

    // ContentMgr::Callback
    virtual void ContentMountBegun(int);
    virtual void ContentDone();

    ContentLoadingPanel();
    void AllowedToShow(bool);

    bool unk3c;
    int unk40;
    int unk44;

private:
    void ShowIfPossible();
};
