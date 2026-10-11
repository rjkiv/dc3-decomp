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
LeftHandListEngagementMsg(bool b) : Message(Type(), b) {}
bool Engaged() const { return mData->Int(2); }
END_MESSAGE

DECLARE_MESSAGE(ResetControllerModeTimeoutMsg, "reset_controller_mode_timeout")
ResetControllerModeTimeoutMsg() : Message(Type()) {}
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
    void DrawDebug();

    SkeletonChooser *GetSkeletonChooser() { return mSkeletonChooser; }

private:
    void SyncVoiceControl();
    void SetCursorAlpha(float) const; // why is it const if it's a setter. i hate you

    DataNode OnMsg(const ResetControllerModeTimeoutMsg &);
    DataNode OnMsg(const LeftHandListEngagementMsg &);
    DataNode OnMsg(const SpeechEnableMsg &);
    DataNode OnMsg(const SpeechRecoMessage &);
    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnMsg(const JoypadConnectionMsg &);

    bool mWasVoiceControlEnabled; // 0x30
    bool mWasHandUp[2]; // 0x31
    ObjPtr<RndAnimatable> mDisengageAnim; // 0x34
    DoubleExponentialSmoother mDisengageFrame; // 0x48
    float mHandReappearedTimer; // 0x5c
    int mLastButtonDownPadNum; // 0x60
    Timer mControllerModeTimeoutTimer; // 0x68
    float mControllerModeTimeoutDurationMs; // 0x98
    float mRightHandDisengage; // 0x9c
    float mLeftHandDisengage; // 0xa0
    bool mLastWrongHandPosition; // 0xa4
    ObjPtr<RndAnimatable> mWrongHandPositionAnim; // 0xa8
    UIPanel *mInputPanel; // 0xbc
    UIPanel *mCursorPanel; // 0xc0
    bool mDrawCursor; // 0xc4
    DepthBuffer *mDepthBuffer; // 0xc8
    SkeletonIdentifier *mSkeletonIdentifier; // 0xcc
    SkeletonChooser *mSkeletonChooser; // 0xd0
    HandInvokeGestureFilter *mInvokeOptionsFilter; // 0xd4
    HandsUpGestureFilter *mCorrectIdentityFilter; // 0xd8
    SkeletonExtentTracker *mSkeletonExtentTracker; // 0xdc
    int mLastInputPadNum; // 0xe0
};
