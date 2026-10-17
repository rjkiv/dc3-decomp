#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/PartLauncher.h"
#include "synth/Sequence.h"
#include "utl/MemMgr.h"

/**
 * @brief An object that can trigger based on an in-game event.
 * Original _objects description:
 * "Triggers animations, sfx, and responses to game events"
 */
class EventTrigger : public virtual Hmx::Object, public RndAnimatable {
public:
    enum TriggerOrder {
        kTriggerRandom = 0,
        kTriggerSequence = 1
    };

    enum TriggerAnim {
        /** "Do not trigger based on any Anim call" */
        kTriggerAnimNone = 0,
        /** "Trigger when StartAnim gets called (e.g. very start of a CamShot), will be
         * Reset when EndAnim gets called" */
        kTriggerAnimStart = 1,
        /** "Trigger when EndAnim gets called (e.g. very end of a CamShot)" */
        kTriggerAnimEnd = 2,
        /** "Trigger when frame crosses anim_frame (e.g. during a CamShot)" */
        kTriggerAnimFrame = 3
    };

    struct Anim {
        Anim(Hmx::Object *);
        Anim &operator=(const Anim &);

        ObjOwnerPtr<RndAnimatable> mAnim; // 0x0
        float mBlend; // 0x14
        bool mWait; // 0x18
        float mDelay; // 0x1c
        /** "Enable animation filtering" */
        bool mEnable; // 0x20
        /** "Rate to animate" */
        RndAnimatable::Rate mRate; // 0x24
        /** "Start frame of animation" */
        float mStart; // 0x28
        /** "End frame of animation" */
        float mEnd; // 0x2c
        /** "Period of animation if non-zero" */
        float mPeriod; // 0x30
        /** "Scale of animation" */
        float mScale; // 0x34
        /** "Type of animation" */
        Symbol mType; // 0x38
    };
    struct ProxyCall {
        ProxyCall(Hmx::Object *);
        ProxyCall &operator=(const ProxyCall &);

        ObjOwnerPtr<ObjectDir> mProxy; // 0x0
        Symbol mCall; // 0x14
        /** "event trigger to fire" */
        ObjOwnerPtr<EventTrigger> mEvent; // 0x18
    };

    struct HideDelay {
        HideDelay(Hmx::Object *);
        HideDelay &operator=(const HideDelay &);

        /** "object to hide" */
        ObjOwnerPtr<RndDrawable> mHide; // 0x0
        /** "seconds to delay hiding" */
        float mDelay; // 0x14
        /** "rate by which to measure delay" */
        int mRate; // 0x18
    };

    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(EventTrigger);
    OBJ_SET_TYPE(EventTrigger);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetName(const char *, ObjectDir *);

    virtual void StartAnim();
    virtual void EndAnim();
    virtual void SetFrame(float, float);
    /** "Tries to trigger this EventTrigger, won't work if [enabled] is false" */
    virtual void Trigger(); // 0x50
    virtual void BasicReset();
    virtual DataArray *SupportedEvents();

    OBJ_MEM_OVERLOAD(0x17);
    NEW_OBJ(EventTrigger);
    REGISTER_OBJ(EventTrigger)

    void SetNextLink(EventTrigger *);
    bool HasTriggerEvents() const { return !mTriggerEvents.empty(); }

    static void Init();

protected:
    EventTrigger();

    virtual void CheckAnims() {}

    void RegisterEvents();
    void UnregisterEvents();
    void CleanupEventCase(std::list<Symbol> &);
    void CleanupHideShow();
    void TriggerSelf();
    void ConvertParticleTriggerType();
    void LoadOldAnim(BinStream &, RndAnimatable *);
    void LoadOldEvent(BinStreamRev &, Hmx::Object *, const char *, ObjectDir *);

    DataNode OnTrigger(DataArray *);
    DataNode OnProxyCalls(DataArray *);

    static DataNode Cleanup(DataArray *);

    ObjList<Anim> mAnims; // 0x10
    ObjPtrList<Task> mSpawnedTasks; // 0x1c
    ObjList<ProxyCall> mProxyCalls; // 0x30
    ObjPtrList<Sequence> mSounds; // 0x3c
    /** "objects to show" */
    ObjPtrList<RndDrawable> mShows; // 0x50
    /** "these triggers will be reset (i.e. hide its shows, etc.)
        when this trigger fires" */
    ObjPtrList<EventTrigger> mResetTriggers; // 0x64
    ObjList<HideDelay> mHideDelays; // 0x78
    /** "Events which enable this trigger" */
    std::list<Symbol> mTriggerEvents; // 0x84
    /** "event which enable this event trigger" */
    std::list<Symbol> mEnableEvents; // 0x8c
    /** "event which disable this event trigger" */
    std::list<Symbol> mDisableEvents; // 0x94
    /** "When events are triggered, arm them,
        and delay fire until this handler is called" */
    std::list<Symbol> mWaitForEvents; // 0x9c
    ObjPtr<EventTrigger> mNextLink; // 0xa4
    Symbol mHandler; // 0xb8
    ObjPtrList<RndPartLauncher> mPartLaunchers; // 0xbc
    bool unkd0;
    TriggerAnim mAnimTrigger; // 0xd4
    float mAnimFrame; // 0xd8
    bool mEnabled; // 0xdc
    bool mEnabledAtStart; // 0xdd
    bool mWaiting; // 0xde
    ObjPtrList<RndDrawable> mHidden; // 0xe0
    ObjPtrList<RndDrawable> mShown; // 0xf4
    bool mTriggered; // 0x108
    TriggerOrder mTriggerOrder; // 0x10c
    int mLastTriggerIndex; // 0x110
};
