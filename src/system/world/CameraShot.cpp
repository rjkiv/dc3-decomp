#include "world/CameraShot.h"
#include "hamobj/HamWardrobe.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/MultiMeshProxy.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "rndobj/VelocityBuffer.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "world/CameraManager.h"
#include "world/Crowd.h"
#include "world/FreeCamera.h"
#include "world/Dir.h"
#include <cstdlib>

inline float ComputeFOVScale(float fov) {
    return 24.0f / (float(std::tan(fov / 2.0f)) * 2.0f);
}
inline float ScaleToFOV(float scale) {
    return float(std::atan(24.0f / (scale * 2.0f))) * 2.0f;
}

#pragma region CamShotFrame

CamShotFrame::CamShotFrame(Hmx::Object *owner)
    : mDuration(0), mBlend(0), mBlendEase(0), mBlendEaseMode(kBlendEaseInAndOut),
      mFrame(-1), mFOV(1.2217305f), mZoomFOV(0), mShakeNoiseFreq(0), mShakeNoiseAmp(0),
      mShakeMaxAngle(0, 0), mBlurDepth(0.35), mMaxBlur(1), mMinBlur(0),
      mFocusBlurMultiplier(0), mTargets(owner), mParent(owner), mFocalTarget(owner),
      mUseParentRotation(false), mParentFirstFrame(false),
      mCamShot(dynamic_cast<CamShot *>(owner)) {
    mWorldOffset.Reset();
    mScreenOffset.Zero();
    mLastTargetPos.x = kHugeFloat;
}

CamShotFrame::CamShotFrame(Hmx::Object *shotOwner, const CamShotFrame &other)
    : mDuration(other.mDuration), mBlend(other.mBlend), mBlendEase(other.mBlendEase),
      mBlendEaseMode(other.mBlendEaseMode), mFOV(other.mFOV), mZoomFOV(other.mZoomFOV),
      mWorldOffset(other.mWorldOffset), mScreenOffset(other.mScreenOffset),
      mShakeNoiseFreq(other.mShakeNoiseFreq), mShakeNoiseAmp(other.mShakeNoiseAmp),
      mShakeMaxAngle(other.mShakeMaxAngle), mBlurDepth(other.mBlurDepth),
      mMaxBlur(other.mMaxBlur), mMinBlur(other.mMinBlur),
      mFocusBlurMultiplier(other.mFocusBlurMultiplier), mTargets(other.mTargets),
      mParent(other.mParent), mFocalTarget(other.mFocalTarget),
      mUseParentRotation(other.mUseParentRotation), mParentFirstFrame(false) {
    mCamShot = dynamic_cast<CamShot *>(shotOwner);
}

void CamShotFrame::Save(BinStream &bs) const {
    bs << mDuration;
    bs << mBlend;
    bs << mBlendEase;
    bs << mBlendEaseMode;
    bs << mFOV;
    bs << mWorldOffset;
    bs << mScreenOffset;
    bs << mBlurDepth;
    bs << mMaxBlur;
    bs << mMinBlur;
    bs << mFocusBlurMultiplier;
    bs << mTargets;
    bs << mFocalTarget;
    bs << mParent;
    bs << mUseParentRotation;
    bs << mShakeNoiseAmp;
    bs << mShakeNoiseFreq;
    bs << mShakeMaxAngle;
    bs << mZoomFOV;
    bs << mParentFirstFrame;
}

RndTransformable *LoadSubPart(BinStreamRev &, CamShot *);

void CamShotFrame::Load(BinStreamRev &d) {
    d >> mDuration;
    d >> mBlend;
    d >> mBlendEase;
    if (d.rev > 0x2D) {
        d >> (int &)mBlendEaseMode;
    }
    d >> mFOV;
    d >> mWorldOffset;
    Transform zeroXfm;
    zeroXfm.Zero();
    if (zeroXfm == mWorldOffset) {
        mWorldOffset.Reset();
    }
    d >> mScreenOffset;
    d >> mBlurDepth;
    if (d.rev < 0x17) {
        mBlurDepth = 1 - mBlurDepth;
        int x;
        d >> x;
    }
    if (d.rev > 0x17) {
        d >> mMaxBlur;
    } else {
        mMaxBlur = 1;
    }
    if (d.rev > 0x1C) {
        d >> mMinBlur;
    } else {
        mMinBlur = 0;
    }
    if (d.rev > 0x14) {
        d >> mFocusBlurMultiplier;
    } else {
        mFocusBlurMultiplier = 0;
    }
    if (d.rev < 0x17) {
        int x;
        d >> x;
    }
    if (d.rev > 0x2B) {
        d >> mTargets;
    } else {
        int count;
        d >> count;
        mTargets.clear();
        for (int i = 0; i < count; i++) {
            RndTransformable *t = LoadSubPart(d, mCamShot);
            if (t) {
                mTargets.push_back(t);
            }
        }
    }
    if (d.rev > 0x1A) {
        if (d.rev > 0x2B) {
            d >> mFocalTarget;
        } else {
            mFocalTarget = LoadSubPart(d, mCamShot);
        }
    }
    if (d.rev > 0x2B) {
        d >> mParent;
    } else {
        mParent = LoadSubPart(d, mCamShot);
    }
    d >> mUseParentRotation;
    if (d.rev > 0x11) {
        d >> mShakeNoiseAmp;
        d >> mShakeNoiseFreq;
        d >> mShakeMaxAngle;
    }
    if (d.rev > 0x15) {
        d >> mZoomFOV;
    }
    if (d.rev > 0x28) {
        d >> mParentFirstFrame;
    }
}

