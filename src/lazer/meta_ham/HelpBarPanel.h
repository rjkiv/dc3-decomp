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

    bool GetUnk7a() const { return unk7a; }

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

    HamNavList *mLeftHandNavList; // 0x3c
    RndGroup *mAll; // 0x40
    bool unk44;
    Timer unk48;
    bool unk78;
    bool mAllowController; // 0x79
    bool unk7a;
    bool unk7b;
    bool mWaveGestureEnabled; // 0x7c
    Timer unk80;
    UIPanel *unkb0;
};
