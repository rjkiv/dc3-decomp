#pragma once
#include "meta_ham/BlacklightPanel.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/OverlayPanel.h"
#include "meta_ham/ShellInput.h"
#include "meta/ConnectionStatusPanel.h"
#include "os/ContentMgr.h"
#include "os/JoypadMsgs.h"
#include "os/PlatformMgr.h"
#include "rndobj/Dir.h"
#include "rndobj/Mesh.h"
#include "rndobj/Overlay.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"

class HamUI : public UIManager {
public:
    HamUI();
    virtual ~HamUI();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void Terminate();
    virtual void Poll();
    virtual void Draw();
    virtual bool IsTimelineResetAllowed() const;

    void ForceLetterboxOff();
    void ForceLetterboxOffImmediate();
    void GotoEventScreen(UIScreen *);

    bool IsBlacklightMode();
    ShellInput *GetShellInput() const { return mShellInput; }
    HelpBarPanel *GetHelpBarPanel() const { return mHelpBar; }
    int Unk108() const { return unk_0x108; }
    UIPanel *EventDialogPanel() const { return mEventDialogPanel; }
    OverlayPanel *GetOverlayPanel() const { return mOverlayPanel; }

    void SetOverlayPanel(OverlayPanel *op) { mOverlayPanel = op; }

protected:
    void AttemptEventTransition();

    DataNode OnMsg(const UITransitionCompleteMsg &);
    DataNode OnMsg(const ContentReadFailureMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const DiskErrorMsg &);
    DataNode OnMsg(const ButtonDownMsg &);
    // DataNode OnMsg(const KinectGuideGestureMsg &);

private:
    bool SetFullScreenDraw(bool);
    Symbol DisplayNextCameraOutput();
    float NextSkeletonDrawRot();
    void ResetSnapshots();
    int TakeSnapshot();
    int NumSnapshots();
    void ApplySnapshotToMesh(int, RndMesh *);
    void InitTextureStore(int);
    void ClearTextureStore();
    int NumStoredTextures() const;
    int StoreTexture(RndTex *);
    void StoreTextureAt(RndTex *, int);
    void StoreTextureClipAt(float, float, float, float, int, int);
    RndTex *GetStoredTexture(int) const;
    void ApplyTextureClip(RndMat *, int) const;
    void StoreColorBufferAt(int);
    void StoreColorBufferClipAt(float, float, float, float, int);
    void StoreDepthBufferAt(int);
    void StoreDepthBufferClipAt(float, float, float, float, int);
    void ReloadStrings();

    HelpBarPanel *mHelpBar; // 0xd8
    u32 mLetterbox; // LetterboxPanel*
    BlacklightPanel *mBlacklight; // 0xe0
    UIPanel *mEventDialogPanel; // 0xe4
    UIPanel *mBackgroundPanel; // 0xe8
    UIPanel *mContentLoadingPanel; // 0xec
    OverlayPanel *mOverlayPanel; // 0xf0
    u32 mGamePanel; // 0xf4 - GamePanel*
    RndDir *unk_0xF8; // 0xf8
    u8 unk_0xFC;
    u8 unk_0xFD;
    UIScreen *mEventScreen; // 0x100
    ShellInput *mShellInput; // 0x104
    s32 unk_0x108; // 0x108
    u32 unk_0x10C;
    bool mFullScreenDrawActive;
    float mSkelRot;
    bool unk_0x118;
    RndOverlay *mUIOverlay; // 0x11c
};

extern HamUI TheHamUI;
