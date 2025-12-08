#pragma once

#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/HandsUpGestureFilter.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonExtentTracker.h"
#include "gesture/SpeechMgr.h"
#include "lazer/meta_ham/SkeletonChooser.h"
#include "lazer/meta_ham/SkeletonIdentifier.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Anim.h"
#include "types.h"
#include "ui/UIPanel.h"

class ShellInput : public Hmx::Object, public SkeletonCallback {
public:
    ShellInput();
    virtual ~ShellInput();
    virtual DataNode Handle(DataArray *, bool);

    // SkeletonCallback
    virtual void Clear();
    virtual void Update(const struct SkeletonUpdateData &);
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &);

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

    u8 unk_0x30, unk_0x31, unk_0x32;
    ObjPtr<RndAnimatable> unk_0x34;
    DoubleExponentialSmoother unk_0x48;
    u32 unk_0x60, unk_0x64;
    Timer unk_0x68;
    float unk_0x98;
    float unk_0x9C;
    float unk_0xA0;
    u8 unk_0xA4;
    ObjPtr<RndAnimatable> unk_0xA8;
    UIPanel *unk_0xBC;
    UIPanel *mCursorPanel; // 0xC0
    u8 unk_0xC4;
    u32 unk_0xC8; // DepthBuffer*
    SkeletonIdentifier *unk_0xCC;
    SkeletonChooser *unk_0xD0;
    HandInvokeGestureFilter *unk_0xD4;
    HandsUpGestureFilter *unk_0xD8;
    SkeletonExtentTracker *unk_0xDC;

private:
    void SyncVoiceControl();
    void SetCursorAlpha(float) const; // why is it const if it's a setter. i hate you

    DataNode OnMsg(class ResetControllerModeTimeoutMsg const &);
    DataNode OnMsg(class LeftHandListEngagementMsg const &);
    DataNode OnMsg(const SpeechEnableMsg &);
    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnMsg(const JoypadConnectionMsg &);
};
