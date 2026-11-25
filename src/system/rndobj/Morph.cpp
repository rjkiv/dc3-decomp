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
    bs << p.mesh << p.weightKeys;
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

BinStreamRev &operator>>(BinStreamRev &d, RndMorph::Pose &pose) {
    d >> pose.mesh;
    if (d.rev < 2) {
        Keys<Weight, Weight> weightKeys;
        d >> weightKeys;
        pose.weightKeys.resize(weightKeys.size());
        Keys<Weight, Weight>::iterator it = weightKeys.begin();
        Keys<float, float>::iterator poseIt = pose.weightKeys.begin();
        for (; it != weightKeys.end(); ++it, ++poseIt) {
            Key<Weight> &curWeight = *it;
            Key<float> &curKey = *poseIt;
            curKey.frame = curWeight.frame;
            curKey.value = curWeight.value.weight;
        }
    } else {
        d >> pose.weightKeys;
    }
    return d;
}

INIT_REVS(4, 0)

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

FORCEINLINE TextStream &operator<<(TextStream &ts, const RndMorph::Pose &pose) {
    ts << "mesh:" << pose.mesh << " weightKeys:" << pose.weightKeys;
    return ts;
}

void RndMorph::Print() {
    TheDebug << "   poses: " << mPoses << "\n";
    TheDebug << "   target: " << mTarget << "\n";
    TheDebug << "   normals: " << mNormals << "\n";
    TheDebug << "   spline: " << mSpline << "\n";
    TheDebug << "   intensity: " << mIntensity << "\n";
}

void RndMorph::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (!mTarget || mPoses.empty())
        return;
    else {
        float f1 = 1.0f;
        bool b3 = true;
        RndMesh::Vert *vertEnd = mTarget->Verts().end();
        if (blend != 1.0f) {
            b3 = false;
            float scalar = 1.0f - blend;
            for (RndMesh::Vert *it = mTarget->Verts().begin(); it != vertEnd; ++it) {
                it->pos *= scalar;
                if (mNormals) {
                    it->norm *= scalar;
                }
            }
        }
        for (auto it = mPoses.rbegin(); it != mPoses.rend(); ++it) {
            if (it->mesh) {
                RndMesh::Vert *targetIt = mTarget->Verts().begin();
                RndMesh::Vert *itVert = it->mesh->Verts().begin();
                RndMesh::Vert *itVertEnd = it->mesh->Verts().end();
                float intenseInterp;
                if (it + 1 == mPoses.rend()) {
                    intenseInterp = f1;
                } else {
                    intenseInterp = mIntensity * InterpWeight(it->weightKeys, frame);
                    f1 = f1 - intenseInterp;
                }
                if (intenseInterp != 0) {
                    intenseInterp *= blend;
                    if (b3) {
                        b3 = false;
                        for (; targetIt != vertEnd && itVert != itVertEnd;
                             ++targetIt, ++itVert) {
                            Scale(itVert->pos, intenseInterp, targetIt->pos);
                            if (mNormals) {
                                Scale(itVert->norm, intenseInterp, targetIt->norm);
                            }
                        }
                    } else {
                        for (; targetIt != vertEnd && itVert != itVertEnd;
                             ++targetIt, ++itVert) {
                            ScaleAddEq(targetIt->pos, itVert->pos, intenseInterp);
                            if (mNormals) {
                                ScaleAddEq(targetIt->norm, itVert->norm, intenseInterp);
                            }
                        }
                    }
                }
            }
        }
        if (mNormals) {
            for (RndMesh::Vert *it = mTarget->Verts().begin(); it != vertEnd; ++it) {
                Normalize(it->norm, it->norm);
            }
        }
        mTarget->Sync(RndMesh::kVerts);
    }
}

float RndMorph::EndFrame() {
    float f4 = 0;
    for (int i = 0; i < mPoses.size(); i++) {
        auto &weightKeys = mPoses[i].weightKeys;
        float cur;
        if (weightKeys.size()) {
            cur = weightKeys.back().frame;
        } else {
            cur = 0;
        }
        if (cur > f4) {
            f4 = cur;
        }
    }
    return f4;
}

float RndMorph::InterpWeight(const Keys<float, float> &weightKeys, float frame) {
    const Key<float> *prev;
    const Key<float> *next;
    float r;

    weightKeys.AtFrame(frame, prev, next, r);
    if (prev) {
        if (mSpline) {
            float prevVal = prev->value;
            float ret = 0;
            if (prevVal || next->value) {
                float f2 = prevVal * 2.0f - next->value * 2.0f;
                ret = r * r * (f2 * r + f2 * -3.0f * 0.5f) + prevVal;
            }
            return ret;
        } else {
            return Interp(prev->value, next->value, r);
        }
    } else {
        return 0;
    }
}

DataNode RndMorph::OnSetIntensity(const DataArray *arr) {
    SetIntensity(arr->Float(2));
    return 0;
}

DataNode RndMorph::OnPoseMesh(const DataArray *arr) {
    return GetPose(arr->Int(2)).mesh.Ptr();
}

DataNode RndMorph::OnSetTarget(const DataArray *arr) {
    SetTarget(arr->Obj<RndMesh>(2));
    return 0;
}

DataNode RndMorph::OnSetPoseMesh(const DataArray *arr) {
    GetPose(arr->Int(2)).mesh = arr->Obj<RndMesh>(3);
    return 0;
}

DataNode RndMorph::OnSetPoseWeight(const DataArray *arr) {
    Keys<float, float> &curWeights = GetPose(arr->Int(2)).weightKeys;
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
