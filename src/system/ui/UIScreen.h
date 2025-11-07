#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"

class UIPanel;

struct PanelRef { // taken from rb3
public:
    class UIPanel *mPanel; // 0x0
    bool mActive; // 0x4
    bool mAlwaysLoad; // 0x5
    bool mLoaded; // 0x6

    friend class UIScreen;

    PanelRef() : mLoaded(false) {}

    bool Active() const { return mActive && mLoaded; }
    bool GetActive() { return mActive; }
};

class UIScreen : public Hmx::Object {
public:
    UIScreen();
    OBJ_CLASSNAME(UIScreen);
    OBJ_SET_TYPE(UIScreen);
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetTypeDef(DataArray *);
    virtual void LoadPanels();
    virtual void UnloadPanels();
    virtual bool CheckIsLoaded();
    virtual bool IsLoaded() const;
    virtual void Poll();
    virtual void Draw();
    virtual bool InComponentSelect() const;
    virtual void Enter(UIScreen *);
    virtual bool Entering() const;
    virtual void Exit(UIScreen *);
    virtual bool Exiting() const;
    virtual void Print(TextStream &);

    UIPanel *FocusPanel() const { return mFocusPanel; }

    void SetFocusPanel(UIPanel *);
    void SetShowing(bool);
    bool HasPanel(UIPanel *);
    void ReenterScreen();
    void SetPanelActive(UIPanel *, bool);
    bool AllPanelsDown();
    bool SharesPanels(UIScreen *);

protected:
    static int sMaxScreenId;
    static UIScreen *sUnloadingScreen;

    virtual bool Unloading() const;

    DataNode OnMsg(ButtonDownMsg const &);
    DataNode ForeachPanel(DataArray const *);
    void ReloadStrings();

    std::list<PanelRef> mPanelList; // 0x2c
    UIPanel *mFocusPanel; // 0x34
    DataArray *mBack; // 0x38
    bool mClearVram; // 0x3c
    bool mShowing; // 0x3d
    int mScreenId; // 0x40
};

void EnterGlitchCB(float, void *);
void UnloadGlitchCB(float, void *);

#include "obj/Msg.h"

DECLARE_MESSAGE(UITransitionCompleteMsg, "transition_complete");
UITransitionCompleteMsg(UIScreen *s1, UIScreen *s2) : Message(Type(), s1, s2) {}
UIScreen *GetNewScreen() const { return mData->Obj<UIScreen>(2); }
UIScreen *GetOldScreen() const { return mData->Obj<UIScreen>(3); }
END_MESSAGE