BinStreamRev &operator>>(BinStreamRev &d, CamShotFrame &csf) {
    csf.Load(d);
    return d;
}

bool CamShotFrame::SameTargets(const CamShotFrame &other) const {
    if (mTargets.size() != other.mTargets.size())
        return false;
    FOREACH (it, mTargets) {
        ObjPtrList<RndTransformable>::iterator otherIt = other.mTargets.begin();
        for (; otherIt != other.mTargets.end(); ++otherIt) {
            if (*it == *otherIt)
                break;
        }
        if (otherIt == other.mTargets.end())
            return false;
    }
    return true;
}

void CamShotFrame::GetCurrentTargetPosition(Vector3 &v) const {
    v.Zero();
    int count = 0;
    FOREACH (it, mTargets) {
        RndTransformable *cur = *it;
        if (cur) {
            count++;
            Add(v, cur->WorldXfm().v, v);
        }
    }
    if (count > 0)
        v *= (1.0f / (float)count);
}

void CamShotFrame::ApplyScreenOffset(Transform &xfm, RndCam *cam) const {
    if (HasTargets()) {
        xfm.LookAt(mLastTargetPos, xfm.m.z);
    }
    Vector3 v;
    Subtract(xfm.v, mLastTargetPos, v);
    float length = std::sqrt(v.y * v.y + v.z * v.z + v.x * v.x);
    Vector3 vother(
        -(mScreenOffset.x / cam->LocalProjectXfm().m.x.x) * length,
        0,
        (mScreenOffset.y / cam->LocalProjectXfm().m.z.y) * length
    );
    Multiply(vother, xfm, xfm.v);
}

void CamShotFrame::UpdateTarget() const {
    CamShotFrame *me = const_cast<CamShotFrame *>(this);
    GetCurrentTargetPosition(me->mLastTargetPos);
    if (mParent) {
        me->mTargetXfm = mParent->WorldXfm();
    }
}

bool CamShotFrame::OnSyncTargets(
    ObjPtrList<RndTransformable> &transList,
    DataNode &node,
    DataArray *prop,
    int i,
    PropOp op
) {
    bool synced;
    if (op != kPropGet && op != kPropSize) {
        AutoPrepTarget target(*this);
        synced = PropSync(transList, node, prop, i, op);
    } else
        synced = PropSync(transList, node, prop, i, op);
    return synced;
}

bool CamShotFrame::OnSyncParent(
    ObjPtr<RndTransformable> &parent, DataNode &node, DataArray *prop, int i, PropOp op
) {
    bool synced;
    if (op != kPropGet) {
        AutoPrepTarget target(*this);
        synced = PropSync(parent, node, prop, i, op);
    } else
        synced = PropSync(parent, node, prop, i, op);
    return synced;
}

bool CamShotFrame::HasTargets() const {
    FOREACH (it, mTargets) {
        if (*it)
            return true;
    }
    return false;
}

Symbol FOV_to_LensSym(float fov) {
    float scaled = ComputeFOVScale(fov);
    if (NearlyEqual(scaled, 15.0f))
        return "15mm";
    else if (NearlyEqual(scaled, 20.0f))
        return "20mm";
    else if (NearlyEqual(scaled, 24.0f))
        return "24mm";
    else if (NearlyEqual(scaled, 28.0f))
        return "28mm";
    else if (NearlyEqual(scaled, 35.0f))
        return "35mm";
    else if (NearlyEqual(scaled, 50.0f))
        return "50mm";
    else if (NearlyEqual(scaled, 85.0f))
        return "85mm";
    else if (NearlyEqual(scaled, 135.0f))
        return "135mm";
    else if (NearlyEqual(scaled, 200.0f))
        return "200mm";
    else
        return "Custom";
}

float LensSym_to_FOV(Symbol sym) {
    String lensStr(sym);
    unsigned int idx = lensStr.find("mm");
    if (idx != FixedString::npos) {
        float scale = std::atof(lensStr.substr(0, idx).c_str());
        return ScaleToFOV(scale);
    } else
        return -1;
}

