#pragma once
#include "hamobj/HamNavList.h"
#include "lazer/meta_ham/HamPanel.h"
#include "meta_ham/SaveLoadManager.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Draw.h"
#include "rndobj/Group.h"
#include "ui/UIPanel.h"

class HelpBarPanel : public HamPanel {
public:
    // Hmx::Object
    virtual ~HelpBarPanel();
    OBJ_CLASSNAME(HelpBarPanel)
    OBJ_SET_TYPE(HelpBarPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    NEW_OBJ(HelpBarPanel)

    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    static HelpBarPanel *sInstance;

    HelpBarPanel();
    bool IsAnimating();
    bool UpdateBackButton(UIPanel *);
    bool UpdateTertiaryButton(UIPanel *);
    void EnterControllerMode();
    void ExitControllerMode(bool);
    bool IsWriteIconShowing();
    void SyncToPanel(UIPanel *);
    DataNode OnEnterBlacklightMode(DataArray const *);
    DataNode OnExitBlacklightMode(DataArray const *);

    UIPanel *unk3c;
    RndGroup *unk40;
    bool unk44;
    Timer unk48;
    bool unk78;
    bool unk79;
    bool unk7a;
    bool unk7b;
    bool unk7c;
    Timer unk80;
    HamNavList *unkb0;

private:
    bool ShouldHideHelpbar() const;
    void ShowPhysicalWriteIcon();
    void HidePhysicalWriteIcon();
    void DeactivatePhysicalWriteIcon();
    void ShowWaveGestureIcon();
    void HideWaveGestureIcon();
    void PollSaveDeactivation();
    DataNode OnWaveGestureEnabled(DataArray const *);
    DataNode OnWaveGestureDisabled(DataArray const *);
    DataNode OnMsg(ButtonDownMsg const &);
    DataNode OnMsg(SaveLoadMgrStatusUpdateMsg const &);
};
