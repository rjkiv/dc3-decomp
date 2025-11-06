#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/BinStream.h"

enum UITransitionAnimationState {
    kUITransitionAnimationInvalid,
    kUITransitionAnimationIdle,
    kUITransitionAnimationInAnimating,
    kUITransitionAnimationOutAnimating,
    kUITransitionAnimationReverseOutAnimating
};

class UITransitionHandler {
public:
    UITransitionHandler(Hmx::Object *);
    virtual ~UITransitionHandler();

    void SetInAnim(RndAnimatable *);
    void SetOutAnim(RndAnimatable *);

    RndAnimatable *GetInAnim() const; //{ return mInAnim; }
    RndAnimatable *GetOutAnim() const; //{ return mOutAnim; }

    void ClearAnimationState();

protected:
    virtual void FinishValueChange();
    virtual void StartValueChange();
    virtual bool IsEmptyValue() const = 0;

    void UpdateHandler();
    void SaveHandlerData(BinStream &);
    void CopyHandlerData(const UITransitionHandler *);

    bool HasTransitions() const;
    bool IsReadyToChange() const;
    void LoadHandlerData(BinStream &);

    /** "animation kicked off before [transition]" */
    ObjPtr<RndAnimatable> mInAnim; // 0x4
    /** "animation kicked off after [transition]" */
    ObjPtr<RndAnimatable> mOutAnim; // 0x18
    /** The current transition animation state. */
    UITransitionAnimationState mAnimationState; // 0x2c
    bool mChangePending; // 0x30
    bool unk31; // 0x31
};
