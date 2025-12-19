#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Group.h"
#include "ui/UIPanel.h"

class LetterboxPanel : public HamPanel {
public:
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

    static LetterboxPanel *sInstance;

    LetterboxPanel();
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

    UIPanel *unk3c;
    RndGroup *unk40;
    bool mIsBlacklightMode; // 0x44
    bool unk45;
    Timer unk48;
    int unk78;
    int unk7c;
    float unk80;
    float unk84;
};
