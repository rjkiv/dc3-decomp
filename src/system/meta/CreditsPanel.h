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
private:
    CreditsPanel();
    // Hmx::Object
    virtual ~CreditsPanel();

public:
    OBJ_CLASSNAME(CreditsPanel)
    OBJ_SET_TYPE(CreditsPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual int NumData() const;

    NEW_OBJ(CreditsPanel)

protected:
    DataNode OnMsg(const ButtonDownMsg &);

private:
    // UIPanel
    virtual void Load();
    virtual void Enter();
    virtual void Exit();
    virtual bool Exiting() const;
    virtual void Poll();
    virtual bool IsLoaded() const;
    virtual void Unload();
    virtual void FinishLoad();

    void PausePanel(bool);
    void DebugToggleAutoScroll();
    void SetAutoScroll(bool);

    bool mCheatOn; // 0x3c
    DataLoader *mLoader; // 0x40
    DataArray *mNames; // 0x44
    UIList *mList; // 0x48
    Stream *mStream; // 0x4c
    bool mAutoScroll; // 0x50
    float mSavedSpeed; // 0x54
    /** Whether or not the panel is paused. */
    bool mPaused; // 0x58
};
