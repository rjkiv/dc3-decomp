#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class CamShot;

class CamShotFrame {
    friend bool PropSync(CamShotFrame &, DataNode &, DataArray *, int, PropOp);

public:
    enum BlendEaseMode {
        /** "blend in and out the same amount" */
        kBlendEaseInAndOut = 0,
        /** "slow rate of change, then fast" */
        kBlendEaseIn = 1,
        /** "fast rate of change, then slow" */
        kBlendEaseOut = 2
    };
    CamShotFrame(Hmx::Object *);
    CamShotFrame(Hmx::Object *, const CamShotFrame &);

    void Save(BinStream &) const;
    void Load(BinStreamRev &);
    bool
    OnSyncTargets(ObjPtrList<RndTransformable> &, DataNode &, DataArray *, int, PropOp);
    bool OnSyncParent(ObjPtr<RndTransformable> &, DataNode &, DataArray *, int, PropOp);

private:
    /** "Duration this keyframe holds steady" */
    float mDuration; // 0x0
    /** "Duration this keyframe blends into the next one" */
    float mBlend; // 0x4
    /** "Amount to ease into this keyframe". Ranges from 0 to 1000. */
    float mBlendEase; // 0x8
    /** "Amount to ease out to the next keyframe" */
    BlendEaseMode mBlendEaseMode; // 0xc
    float unk10;
    /** "Field of view, in degrees, for this keyframe.
        Same as setting lens focal length below". Ranges from 0 to 360. */
    float mFOV; // 0x14
    /** "Field of view adjustment (not affected by target reframing" */
    float mZoomFOV; // 0x18
    /** "Camera position for this keyframe" */
    Transform mWorldOffset; // 0x1c
    /** "Screen space offset of target for this keyframe" */
    Vector2 mScreenOffset; // 0x5c
    /** "Noise frequency for camera shake" */
    float mShakeNoiseFreq; // 0x64
    /** "Noise amplitude for camera shake" */
    float mShakeNoiseAmp; // 0x68
    /** "Maximum angle for camera shake" */
    Vector2 mShakeMaxAngle; // 0x6c
    /** "0 to 1 scale representing the Depth size of the blur valley
        (offset from the focal target + focus_blur_multiplier) in the Camera Frustrum.
        Zero puts everything in Blur. 1 puts everything in the Blur falloff valley."
        Ranges from 0 to 1. */
    float mBlurDepth; // 0x74
    /** "Maximum blurriness". Ranges from 0 to 1. */
    float mMaxBlur; // 0x78
    /** "Minimum blurriness". Ranges from 0 to 1. */
    float mMinBlur; // 0x7c
    /** "Multiplier of distance from camere to focal target.
        Offsets focal point of blur." */
    float mFocusBlurMultiplier; // 0x80
    /** "Target(s) that the camera should look at" */
    ObjPtrList<RndTransformable> mTargets; // 0x84
    Vector3 mLastTargetPos; // 0x98
    /** "Parent that the camera should attach itself to" */
    ObjPtr<RndTransformable> mParent; // 0xa8
    Transform unkbc;
    /** "The focal point when calculated depth of field" */
    ObjPtr<RndTransformable> mFocalTarget; // 0xfc
    /** "Whether to take the parent object's rotation into account" */
    bool mUseParentRotation; // 0x110
    /** "Only parent on the first frame" */
    bool mParentFirstFrame; // 0x111
    CamShot *mCamShot; // 0x114
};

class CamShotCrowd {
public:
};

/** "A camera shot. This is an animated camera path with keyframed settings." */
class CamShot : public RndAnimatable, public RndTransformable {
public:
    // Hmx::Object
    virtual ~CamShot();
    OBJ_CLASSNAME(CamShot);
    OBJ_SET_TYPE(CamShot);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void StartAnim();
    virtual void EndAnim();
    virtual void SetFrame(float, float);
    virtual float StartFrame() { return 0; }
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // CamShot
    virtual void SetPreFrame(float, float) {}
    virtual CamShot *CurrentShot() {}
    virtual bool CheckShotStarted();
    virtual bool CheckShotOver(float);

    OBJ_MEM_OVERLOAD(0xAD)
    NEW_OBJ(CamShot)

protected:
    CamShot();

    // these three could be re-ordered, unsure of current order rn
    virtual void ApplyDynamicOffsetPreLookAt(Transform &, bool);
    virtual void ApplyDynamicOffsetPostLookAt(Transform &);
    virtual void ApplyFinalCamTransform(Transform &);

    virtual float ZoomFovOffset() { return 0; }
};