BEGIN_CUSTOM_PROPSYNC(CamShotFrame)
    SYNC_PROP(duration, o.mDuration)
    SYNC_PROP(blend, o.mBlend)
    SYNC_PROP(blend_ease, o.mBlendEase)
    SYNC_PROP(blend_ease_mode, (int &)o.mBlendEaseMode)
    SYNC_PROP(world_offset, o.mWorldOffset)
    SYNC_PROP(screen_offset, o.mScreenOffset) {
        static Symbol _s("targets");
        if (sym == _s) {
            o.OnSyncTargets(o.mTargets, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    {
        static Symbol _s("parent");
        if (sym == _s) {
            o.OnSyncParent(o.mParent, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    SYNC_PROP(focal_target, o.mFocalTarget)
    SYNC_PROP(use_parent_rotation, o.mUseParentRotation)
    SYNC_PROP(parent_first_frame, o.mParentFirstFrame)
    SYNC_PROP_SET(field_of_view, o.mFOV * RAD2DEG, o.mFOV = _val.Float() * DEG2RAD)
    SYNC_PROP_SET(lens_mm, ComputeFOVScale(o.mFOV), o.mFOV = ScaleToFOV(_val.Float()))
    SYNC_PROP_SET(lens_preset, FOV_to_LensSym(o.mFOV), {
        float fov = LensSym_to_FOV(_val.Sym());
        if (fov != -1.0f)
            o.mFOV = fov;
        else
            o.mFOV += 0.00010011921f;
    })
    SYNC_PROP(blur_depth, o.mBlurDepth)
    SYNC_PROP(max_blur, o.mMaxBlur)
    SYNC_PROP(min_blur, o.mMinBlur)
    SYNC_PROP(focus_blur_multiplier, o.mFocusBlurMultiplier)
    SYNC_PROP(shake_noisefreq, o.mShakeNoiseFreq)
    SYNC_PROP(shake_noiseamp, o.mShakeNoiseAmp)
    SYNC_PROP(shake_maxangle, o.mShakeMaxAngle)
    SYNC_PROP_SET(zoom_fov, o.mZoomFOV * RAD2DEG, o.mZoomFOV = _val.Float() * DEG2RAD)
END_CUSTOM_PROPSYNC

#pragma endregion
#pragma region CamShotCrowd

CamShotCrowd::CamShotCrowd(Hmx::Object *owner)
    : mCrowd(owner), mCrowdRotate(kCrowdRotateNone),
      unk24(dynamic_cast<CamShot *>(owner)) {}

CamShotCrowd::CamShotCrowd(Hmx::Object *owner, const CamShotCrowd &other)
    : mCrowd(other.mCrowd), mCrowdRotate(other.mCrowdRotate), unk18(other.unk18),
      unk24(dynamic_cast<CamShot *>(owner)) {}

void CamShotCrowd::Save(BinStream &bs) const {
    bs << mCrowd;
    bs << mCrowdRotate;
    bs << unk18;
    int num = -1;
    if (mCrowd) {
        num = mCrowd->GetModifyStamp();
    }
    bs << num;
}

void CamShotCrowd::Load(BinStream &bs) {
    bs >> mCrowd;
    bs >> (int &)mCrowdRotate;
    bs >> unk18;
    int num;
    bs >> num;
    if (mCrowd && num != mCrowd->GetModifyStamp() || (!mCrowd && num != -1)) {
        unk18.clear();
    }
}

BinStream &operator>>(BinStreamRev &d, CamShotCrowd &c) {
    c.Load(d.stream);
    return d.stream;
}

void CamShotCrowd::AddCrowdChars() {
    std::list<std::pair<RndMultiMesh *, std::list<RndMultiMesh::Instance>::iterator> >
        selectedCrowd;
    GetSelectedCrowd(selectedCrowd);
    if (selectedCrowd.empty()) {
        MILO_NOTIFY("No selected crowd members in this crowd");
    } else {
        AddCrowdChars(selectedCrowd);
    }
}

void CamShotCrowd::SetCrowdChars() {
    std::list<std::pair<RndMultiMesh *, std::list<RndMultiMesh::Instance>::iterator> >
        selectedCrowd;
    GetSelectedCrowd(selectedCrowd);
    if (selectedCrowd.empty()) {
        MILO_NOTIFY("No selected crowd members in this crowd");
    } else {
        ClearCrowdChars();
        AddCrowdChars(selectedCrowd);
    }
}

void CamShotCrowd::ClearCrowdChars() {
    unk18.clear();
    if (!mCrowd) {
        MILO_NOTIFY("No crowd selected");
    }
    mCrowd->Set3DCharList(unk18, unk24);
}

void CamShotCrowd::GetSelectedCrowd(
    std::list<std::pair<RndMultiMesh *, std::list<RndMultiMesh::Instance>::iterator> >
        &crowdChars
) {
    FOREACH (it, RndMultiMesh::ProxyPool()) {
        RndMultiMeshProxy *proxy = it->first;
        MILO_ASSERT(proxy, 0xA06);
        RndMultiMesh *multiMesh = proxy->MultiMesh();
        if (!proxy->Refs().empty() && multiMesh) {
            crowdChars.push_back(std::make_pair(multiMesh, proxy->Index()));
            proxy->SetMultiMesh(0, 0);
        }
    }
}

BEGIN_CUSTOM_PROPSYNC(CamShotCrowd)
    SYNC_PROP_MODIFY(crowd, o.mCrowd, o.unk18.clear())
    SYNC_PROP(crowd_rotate, (int &)o.mCrowdRotate)
END_CUSTOM_PROPSYNC

#pragma endregion
#pragma region CamShot

#define CAMERA_LOG(...)                                                                  \
    if (DataVariable("camera_spew") != 0) {                                              \
        MILO_LOG(__VA_ARGS__);                                                           \
    }

CamShot::CamShot()
    : mKeyframes(this), mLooping(false), mLoopKeyframe(0),
      mNearPlane(RndCam::DefaultNearPlane()),
      mFarPlane(mNearPlane * RndCam::MaxFarNearPlaneRatio()), mUseDepthOfField(true),
      mFilter(0.9), mClampHeight(-1), mAnims(this), mPath(this), mPathFrame(-1),
      mPlatform(kPlatformNone), mHideList(this), mShowList(this), mGenHideList(this),
      mDrawOverrides(this), mPostProcOverrides(this), unk1a4(this), mCrowds(this),
      mCrowdStateOverride(gNullStr), mPS3PerPixel(true), mGlowSpot(this), mFlags(0),
      mEndHideList(this), mEndShowList(this), unk210(0, 0, 0), unk220(0, 0, 0),
      unk230(0, 0, 0), unk240(0, 0, 0), unk250(0, 0, 0), unk260(0, 0, 0), mLastNext(0),
      mLastPrev(0), mDuration(0), mDisabled(0), mShotStarted(1), mShotOver(0), mHidden(0),
      unk283(0) {}

CamShot::~CamShot() {}

DataNode CamShot::OnGetOccluded(DataArray *) { return 0; }
DataNode CamShot::OnSetAllCrowdChars3D(DataArray *) { return 0; }

BEGIN_HANDLERS(CamShot)
    HANDLE(has_targets, OnHasTargets)
    HANDLE(set_pos, OnSetPos)
    HANDLE_EXPR(duration_seconds, GetDurationSeconds())
    HANDLE(set_3d_crowd, OnSetCrowdChars)
    HANDLE(add_3d_crowd, OnAddCrowdChars)
    HANDLE(clear_3d_crowd, OnClearCrowdChars)
    HANDLE_EXPR(get_crowd_dir, GetCrowdDir())
    HANDLE_EXPR(gen_hide_list, 0)
    HANDLE_EXPR(clear_hide_list, 0)
    HANDLE(get_occluded, OnGetOccluded)
    HANDLE_EXPR(platform_ok, PlatformOk())
    HANDLE(set_all_to_3D, OnSetAllCrowdChars3D)
    HANDLE(radio, OnRadio)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

WorldDir *CamShot::GetCrowdDir() const {
    ObjectDir *dir = unk1a4.Ptr() ? unk1a4.Ptr() : Dir();
    return dynamic_cast<WorldDir *>(dir);
}

#define SYNC_PROP_LIST(s, member)                                                        \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (!(_op & (kPropGet | kPropSize)))                                         \
                UnHide();                                                                \
            if (PropSync(member, _val, _prop, _i + 1, _op))                              \
                return true;                                                             \
            else                                                                         \
                return false;                                                            \
        }                                                                                \
    }

BEGIN_PROPSYNCS(CamShot)
    SYNC_PROP_MODIFY(keyframes, mKeyframes, CacheFrames())
    SYNC_PROP(looping, mLooping)
    SYNC_PROP(loop_keyframe, mLoopKeyframe)
    SYNC_PROP_SET(category, mCategory, mCategory = _val.ForceSym())
    SYNC_PROP(filter, mFilter)
    SYNC_PROP(clamp_height, mClampHeight)
    SYNC_PROP(near_plane, mNearPlane)
    SYNC_PROP(far_plane, mFarPlane) {
        static Symbol _s("duration");
        if (sym == _s && _op & kPropGet)
            return PropSync(mDuration, _val, _prop, _i + 1, _op);
    }
    SYNC_PROP(use_depth_of_field, mUseDepthOfField)
    SYNC_PROP(path, mPath)
    SYNC_PROP(path_frame, mPathFrame)
    SYNC_PROP(platform_only, (int &)mPlatform)
    SYNC_PROP_LIST(hide_list, mHideList)
    SYNC_PROP_LIST(show_list, mShowList)
    SYNC_PROP_LIST(gen_hide_list, mGenHideList)
    SYNC_PROP(draw_overrides, mDrawOverrides)
    SYNC_PROP(postproc_overrides, mPostProcOverrides)
    SYNC_PROP(glow_spot, mGlowSpot)
    SYNC_PROP(crowds, mCrowds)
    SYNC_PROP(crowd_state_override, mCrowdStateOverride)
    SYNC_PROP(ps3_per_pixel, mPS3PerPixel)
    SYNC_PROP_BITFIELD(flags, mFlags, 0xB94)
    SYNC_PROP_SET(disabled, mDisabled, )
    SYNC_PROP(anims, mAnims)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CamShot)
    SAVE_REVS(0x34, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mKeyframes;
    bs << mLooping;
    bs << mLoopKeyframe;
    bs << mNearPlane;
    bs << mFarPlane;
    bs << mUseDepthOfField;
    bs << mFilter;
    bs << mClampHeight;
    bs << mPath;
    bs << mCategory;
    bs << mPlatform;
    bs << mHideList;
    MILO_ASSERT(mGenHideVector.empty(), 0x3CE);
    if (bs.Cached()) {
        FOREACH (it, mHideList) {
            mGenHideList.remove(*it);
        }
        if (bs.GetPlatform() == kPlatformXBox) {
            mGenHideList.clear();
        }
    }
    bs << mGenHideList;
    bs << mShowList;
    bs << mGlowSpot;
    bs << mDrawOverrides;
    bs << mPostProcOverrides;
    bs << mPS3PerPixel;
    bs << mFlags;
    bs << mCrowds;
    bs << mCrowdStateOverride;
    bs << mAnims;
END_SAVES

BEGIN_COPYS(CamShot)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(CamShot)
    BEGIN_COPYING_MEMBERS
        mKeyframes.clear();
        for (int i = 0; i != c->mKeyframes.size(); i++) {
            mKeyframes.push_back(CamShotFrame(this, c->mKeyframes[i]));
        }
        mCrowds.clear();
        for (int i = 0; i != c->mCrowds.size(); i++) {
            mCrowds.push_back(CamShotCrowd(this, c->mCrowds[i]));
        }
        COPY_MEMBER(mCrowdStateOverride)
        COPY_MEMBER(mNearPlane)
        COPY_MEMBER(mFarPlane)
        COPY_MEMBER(mUseDepthOfField)
        COPY_MEMBER(mFilter)
        COPY_MEMBER(mClampHeight)
        COPY_MEMBER(mPath)
        COPY_MEMBER(mPlatform)
        COPY_MEMBER(mCategory)
        COPY_MEMBER(mHideList)
        COPY_MEMBER(mGenHideList)
        COPY_MEMBER(mGenHideVector)
        COPY_MEMBER(mShowList)
        COPY_MEMBER(mLooping)
        COPY_MEMBER(mLoopKeyframe)
        COPY_MEMBER(mGlowSpot)
        COPY_MEMBER(mDrawOverrides)
        COPY_MEMBER(mPostProcOverrides)
        COPY_MEMBER(mPS3PerPixel)
        COPY_MEMBER(mFlags)
        COPY_MEMBER(mAnims)
        CacheFrames();
    END_COPYING_MEMBERS
END_COPYS

void LoadDrawables(BinStream &bs, std::vector<RndDrawable *> &draws, ObjectDir *dir) {
    MILO_ASSERT(dir, 0x3AC);
    draws.clear();
    uint count;
    bs >> count;
    draws.reserve(count);
    while (count != 0) {
        char name[0x80];
        bs.ReadString(name, 0x80);
        RndDrawable *draw =
            dynamic_cast<RndDrawable *>(dir->FindObject(name, false, true));
        if (draw) {
            draws.push_back(draw);
        }
        count--;
    }
}

BEGIN_LOADS(CamShot)
    LOAD_REVS(bs)
    ASSERT_REVS(0x34, 0)
    if (mShotOver) {
        UnHide();
    }
    float f19 = 0;
    float f3f4 = 0;
    if (d.rev > 0) {
        Hmx::Object::Load(bs);
        RndAnimatable::Load(bs);
    }
    if (d.rev > 0x32) {
        RndTransformable::Load(bs);
    }
    if (d.rev > 0xC) {
        d >> mKeyframes;
        d >> mLooping;
        if (d.rev > 0x1E) {
            d >> mLoopKeyframe;
        } else {
            mLoopKeyframe = false;
        }
        if (d.rev < 0x28) {
            d >> f3f4;
        }
        d >> mNearPlane;
        d >> mFarPlane;
        d >> mUseDepthOfField;
        d >> mFilter;
        d >> mClampHeight;
    } else {
        mLoopKeyframe = false;
        mNearPlane = 0;

        float fov1, fov2;
        d >> fov1;
        d >> fov2;
        if (d.rev < 9) {
            fov1 = ConvertFov(fov1, 0.75f);
            fov2 = ConvertFov(fov2, 0.75f);
        }
        Transform tf1;
        Transform tf2;
        d >> tf1;
        d >> tf2;
        Vector2 vec1;
        Vector2 vec2;
        d >> vec1;
        d >> vec2;
        if (d.rev < 0x28)
            d >> f3f4;

        float fdummy1;
        d >> fdummy1;
        d >> mNearPlane;
        d >> mFarPlane;
        d >> mUseDepthOfField;
        float someotherfloat = 1.0f;
        if (d.rev > 9) {
            float newblurdepth;
            float ff, ff2;
            d >> newblurdepth;
            d >> ff;
            d >> ff2;
            someotherfloat = 1.0f - newblurdepth;
        }
        if (d.rev < 4) {
            bool ratebool;
            d >> ratebool;
            SetRate((Rate)!ratebool);
        }
        d >> mFilter;
        if (d.rev < 7)
            mFilter = 0.9f;
        d >> mClampHeight;
        ObjPtrList<RndTransformable> pList(this);
        ObjPtr<RndTransformable> ptr(this);
        int listsize;
        d >> listsize;
        for (int i = 0; i < listsize; i++) {
            RndTransformable *subpart = LoadSubPart(d, this);
            if (subpart)
                pList.push_back(subpart);
        }
        ptr = LoadSubPart(d, this);
        bool somebool = false;
        if (d.rev > 10)
            d >> somebool;
        CamShotFrame csf1(this);
        CamShotFrame csf2(this);
        if (fdummy1 > 0.0f) {
            csf1.mDuration = 0.0f;
            csf1.mBlend = fdummy1;
            csf1.mWorldOffset = tf1;
            csf1.mScreenOffset = vec1;
            csf1.mFOV = fov1;
            csf1.mBlurDepth = someotherfloat;
            csf1.mMaxBlur = 1;
            csf1.mMinBlur = 0;
            csf1.mFocusBlurMultiplier = 0.0f;
            csf1.mTargets = pList;
            csf1.mParent = ptr;
            csf1.mUseParentRotation = somebool;
            mKeyframes.push_back(csf1);
        }
        csf2.mDuration = 0.0f;
        csf2.mBlend = 0.0f;
        csf2.mWorldOffset = tf2;
        csf2.mScreenOffset = vec2;
        csf2.mFOV = fov2;
        csf2.mBlurDepth = someotherfloat;
        csf2.mMaxBlur = 1;
        csf2.mMinBlur = 0;
        csf2.mFocusBlurMultiplier = 0.0f;
        csf2.mTargets = pList;
        csf2.mParent = ptr;
        csf2.mUseParentRotation = somebool;
        mKeyframes.push_back(csf2);
    }

    d >> mPath;
    if (d.rev > 1 && d.rev < 0x2D) {
        float f2b;
        d >> f2b;
    }
    if (d.rev > 2) {
        d >> mCategory;
        if (d.rev < 0x26) {
            float f26;
            d >> f26;
        }
    }
    if (d.rev > 0x22) {
        d >> (int &)mPlatform;
    } else if (d.rev > 0x21) {
        int state;
        d >> state;
        if (state == 1) {
            mPlatform = kPlatformXBox;
        } else if (state == 2) {
            mPlatform = kPlatformPS3;
        } else {
            mPlatform = kPlatformNone;
        }
    }
    if (d.rev < 1) {
        RndAnimatable::Load(bs);
    }
    CamShotCrowd csc(this);

    if (d.rev > 4 && d.rev < 42) {
        d >> csc.unk18;
    }
    int loc240 = -1;
    if (d.rev >= 8 && d.rev < 42)
        d >> loc240;
    if (d.rev > 5) {
        mGenHideVector.clear();
        mShowList.clear();
        mHideList.clear();
        if (d.rev <= 0x2F || (bs.Cached() && d.rev < 0x32)) {
            d >> mHideList;
        } else {
            d >> mHideList;
            d >> mShowList;
        }
    }
    if (d.rev > 0x1B) {
        d >> mGenHideList;
    }

    if (d.rev > 0xB) {
        if (d.rev < 0x2A)
            d >> csc.mCrowd;
    } else {
        const DataNode *prop = Property("hide_crowd", false);
        if (!prop || prop->Int() == 0) {
            ObjDirItr<WorldCrowd> iter(Dir(), true);
            if (iter) {
                csc.mCrowd = iter;
            }
        }
    }
    if (d.rev > 32 && d.rev < 42)
        d >> (int &)csc.mCrowdRotate;
    if (d.rev >= 8 && d.rev < 42) {
        if (csc.mCrowd) {
            if (loc240 != csc.mCrowd->GetModifyStamp())
                csc.unk18.clear();
        } else if (loc240 != -1)
            csc.unk18.clear();
    }
    if (d.rev == 0xE) {
        float f244, f248, f24c;
        d >> f244;
        d >> f248;
        d >> f24c;
    }

    if (d.rev > 15 && d.rev < 18) {
        float f250, f254;
        bs >> f250;
        bs >> f254;
        for (int i = 0; i != mKeyframes.size(); i++) {
            mKeyframes[i].mShakeNoiseAmp = f254;
            mKeyframes[i].mShakeNoiseFreq = f250;
        }
    }
    if (d.rev > 0x10 && d.rev < 0x12) {
        Vector2 v210;
        bs >> v210;
        for (int i = 0; i != mKeyframes.size(); i++) {
            mKeyframes[i].mShakeMaxAngle = v210;
        }
    }
    if (d.rev > 0x13)
        d >> mGlowSpot;
    if (d.rev > 0x1D)
        d >> mDrawOverrides;
    if (d.rev > 0x1F)
        d >> mPostProcOverrides;
    if (d.rev > 0x23 && !(d.rev >= 47 && d.rev <= 48)) {
        d >> mPS3PerPixel;
    }
    if (d.rev > 0x24)
        d >> mFlags;
    Symbol s258;
    if (d.rev > 39 && d.rev < 43)
        d >> s258;
    if (d.rev < 0x2A) {
        if (csc.mCrowd)
            mCrowds.push_back(csc);
    } else
        d >> mCrowds;
    if (d.rev > 0x2A)
        d >> mAnims;
    if (d.rev > 0x33) {
        d >> mCrowdStateOverride;
    } else {
        static Symbol none("none");
        mCrowdStateOverride = none;
    }
    if (d.rev > 0x2A) {
        d >> mAnims;
    }

    if (!s258.Null()) {
        mAnims.push_back(Dir()->Find<RndAnimatable>(s258.Str(), false));
    }
    CacheFrames();
    if (mShotOver)
        DoHide();
END_LOADS

void CamShot::StartAnim() {
    CAMERA_LOG("** %s CamShot::StartAnim() start\n", Name());
    START_AUTO_TIMER("cam_switch");
    static Message msg("start_shot");
    Export(msg, true);
    WorldDir *crowdDir = GetCrowdDir();
    if (crowdDir) {
        CameraManager *camMgr = crowdDir->GetCameraManager();
        if (camMgr) {
            camMgr->SetCrowds(mCrowds);
        }
        if (TheHamWardrobe) {
            TheHamWardrobe->ForceCrowdAnimationStart(mCrowdStateOverride);
        }
    }
    mShotOver = false;
    mLastNext = 0;
    mLastPrev = 0;
    mShotStarted = true;
    unk210.Zero();
    unk230.Zero();
    unk250.Zero();
    unk220.Zero();
    unk240.Zero();
    unk260.Zero();
    StartAnims(mAnims);
    for (int i = 0; i != mCrowds.size(); i++) {
        CamShotCrowd &cur = mCrowds[i];
        if (cur.mCrowd) {
            cur.mCrowd->Set3DCharList(cur.unk18, cur.unk24);
        }
    }
    RndVelocityBuffer::Singleton().ResetFrame();
    DoHide();
    CAMERA_LOG("** %s CamShot::StartAnim() stop\n", Name());
}

void CamShot::EndAnim() {
    CAMERA_LOG("** %s CamShot::EndAnim() start\n", Name());
    UnHide();
    if (TheHamWardrobe) {
        TheHamWardrobe->ForceCrowdAnimationEnd();
    }
    static Message msg("stop_shot");
    Export(msg, true);
    EndAnims(mAnims);
    CAMERA_LOG("** %s CamShot::EndAnim() stop\n", Name());
}

void CamShot::SetFrame(float frame, float blend) {
    START_AUTO_TIMER("camera");
    if (unk283)
        return;
    RndAnimatable::SetFrame(frame, blend);
    RndCam *cam = GetCam();
    if (!cam)
        return;
    SetFrames(mAnims, frame);
    if (mKeyframes.empty())
        return;
    unk283 = true;
    mPathFrame = -1;
    EndFrame();
    static CamShotFrame nullFrame(nullptr);
    nullFrame.mCamShot = this;
    float f48 = 1.0f;
    CamShotFrame *frame4c = nullptr;
    CamShotFrame *frame50 = nullptr;
    GetKey(frame, frame4c, frame50, f48);
    if (mDisabled != 0) {
        frame50->UpdateTarget();
        if (frame4c)
            frame4c->UpdateTarget();
        unk283 = false;
    } else {
        if (frame50 != mLastNext) {
            frame50->UpdateTarget();
        }
        if (!frame4c) {
            nullFrame.Interp(*frame50, 1.0f, blend, cam);
        } else {
            if (frame4c != mLastPrev) {
                if (frame4c != mLastNext) {
                    frame4c->UpdateTarget();
                }
                mLastPrev = frame4c;
            }
            frame4c->Interp(*frame50, f48, blend, cam);
        }
        mLastNext = frame50;
        if (CheckShotStarted()) {
            static Message msg("shot_started");
            HandleType(msg);
            mShotStarted = false;
        }
        if (CheckShotOver(frame)) {
            SetShotOver();
        }
        unk283 = false;
    }
}

void CamShot::ListAnimChildren(std::list<RndAnimatable *> &children) const {
    FOREACH (it, mAnims) {
        children.push_back(*it);
    }
}

void CamShot::Init() {
    REGISTER_OBJ_FACTORY(CamShot)
    sAnimTarget = Hmx::Object::New<Hmx::Object>();
}

void CamShot::Disable(bool disable, int mask) {
    if (disable)
        mDisabled |= mask;
    else
        mDisabled &= ~mask;
}

bool CamShot::CheckShotStarted() { return mShotStarted; }

bool CamShot::CheckShotOver(float f) { return !mShotOver && !mLooping && f >= mDuration; }

bool CamShot::PlatformOk() const {
    if (TheLoadMgr.EditMode() || mPlatform == kPlatformNone
        || TheLoadMgr.GetPlatform() == kPlatformNone)
        return true;
    Platform plat = TheLoadMgr.GetPlatform();
    if (TheLoadMgr.GetPlatform() == kPlatformPC)
        plat = kPlatformXBox;
    return plat == mPlatform;
}

float CamShot::GetDurationSeconds() const {
    if (Units() == kTaskBeats) {
        return 0.0f;
    } else {
        MILO_ASSERT(Units() == kTaskSeconds, 0x5cc);
        return mDuration / 30.0f;
    }
}

void CamShot::CacheFrames() {
    float frames = 0.0f;
    for (int i = 0; i != mKeyframes.size(); i++) {
        CamShotFrame &curframe = mKeyframes[i];
        curframe.SetFrame(frames);
        frames += curframe.GetDuration() + curframe.GetBlend();
    }
    mDuration = frames;
}

DataNode CamShot::OnHasTargets(DataArray *da) {
    return mKeyframes[da->Int(2)].HasTargets();
}

DataNode CamShot::OnRadio(DataArray *da) {
    int i2 = da->Int(2);
    int i3 = da->Int(3);
    if (mFlags & i2) {
        mFlags &= ~i3;
        mFlags |= i2;
    }
    return 0;
}

bool CamShot::ShotOk(CamShot *shot) {
    static Message msg("shot_ok", 0);
    msg[0] = shot;
    DataNode handled = HandleType(msg);
    if (handled.Type() != kDataUnhandled) {
        if (handled.Type() == kDataString) {
            CAMERA_LOG("Shot %s rejected: %s.\n", Name(), handled.Str());
            return false;
        } else if (handled.Int() == 0) {
            CAMERA_LOG("Shot %s rejected: not ok.\n", Name());
            return false;
        } else {
            return true;
        }
    } else
        return true;
}

DataNode CamShot::OnSetPos(DataArray *da) {
    int idx = da->Int(2);
    return SetPos(mKeyframes[idx], RndCam::Current());
}

DataNode CamShot::OnClearCrowdChars(DataArray *da) {
    int idx = da->Int(2);
    MILO_ASSERT(idx < mCrowds.size(), 0xb03);
    mCrowds[idx].ClearCrowdChars();
    return 0;
}

DataNode CamShot::OnAddCrowdChars(DataArray *da) {
    int idx = da->Int(2);
    MILO_ASSERT(idx < mCrowds.size(), 0xb0b);
    mCrowds[idx].AddCrowdChars();
    return 0;
}

DataNode CamShot::OnSetCrowdChars(DataArray *da) {
    int idx = da->Int(2);
    MILO_ASSERT(idx < mCrowds.size(), 0xb13);
    mCrowds[idx].SetCrowdChars();
    return 0;
}

void CamShot::StartAnims(ObjPtrList<RndAnimatable> &anims) {
    FOREACH (it, anims) {
        (*it)->StartAnim();
    }
}

void CamShot::EndAnims(ObjPtrList<RndAnimatable> &anims) {
    FOREACH (it, anims) {
        (*it)->EndAnim();
    }
}

void CamShot::SetFrames(ObjPtrList<RndAnimatable> &anims, float frame) {
    FOREACH (it, anims) {
        (*it)->SetFrame(frame, 1);
    }
}

void CamShot::SetShotOver() {
    static Message msg("shot_over");
    Export(msg, true);
    mShotOver = true;
}

void CamShot::AddAnim(RndAnimatable *anim) {
    if (mAnims.find(anim) == mAnims.end()) {
        mAnims.push_back(anim);
    }
}

void CamShot::DoHide() {
    CAMERA_LOG("** %s CamShot::DoHide() start\n", Name());
    if (!mHidden) {
        mEndHideList.clear();
        mEndShowList.clear();
        FOREACH (it, mHideList) {
            RndDrawable *cur = *it;
            if (cur->Showing()) {
                cur->SetShowing(false);
                mEndShowList.push_back(cur);
                CAMERA_LOG("   ** %s hide from mHideList\n", cur->Name());
            }
        }
        FOREACH (it, mGenHideList) {
            RndDrawable *cur = *it;
            if (cur->Showing()) {
                cur->SetShowing(false);
                mEndShowList.push_back(cur);
                CAMERA_LOG("   ** %s hide from mEndShowList\n", cur->Name());
            }
        }
        FOREACH (it, mGenHideVector) {
            RndDrawable *cur = *it;
            if (cur->Showing()) {
                cur->SetShowing(false);
                CAMERA_LOG("   ** %s hide from mGenHideVector\n", cur->Name());
                mEndShowList.push_back(cur);
            }
        }
        FOREACH (it, mShowList) {
            RndDrawable *cur = *it;
            if (!cur->Showing()) {
                cur->SetShowing(true);
                mEndHideList.push_back(cur);
                CAMERA_LOG("   ** %s show from mEndShowList\n", cur->Name());
            }
        }
        mHidden = true;
    }
    CAMERA_LOG("** %s CamShot::DoHide() stop\n", Name());
}

void CamShot::UnHide() {
    CAMERA_LOG(" ** %s CamShot::UnHide() start\n", Name());
    if (mHidden) {
        FOREACH (it, mEndHideList) {
            (*it)->SetShowing(false);
            CAMERA_LOG("   ** %s hide from mEndHideList\n", (*it)->Name());
        }
        FOREACH (it, mEndShowList) {
            (*it)->SetShowing(true);
            CAMERA_LOG("   ** %s show from mEndShowList\n", (*it)->Name());
        }
        mEndHideList.clear();
        mEndShowList.clear();
        mHidden = false;
    }
    CAMERA_LOG(" ** %s CamShot::UnHide() stop\n", Name());
}

RndCam *CamShot::GetCam() {
    RndCam *ret = nullptr;
    WorldDir *crowdDir = GetCrowdDir();
    if (crowdDir) {
        if (crowdDir->Cam()) {
            ret = crowdDir->Cam();
        }
        if (!ret) {
            MILO_NOTIFY_ONCE("%s: paneldir but no cam", PathName(crowdDir));
        }
    } else {
        MILO_NOTIFY_ONCE("%s: no worlddir, so no cam", PathName(this));
    }
    return ret;
}

void CamShot::ClearCrowds() {
    for (auto it = mCrowds.begin(); it != mCrowds.end(); it) {
        if (!it->mCrowd) {
            it = mCrowds.erase(it);
        } else {
            ++it;
        }
    }
}

bool CamShot::AddCrowd(CamShotCrowd &crowd) {
    bool ret = true;
    FOREACH (it, mCrowds) {
        if (it->mCrowd == crowd.mCrowd) {
            ret = false;
            break;
        }
    }
    if (ret) {
        mCrowds.push_back(crowd);
    }
    return ret;
}
