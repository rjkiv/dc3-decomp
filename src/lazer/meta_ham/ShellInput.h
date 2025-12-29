#pragma once
#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/HandsUpGestureFilter.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonExtentTracker.h"
#include "gesture/SpeechMgr.h"
#include "meta_ham/DepthBuffer.h"
#include "meta_ham/SkeletonChooser.h"
#include "meta_ham/SkeletonIdentifier.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Anim.h"
#include "types.h"
#include "ui/UIPanel.h"

DECLARE_MESSAGE(LeftHandListEngagementMsg, "left_hand_list_engagement")
END_MESSAGE

DECLARE_MESSAGE(ResetControllerModeTimeoutMsg, "reset_controller_mode_timeout")
END_MESSAGE

class ShellInput : public Hmx::Object, public SkeletonCallback {
public:
    ShellInput();
    // Hmx::Object
    virtual ~ShellInput();
    virtual DataNode Handle(DataArray *, bool);
    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    void Init();
    void Poll();
    void Draw();

    int CycleDrawCursor();
    void ExitControllerMode(bool);
    void SyncToCurrentScreen();
    void UpdateInputPanel(UIPanel *);
    bool IsGameplayPanel() const;
    bool HasSkeleton() const;
    int NumTrackedSkeletons() const;
    void EnterControllerMode(bool);

    SkeletonChooser *GetSkeletonChooser() { return mSkelChooser; }

    bool mVoiceControlEnabled; // 0x30
    bool unk_0x31, unk_0x32;
    ObjPtr<RndAnimatable> unk_0x34; // 0x34
    DoubleExponentialSmoother unk_0x48; // 0x48
    int unk5c; // 0x5c
    u32 unk_0x60, unk_0x64;
    Timer unk_0x68;
    float unk_0x98;
    float unk_0x9C;
    float unk_0xA0;
    u8 unk_0xA4;
    ObjPtr<RndAnimatable> mWrongHandPosAnim; // 0xa8
    UIPanel *mInputPanel; // 0xbc
    UIPanel *mCursorPanel; // 0xc0
    u8 unk_0xC4;
    DepthBuffer *mDepthBuffer; // 0xc8
    SkeletonIdentifier *mSkelIdentifier; // 0xcc
    SkeletonChooser *mSkelChooser; // 0xd0
    HandInvokeGestureFilter *mHandInvokeGestureFilter; // 0xd4
    HandsUpGestureFilter *mHandsUpGestureFilter; // 0xd8
    SkeletonExtentTracker *mSkelExtTracker; // 0xdc
    int unke0;

private:
    void SyncVoiceControl();
    void SetCursorAlpha(float) const; // why is it const if it's a setter. i hate you

    DataNode OnMsg(const ResetControllerModeTimeoutMsg &);
    DataNode OnMsg(const LeftHandListEngagementMsg &);
    DataNode OnMsg(const SpeechEnableMsg &);
    DataNode OnMsg(const SpeechRecoMessage &);
    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnMsg(const JoypadConnectionMsg &);
};
