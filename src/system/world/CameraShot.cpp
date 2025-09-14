#include "world/CameraShot.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Str.h"
#include <cstdlib>

inline float ComputeFOVScale(float fov) {
    return 24.0f / (float(std::tan(fov / 2.0f)) * 2.0f);
}
inline float ScaleToFOV(float scale) {
    return float(std::atan(24.0f / (scale * 2.0f))) * 2.0f;
}

CamShotFrame::CamShotFrame(Hmx::Object *owner)
    : mDuration(0), mBlend(0), mBlendEase(0), mBlendEaseMode(kBlendEaseInAndOut),
      unk10(-1), mFOV(1.2217305f), mZoomFOV(0), mShakeNoiseFreq(0), mShakeNoiseAmp(0),
      mShakeMaxAngle(0, 0), mBlurDepth(0.35), mMaxBlur(1), mMinBlur(0),
      mFocusBlurMultiplier(0), mTargets(owner), mParent(owner), mFocalTarget(owner),
      mUseParentRotation(false), mParentFirstFrame(false),
      mCamShot(dynamic_cast<CamShot *>(owner)) {
    mWorldOffset.Reset();
    mScreenOffset.Zero();
    mLastTargetPos.x = kHugeFloat;
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

bool CamShotFrame::SameTargets(const CamShotFrame &other) const {
    if (mTargets.size() != other.mTargets.size())
        return false;
    for (ObjPtrList<RndTransformable>::iterator it = mTargets.begin();
         it != mTargets.end();
         ++it) {
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
    for (ObjPtrList<RndTransformable>::iterator it = mTargets.begin();
         it != mTargets.end();
         ++it) {
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
    for (ObjPtrList<RndTransformable>::iterator it = mTargets.begin();
         it != mTargets.end();
         ++it) {
        if (*it) {
            xfm.LookAt(mLastTargetPos, xfm.m.z);
            break;
        }
    }
    Vector3 v;
    Subtract(xfm.v, mLastTargetPos, v);
    float length = Length(v);
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
    // writes whatever unkd4 is of mCrowd
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

BEGIN_CUSTOM_PROPSYNC(CamShotCrowd)
    SYNC_PROP_MODIFY(crowd, o.mCrowd, o.unk18.clear())
    SYNC_PROP(crowd_rotate, (int &)o.mCrowdRotate)
END_CUSTOM_PROPSYNC

CamShot::CamShot()
    : mKeyframes(this), unk104(this), unk118(this), unk134(this), unk148(this),
      unk15c(this), unk17c(this), unk190(this), unk1a4(this), unk1b8(this), unk1d0(this),
      unk1e8(this), unk1fc(this) {}

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
    SYNC_PROP(looping, unke0)
    SYNC_PROP(loop_keyframe, unke4)
    SYNC_PROP_SET(category, unkfc, unkfc = _val.ForceSym())
    SYNC_PROP(filter, unkf4)
    SYNC_PROP(clamp_height, unkf8)
    SYNC_PROP(near_plane, unke8)
    SYNC_PROP(far_plane, unkec) {
        static Symbol _s("duration");
        if (sym == _s && _op & kPropGet)
            return PropSync(unk278, _val, _prop, _i + 1, _op);
    }
    SYNC_PROP(use_depth_of_field, unkf0)
    SYNC_PROP(path, unk118)
    SYNC_PROP(path_frame, unk12c)
    SYNC_PROP(platform_only, unk130)
    SYNC_PROP_LIST(hide_list, unk134)
    SYNC_PROP_LIST(show_list, unk148)
    SYNC_PROP_LIST(gen_hide_list, unk15c)
    SYNC_PROP(draw_overrides, unk17c)
    SYNC_PROP(postproc_overrides, unk190)
    SYNC_PROP(glow_spot, unk1d0)
    SYNC_PROP(crowds, unk1b8)
    SYNC_PROP(crowd_state_override, unk1c8)
    SYNC_PROP(ps3_per_pixel, unk1cc)
    SYNC_PROP_BITFIELD(flags, unk1e4, 0xB94)
    SYNC_PROP_SET(disabled, unk27c, )
    SYNC_PROP(anims, unk104)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS
