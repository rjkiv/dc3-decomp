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
    HelpBarPanel();
    // Hmx::Object
    virtual ~HelpBarPanel();
    OBJ_CLASSNAME(HelpBarPanel)
    OBJ_SET_TYPE(HelpBarPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // UIPanel
    virtual void Draw();
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    static HelpBarPanel *sInstance;
    NEW_OBJ(HelpBarPanel)

    bool IsAnimating();
    bool UpdateBackButton(UIPanel *);
    bool UpdateTertiaryButton(UIPanel *);
    void EnterControllerMode();
    void ExitControllerMode(bool);
    bool IsWriteIconShowing();
    bool IsWriteIconUp() const;
    void SyncToPanel(UIPanel *);
    void SetTertiaryLabels(DataArray *);

    bool IsControllerModeAllowed() const { return mControllerModeAllowed; }

    DataNode OnEnterBlacklightMode(const DataArray *);
    DataNode OnExitBlacklightMode(const DataArray *);

private:
    bool ShouldHideHelpbar() const;
    void ShowPhysicalWriteIcon();
    void HidePhysicalWriteIcon();
    void DeactivatePhysicalWriteIcon();
    void ShowWaveGestureIcon();
    void HideWaveGestureIcon();
    void PollSaveDeactivation();
    DataNode OnWaveGestureEnabled(const DataArray *);
    DataNode OnWaveGestureDisabled(const DataArray *);
    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnMsg(const SaveLoadMgrStatusUpdateMsg &);

    HamNavList *mNavList; // 0x3c
    RndGroup *mHelpbarGroup; // 0x40
    bool mIsSaveDeactivationQueued; // 0x44
    Timer mSaveTimerActive; // 0x48
    bool mNoButtons; // 0x78
    bool mControllerModeAllowed; // 0x79
    bool mIsPhysicalWriteActive; // 0x7a
    bool mIsWriteIconUp; // 0x7b
    bool mIsWaveGestureIconUp; // 0x7c
    Timer mTimerWrite; // 0x80
    UIPanel *mSyncedPanel; // 0xb0
};
