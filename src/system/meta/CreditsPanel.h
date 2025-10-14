#pragma once

#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "synth/Stream.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

class CreditsPanel : public UIListProvider, public UIPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CreditsPanel)
    OBJ_SET_TYPE(CreditsPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual int NumData() const;

protected:
    DataNode OnMsg(ButtonDownMsg const &);

private:
    // Hmx::Object
    virtual ~CreditsPanel();

    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual void Exit();
    virtual bool Exiting() const;
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    CreditsPanel();
    void PausePanel(bool);
    void DebugToggleAutoScroll();

    bool mCheatOn;
    DataLoader *mLoader;
    DataArray *mNames;
    UIList *mList;
    Stream *mStream;
    bool mAutoScroll; // 0x50
    float mSavedSpeed; // 0x54
    /** Whether or not the panel is paused. */
    bool mPaused;
};
