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
    OBJ_CLASSNAME(CreditsPanel)
    OBJ_SET_TYPE(CreditsPanel)
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual DataNode Handle(DataArray *, bool);

protected:
    DataNode OnMsg(ButtonDownMsg const &);

private:
    CreditsPanel();
    ~CreditsPanel();
    virtual bool IsLoaded() const;
    virtual void Exit();
    virtual void Enter();
    void PausePanel(bool);
    void DebugToggleAutoScroll();
    virtual void Load();
    virtual bool Exiting() const;
    virtual void FinishLoad();
    virtual void Unload();
    virtual void Poll();

    bool mCheatOn;

    DataLoader *mLoader;
    DataArray *mNames;
    UIList *mList;
    Stream *mStream;
    bool mAutoScroll; // 0x50
    float mSavedSpeed; // 0x54
    /** Whether or not the panel is paused. */
    bool mPaused; // 0x58
};
