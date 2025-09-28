#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Platform.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "world/Crowd.h"
#include "world/Spotlight.h"

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
    bool SameTargets(const CamShotFrame &) const;
    void GetCurrentTargetPosition(Vector3 &) const;
    void ApplyScreenOffset(Transform &, RndCam *) const;
    void UpdateTarget() const;
    void BuildTransform(RndCam *, Transform &, bool) const;
    void Interp(const CamShotFrame &, float, float, RndCam *);
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
    Transform mTargetXfm; // 0xbc
    /** "The focal point when calculated depth of field" */
    ObjPtr<RndTransformable> mFocalTarget; // 0xfc
    /** "Whether to take the parent object's rotation into account" */
    bool mUseParentRotation; // 0x110
    /** "Only parent on the first frame" */
    bool mParentFirstFrame; // 0x111
    CamShot *mCamShot; // 0x114
};

inline BinStream &operator<<(BinStream &bs, const CamShotFrame &f) {
    f.Save(bs);
    return bs;
}

enum CrowdRotate {
    /** "Face along the placement mesh, or along focus, if set" */
    kCrowdRotateNone = 0,
    /** "Face towards the camera" */
    kCrowdRotateFace = 1,
    /** "Face away from the camera" */
    kCrowdRotateAway = 2
};

class CamShotCrowd {
    friend bool PropSync(CamShotCrowd &, DataNode &, DataArray *, int, PropOp);

public:
    CamShotCrowd(Hmx::Object *);
    CamShotCrowd(Hmx::Object *, const CamShotCrowd &);

    void Save(BinStream &) const;
    void Load(BinStream &);
    void AddCrowdChars();
    void SetCrowdChars();
    void ClearCrowdChars();
    void
    GetSelectedCrowd(std::list<
                     std::pair<RndMultiMesh *, std::list<RndMultiMesh::Instance>::iterator> >
                         &);
    void
    AddCrowdChars(std::list<
                  std::pair<RndMultiMesh *, std::list<RndMultiMesh::Instance>::iterator> >
                      &);

private:
    /** "The crowd to show for this shot" */
    ObjPtr<WorldCrowd> mCrowd; // 0x0
    /** "How to rotate crowd" */
    CrowdRotate mCrowdRotate; // 0x14
    std::vector<std::pair<int, int> > unk18; // 0x18
    CamShot *unk24; // 0x24
};

inline BinStream &operator<<(BinStream &bs, const CamShotCrowd &f) {
    f.Save(bs);
    return bs;
}

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
    virtual float EndFrame() { return mDuration; }
    virtual Hmx::Object *AnimTarget() { return sAnimTarget; }
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // CamShot
    virtual void SetPreFrame(float, float) {}
    virtual CamShot *CurrentShot() { return nullptr; }

    OBJ_MEM_OVERLOAD(0xAD)
    NEW_OBJ(CamShot)

    float GetDurationSeconds() const;
    bool PlatformOk() const;
    void StartAnims(ObjPtrList<RndAnimatable> &);
    Symbol Category() const { return mCategory; }

protected:
    CamShot();

    static Hmx::Object *sAnimTarget;

    virtual bool CheckShotStarted();
    virtual bool CheckShotOver(float);
    // these three could be re-ordered, unsure of current order rn
    virtual void ApplyDynamicOffsetPreLookAt(Transform &, bool) {}
    virtual void ApplyDynamicOffsetPostLookAt(Transform &) {}
    virtual void ApplyFinalCamTransform(Transform &) {}

    virtual float ZoomFovOffset() { return 0; }

    void CacheFrames();
    void UnHide();

    DataNode OnHasTargets(DataArray *);
    DataNode OnSetPos(DataArray *);
    DataNode OnSetCrowdChars(DataArray *);
    DataNode OnAddCrowdChars(DataArray *);
    DataNode OnClearCrowdChars(DataArray *);
    DataNode OnGetOccluded(DataArray *);
    DataNode OnSetAllCrowdChars3D(DataArray *);
    DataNode OnRadio(DataArray *);

    /** The collection of keyframes. */
    ObjVector<CamShotFrame> mKeyframes; // 0xd0
    /** "Whether the animation should loop." */
    bool mLooping; // 0xe0
    /** "If looping true, which keyframe to loop to." */
    int mLoopKeyframe; // 0xe4
    /** "Near clipping plane for the camera" */
    float mNearPlane; // 0xe8
    /** "Far clipping plane for the camera" */
    float mFarPlane; // 0xec
    /** "Whether to use depth-of-field effect on platforms that support it" */
    bool mUseDepthOfField; // 0xf0
    /** "Filter amount" */
    float mFilter; // 0xf4
    /** "Height above target's base at which to clamp camera" */
    float mClampHeight; // 0xf8
    /** "Category for shot-picking" */
    Symbol mCategory; // 0xfc
    int unk100; // 0x100
    /** "animatables to be driven with the same frame" */
    ObjPtrList<RndAnimatable> mAnims; // 0x104
    /** "Optional camera path to use" */
    ObjPtr<RndTransAnim> mPath; // 0x118
    float mPathFrame; // 0x12c
    /** "Limit this shot to given platform" - the options are kPlatformNone/PS3/Xbox */
    Platform mPlatform; // 0x130
    /** "List of objects to hide while this camera shot is active,
        shows them when done" */
    ObjPtrList<RndDrawable> mHideList; // 0x134
    /** "List of objects to show while this camera shot is active,
        hides them when done" */
    ObjPtrList<RndDrawable> mShowList; // 0x148
    /** "Automatically generated list of objects to hide while this camera shot is active,
        shows them when done.  Not editable" */
    ObjPtrList<RndDrawable> mGenHideList; // 0x15c
    std::vector<RndDrawable *> mGenHideVector; // 0x170
    /** "List of objects to draw in order instead of whole world" */
    ObjPtrList<RndDrawable> mDrawOverrides; // 0x17c
    /** "List of objects to draw after post-processing" */
    ObjPtrList<RndDrawable> mPostProcOverrides; // 0x190
    ObjPtr<RndDir> unk1a4;
    ObjVector<CamShotCrowd> mCrowds; // 0x1b8
    /** "Force the croawd into a particular state".
        Options are: (none bad ok great
            skills_bad skills_ok skills_great
            realtime_idle realtime_bad realtime_ok realtime_great) */
    Symbol mCrowdStateOverride; // 0x1c8
    /** "global per-pixel setting for PS3" */
    bool mPS3PerPixel; // 0x1cc
    /** "The spotlight to get glow settings from" */
    ObjPtr<Spotlight> mGlowSpot; // 0x1d0
    int mFlags; // 0x1e4
    ObjPtrList<RndDrawable> unk1e8;
    ObjPtrList<RndDrawable> unk1fc;
    Vector3 unk210;
    Vector3 unk220;
    Vector3 unk230;
    Vector3 unk240;
    Vector3 unk250;
    Vector3 unk260;
    int unk270;
    int unk274;
    /** "duration of the camshot" */
    float mDuration; // 0x278
    /** "disabled bits" */
    int mDisabled; // 0x27c
    bool unk280;
    bool unk281;
    bool unk282;
    bool unk283;
};

class AutoPrepTarget {
public:
    AutoPrepTarget(CamShotFrame &);
    ~AutoPrepTarget();

    static bool sChanging;

private:
    CamShotFrame *mFrame; // 0x0
    CamShot *mShot; // 0x4
    float mOldFilter; // 0x8
    float mOldCamHeight; // 0xc
    float mOldZoomFov; // 0x10
};
