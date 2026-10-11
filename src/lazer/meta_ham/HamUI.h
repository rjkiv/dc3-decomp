#pragma once
#include "game/GamePanel.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "meta_ham/BlacklightPanel.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/LetterboxPanel.h"
#include "meta_ham/OverlayPanel.h"
#include "meta_ham/ShellInput.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Msg.h"
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
    enum SignoutError {
        kLocalUserSignedOut = 0x0000,
        kProfileOwnerSignedOut = 0x0001
    };

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
    void InitPanels();

    bool IsBlacklightMode();
    ShellInput *GetShellInput() const { return mShellInput; }
    HelpBarPanel *Helpbar() { return mHelpBar; }
    int GetLastButtonDownPadNum() const { return mLastButtonDownPadNum; }
    UIPanel *EventDialog() { return mEventDialog; }
    OverlayPanel *GetOverlayPanel() { return mOverlayPanel; }
    LetterboxPanel *Letterbox() const { return mLetterbox; }
    BlacklightPanel *Blacklight() const { return mBlacklight; }

    void SetOverlayPanel(OverlayPanel *op) { mOverlayPanel = op; }

protected:
    void AttemptEventTranstion();

    DataNode OnMsg(const UITransitionCompleteMsg &);
    DataNode OnMsg(const ContentReadFailureMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const DiskErrorMsg &);
    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnMsg(const KinectGuideGestureMsg &);

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
    void UpdateUIOverlay();
    void DrawDebug();
    bool IsGameActive() const;

    HelpBarPanel *mHelpBar; // 0xd8
    LetterboxPanel *mLetterbox; // 0xdc
    BlacklightPanel *mBlacklight; // 0xe0
    UIPanel *mEventDialog; // 0xe4
    UIPanel *mBackground; // 0xe8
    UIPanel *mContentLoadingPanel; // 0xec
    OverlayPanel *mOverlayPanel; // 0xf0
    GamePanel *mGamePanel; // 0xf4
    RndDir *mAugmentedPhoto; // 0xf8
    bool mTakeSnapshotNextDraw; // 0xfc
    bool mExtraSnapshotDrawForLag; // 0xfd
    UIScreen *mEventScreen; // 0x100
    ShellInput *mShellInput; // 0x104
    int mLastButtonDownPadNum; // 0x108
    LiveCameraInput::BufferType mCameraOutput; // 0x10C
    bool mFullScreenDraw; // 0x110
    float mSkeletonDrawRadians; // 0x114
    bool mButtonSpam; // 0x118
    RndOverlay *mUIOverlay; // 0x11c
};

bool ToggleDrawSkeletons();

extern HamUI TheHamUI;
