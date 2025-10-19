#include "rndobj/Morph.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/BinStream.h"

RndMorph::RndMorph()
    : mPoses(this), mTarget(this), mNormals(false), mSpline(false), mIntensity(1) {}

BEGIN_HANDLERS(RndMorph)
    HANDLE(set_target, OnSetTarget)
    HANDLE(set_pose_weight, OnSetPoseWeight)
    HANDLE(set_pose_mesh, OnSetPoseMesh)
    HANDLE(set_intensity, OnSetIntensity)
    HANDLE_ACTION(set_num_poses, SetNumPoses(_msg->Int(2)))
    HANDLE(pose_mesh, OnPoseMesh)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMorph)
    SYNC_PROP(target, mTarget)
    SYNC_PROP_SET(num_poses, NumPoses(), SetNumPoses(_val.Int()))
    SYNC_PROP(intensity, mIntensity)
    SYNC_PROP(normals, mNormals)
    SYNC_PROP(spline, mSpline)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const RndMorph::Pose &p) {
    bs << p.mesh << p.weights;
    return bs;
}

BEGIN_SAVES(RndMorph)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mPoses << mTarget << mNormals << mSpline << mIntensity;
END_SAVES

BEGIN_COPYS(RndMorph)
    CREATE_COPY_AS(RndMorph, f)
    MILO_ASSERT(f, 0xA8);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(f, mPoses)
    COPY_MEMBER_FROM(f, mTarget)
    COPY_MEMBER_FROM(f, mNormals)
    COPY_MEMBER_FROM(f, mSpline)
    COPY_MEMBER_FROM(f, mIntensity)
END_COPYS

BinStreamRev &operator>>(BinStreamRev &bs, RndMorph::Pose &pose) {
    bs >> pose.mesh;
    if (bs.rev < 2) {
        Keys<Weight, Weight> weightKeys;
        bs >> weightKeys;
        pose.weights.resize(weightKeys.size());
        Keys<Weight, Weight>::iterator it = weightKeys.begin();
        Keys<float, float>::iterator poseIt = pose.weights.begin();
        for (; it != weightKeys.end(); ++it, ++poseIt) {
            Key<Weight> &curWeight = *it;
            Key<float> &curKey = *poseIt;
            curKey.frame = curWeight.frame;
            curKey.value = curWeight.value.weight;
        }
    } else {
        bs >> pose.weights;
    }
    return bs;
}

BEGIN_LOADS(RndMorph)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    if (d.rev > 3) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndAnimatable)
    d >> mPoses >> mTarget;
    if (d.rev > 0) {
        d >> mNormals >> mSpline;
    }
    if (d.rev > 2) {
        d >> mIntensity;
    }
END_LOADS

TextStream &operator<<(TextStream &ts, const RndMorph::Pose &pose) {
    ts << "mesh:" << pose.mesh << " weightKeys:" << pose.weights;
    return ts;
}

void RndMorph::Print() {
    TheDebug << "   poses: " << mPoses << "\n";
    TheDebug << "   target: " << mTarget << "\n";
    TheDebug << "   normals: " << mNormals << "\n";
    TheDebug << "   spline: " << mSpline << "\n";
    TheDebug << "   intensity: " << mIntensity << "\n";
}

DataNode RndMorph::OnSetIntensity(const DataArray *arr) {
    SetIntensity(arr->Float(2));
    return 0;
}

DataNode RndMorph::OnPoseMesh(const DataArray *arr) {
    return PoseAt(arr->Int(2)).mesh.Ptr();
}

DataNode RndMorph::OnSetTarget(const DataArray *arr) {
    SetTarget(arr->Obj<RndMesh>(2));
    return 0;
}

DataNode RndMorph::OnSetPoseMesh(const DataArray *arr) {
    PoseAt(arr->Int(2)).mesh = arr->Obj<RndMesh>(3);
    return 0;
}

DataNode RndMorph::OnSetPoseWeight(const DataArray *arr) {
    Keys<float, float> &curWeights = PoseAt(arr->Int(2)).weights;
    float frame = arr->Float(3);
    float weightVal = arr->Float(4);
    Keys<float, float>::iterator it = curWeights.begin();
    for (; it != curWeights.end(); ++it) {
        if (it->frame == frame) {
            it->value = weightVal;
            break;
        }
    }
    if (it == curWeights.end()) {
        curWeights.Add(weightVal, frame, false);
    }
    return 0;
}
