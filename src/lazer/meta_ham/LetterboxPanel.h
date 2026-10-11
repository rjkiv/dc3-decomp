#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Group.h"
#include "ui/UIPanel.h"

enum letterboxState {
    kLetterBox_Off = 0x0000,
    kLetterBox_Activate = 0x0001,
    kLetterBox_Active = 0x0002,
    kLetterBox_PulseLeft = 0x0003,
    kLetterBox_PulseTop = 0x0004,
    kLetterBox_Hold = 0x0005,
    kLetterBox_Deactivate = 0x0006
};

class LetterboxPanel : public HamPanel {
public:
    LetterboxPanel();
    // Hmx::Object
    virtual ~LetterboxPanel();
    OBJ_CLASSNAME(LetterboxPanel)
    OBJ_SET_TYPE(LetterboxPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Poll();
    virtual void Unload();

    NEW_OBJ(LetterboxPanel)

    static LetterboxPanel *sInstance;

    bool ShouldHideLetterbox() const;
    bool ShouldShowHandHelp() const;
    bool IsBlacklightMode();
    bool IsLeavingBlacklightMode();
    bool IsEnteringBlacklightMode();
    bool InBlacklightTransition();
    void VoiceInput(int, bool);
    void SyncToPanel(UIPanel *);
    void EnterBlacklightMode();
    void ExitBlacklightMode(bool);
    void SetBlacklightMode(bool);
    void SetBlacklightModeImmediately(bool);
    void ToggleBlacklightMode(bool);

private:
    UIPanel *mSyncedPanel; // 0x3c
    RndGroup *mLetterboxGroup; // 0x40
    bool mIsBlacklightMode; // 0x44
    bool mIsHeartbeatActive; // 0x45
    Timer mBlackLightTimer; // 0x48
    int mBlacklightActiveDurationMs; // 0x78
    letterboxState mLetterboxState; // 0x7c
    float mStateDuration; // 0x80
};
