#include "hamobj/HamSkeletonConverter.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/JointUtl.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/HamCharacter.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/Str.h"

HamSkeletonConverter::HamSkeletonConverter()
    : mBones(this), mSkeletonIndex(0), mCharacter(this), mIsActive(0), mIsCaptured(0),
      mCamPelvisHeight(0) {}

HamSkeletonConverter::~HamSkeletonConverter() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
}

BEGIN_HANDLERS(HamSkeletonConverter)
    HANDLE_ACTION(run_test, 0)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamSkeletonConverter)
    SYNC_PROP(bones, mBones)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamSkeletonConverter)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBones;
END_SAVES

BEGIN_COPYS(HamSkeletonConverter)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamSkeletonConverter)
    BEGIN_COPYING_MEMBERS
        mBones = c->mBones.Ptr();
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(HamSkeletonConverter)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mBones;
END_LOADS

void HamSkeletonConverter::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    mCharacter = dynamic_cast<HamCharacter *>(dir);
}

void HamSkeletonConverter::Enter() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (!handle.HasCallback(this)) {
        handle.AddCallback(this);
    }
    mPelvisMesh = mCharacter->Find<RndTransformable>("bone_pelvis.mesh", true);
    mPelvisMiloHeight = mPelvisMesh->LocalXfm().v.z;
    mMeshPtrs.clear();
    mMeshPtrs.resize(kNumJoints);
    for (int i = 0; i < kNumJoints; i++) {
        RndTransformable *t =
            mCharacter->Find<RndTransformable>(MirrorBoneName((SkeletonJoint)i), true);
        mMeshPtrs[i] = t;
    }
    Vector3 z = mMeshPtrs[kJointHipLeft]->WorldXfm().m.z;
    mFootRots[0] = z;
    z = mMeshPtrs[kJointHipRight]->WorldXfm().m.z;
    mFootRots[1] = z;
    mFootDestRots[0] = mFootRots[0];
    mFootDestRots[1] = mFootRots[1];
}

void HamSkeletonConverter::Exit() {
    RndPollable::Exit();
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
}

void HamSkeletonConverter::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mBones);
}

void HamSkeletonConverter::Highlight() {
    for (int i = 0; i < kNumJoints; i++) {
        Vector3 curV = mMiloJoints[i];
        UtilDrawSphere(curV, 1, Hmx::Color(0, 0, 1), nullptr);
        Transform curXfm = mMiloWorldXfms[i];
        Vector3 scaledX;
        Scale(curXfm.m.x, 4.0f, scaledX);
        Vector3 scaledY;
        Scale(curXfm.m.y, 4.0f, scaledY);
        Vector3 scaledZ;
        Scale(curXfm.m.z, 4.0f, scaledZ);

        Vector3 scaled;
        Add(scaledX, curXfm.v, scaled);
        TheRnd.DrawLine(curXfm.v, scaled, Hmx::Color(1, 0, 0), false);
        Add(scaledY, curXfm.v, scaled);
        TheRnd.DrawLine(curXfm.v, scaled, Hmx::Color(0, 1, 0), false);
        Add(scaledZ, curXfm.v, scaled);
        TheRnd.DrawLine(curXfm.v, scaled, Hmx::Color(0, 0, 1), false);
    }
}

void HamSkeletonConverter::PostUpdate(const SkeletonUpdateData *data) {
    if (mIsActive && data) {
        const BaseSkeleton *skeleton = nullptr;
        for (int i = 0; i < 6; i++) {
            if (data->mPlayerSkeletons[i] && data->mPlayerSkeletons[i]->IsTracked()) {
                skeleton = data->mPlayerSkeletons[i];
                break;
            }
        }
        Set(skeleton);
    }
}

void HamSkeletonConverter::GetParentWorldXfm(
    RndTransformable *mesh, Transform &target, SkeletonJoint parent
) {
    RndTransformable *meshParent = mesh->TransParent();
    if (streq(meshParent->Name(), "bone_pelvis.mesh")) {
        target.Set(mPelvisWorldXfm.m, mPelvisWorldXfm.v);
    } else if (IsSkeletonBone(meshParent->Name())) {
        MILO_ASSERT(streq(meshParent->Name(), CharBoneName(parent)), 0x2B2);
        target.Set(mMiloWorldXfms[parent].m, mMiloWorldXfms[parent].v);
    } else {
        GetParentWorldXfm(meshParent, target, parent);
        Multiply(meshParent->LocalXfm(), target, target);
    }
}

void HamSkeletonConverter::SetQuatBoneValue(String s, Hmx::Quat q) {
    String str(s);
    if (str.find(".mesh") != FixedString::npos) {
        str = str.substr(0, s.length() - 5);
    }
    str += ".quat";
    Hmx::Quat *qPtr = (Hmx::Quat *)mBones->FindPtr(str.c_str());
    // this is stupid but hey if it matches lmao
    qPtr->w = q.w;
    qPtr->x = q.x;
    qPtr->y = q.y;
    qPtr->z = q.z;
}

void HamSkeletonConverter::SetRotzBoneValue(String s, float r) {
    String str(s);
    if (str.find(".mesh") != FixedString::npos) {
        str = str.substr(0, s.length() - 5);
    }
    str += ".rotz";
    float *rPtr = (float *)mBones->FindPtr(str.c_str());
    *rPtr = r;
}

void HamSkeletonConverter::SetPosBoneValue(String s, Vector3 v) {
    String str(s);
    if (str.find(".mesh") != FixedString::npos) {
        str = str.substr(0, s.length() - 5);
    }
    str += ".pos";
    Vector3 *vPtr = (Vector3 *)mBones->FindPtr(str.c_str());
    // this is stupid but hey if it matches lmao
    vPtr->x = v.x;
    vPtr->y = v.y;
    vPtr->z = v.z;
}

void HamSkeletonConverter::ScaleBone(
    SkeletonJoint parent,
    SkeletonJoint child,
    SkeletonCoordSys cs,
    const Vector3 &parentPos,
    const Vector3 &childPos,
    const Vector3 &newParentPos,
    Vector3 &outChildPos
) {
    float len = Distance(parentPos, childPos);
    RndTransformable *t1 = mMeshPtrs[parent];
    RndTransformable *t2 = mMeshPtrs[child];
    float tlen = Distance(t1->WorldXfm().v, t2->WorldXfm().v);
    Vector3 diff;
    Subtract(childPos, parentPos, diff);
    Scale(diff, tlen / len, diff);
    Add(diff, newParentPos, outChildPos);
}

void HamSkeletonConverter::CalcRotzBone(
    SkeletonJoint parent, SkeletonJoint rotzJoint, SkeletonJoint child
) {
    Vector3 vd0;
    Subtract(mMiloJoints[rotzJoint], mMiloJoints[parent], vd0);
    Normalize(vd0, vd0);
    Vector3 vc0;
    Subtract(mMiloJoints[child], mMiloJoints[rotzJoint], vc0);
    Normalize(vc0, vc0);
    float dot = -acosf(Dot(vc0, vd0));
    if (!IsNaN(dot)) {
        RndTransformable *t = mMeshPtrs[rotzJoint];
        Hmx::Matrix3 mb0 = t->LocalXfm().m;
        MakeRotMatrixZ(dot, mb0);
        Multiply(
            Transform(mb0, t->LocalXfm().v),
            mMiloWorldXfms[parent],
            mMiloWorldXfms[rotzJoint]
        );
        SetRotzBoneValue(MirrorBoneName(rotzJoint), dot);
    }
}

void HamSkeletonConverter::CalcQuatBone(
    SkeletonJoint parent, SkeletonJoint boneToSet, SkeletonJoint child
) {
    Vector3 v1b0;
    Subtract(mMiloJoints[child], mMiloJoints[boneToSet], v1b0);
    Normalize(v1b0, v1b0);
    RndTransformable *t = mMeshPtrs[boneToSet];
    Transform tf180 = t->LocalXfm();
    Transform tff0;
    GetParentWorldXfm(t, tff0, parent);
    Multiply(tf180, tff0, tf180);
    Hmx::Quat q1a0;
    MakeRotQuat(tf180.m.x, v1b0, q1a0);
    Transform tf130;
    Multiply(tf180.m.x, q1a0, tf130.m.x);
    Multiply(tf180.m.y, q1a0, tf130.m.y);
    Multiply(tf180.m.z, q1a0, tf130.m.z);
    Normalize(tf130.m, tf130.m);
    tf130.v = tf180.v;
    mMiloWorldXfms[boneToSet].Set(tf130.m, tf180.v);
    Transform tfb0;
    Invert(tff0, tfb0);
    Transform tf70;
    Multiply(tf130, tfb0, tf70);
    Hmx::Quat q190(tf70.m);
    SetQuatBoneValue(CharBoneName(boneToSet), q190);
}

void HamSkeletonConverter::RotateTowards(
    const Vector3 &src, const Vector3 &dst, float radianLimit, Vector3 &out
) {
    if (src == dst)
        return;
    else {
        Hmx::Quat q50;
        q50.Reset();
        Hmx::Quat q40;
        MakeRotQuat(src, dst, q40);
        float angle = acos(Dot(src, dst));
        if (IsNaN(angle) || fabsf(angle) < 1e-9) {
            out.x = src.x;
            out.y = src.y;
            out.z = src.z;
        } else {
            float fabsed = fabsf(radianLimit / angle);
            if (fabsed >= 1.0f) {
                out.x = dst.x;
                out.y = dst.y;
                out.z = dst.z;
            } else {
                Interp(q50, q40, fabsed, q40);
                Multiply(src, q40, out);
            }
        }
    }
}

void HamSkeletonConverter::SetArm(
    SkeletonJoint parent, SkeletonJoint shoulder, SkeletonJoint elbow, SkeletonJoint hand
) {
    Vector3 v1d0;
    Subtract(mMiloJoints[elbow], mMiloJoints[shoulder], v1d0);
    Normalize(v1d0, v1d0);

    Vector3 vsub;
    Subtract(mMiloJoints[elbow], mMiloJoints[hand], vsub);
    Vector3 v1c0;
    Cross(v1d0, vsub, v1c0);
    Normalize(v1c0, v1c0);
    Vector3 v1b0;
    Cross(v1c0, v1d0, v1b0);
    Normalize(v1b0, v1b0);

    Transform tf110;
    GetParentWorldXfm(mMeshPtrs[shoulder], tf110, parent);
    Vector3 v190;
    Multiply(mMeshPtrs[shoulder]->LocalXfm().v, tf110, v190);

    Hmx::Matrix3 m180(v1d0, v1b0, v1c0);
    Transform tf150(m180, v190);
    mMiloWorldXfms[shoulder].Set(m180, v190);

    Transform tfd0;
    Invert(tf110, tfd0);
    Multiply(tf150, tfd0, tf150);
    NormalizeAboutX(tf150.m);
    Hmx::Quat q1a0(tf150.m);
    SetQuatBoneValue(MirrorBoneName(shoulder), q1a0);
    CalcRotzBone(shoulder, elbow, hand);
}

void HamSkeletonConverter::SetLeg(
    SkeletonJoint parent,
    SkeletonJoint hip,
    SkeletonJoint knee,
    SkeletonJoint ankle,
    SkeletonJoint foot,
    const BaseSkeleton *skeleton,
    int index
) {
    Vector3 v210;
    Subtract(mMiloJoints[knee], mMiloJoints[hip], v210);
    Normalize(v210, v210);

    Vector3 v200;
    Subtract(mMiloJoints[ankle], mMiloJoints[knee], v200);
    Normalize(v200, v200);
    float dot = -acosf(Dot(v200, v210));
    if (!IsNaN(dot)) {
        Transform tf100;
        GetParentWorldXfm(mMeshPtrs[hip], tf100, parent);

        Plane p200;
        p200.Set(mMiloJoints[hip], mMiloJoints[knee], mMiloJoints[ankle]);
        int cmp = dot < 0.2;
        if (abs(cmp)) {
            Vector3 vsub;
            Subtract(mMiloJoints[hip], mMiloJoints[knee], vsub);
            p200.Set(vsub, mPelvisWorldXfm.m.z);
        }

        mFootDestRots[index].Set(p200.a * -1, p200.b * -1, p200.c * -1);

        Vector3 v1c0;
        Multiply(mMeshPtrs[hip]->LocalXfm().v, tf100, v1c0);

        Subtract(mMiloJoints[knee], mMiloJoints[hip], v210);
        Normalize(v210, v210);
        RotateTowards(mFootRots[index], mFootDestRots[index], 1000, mFootRots[index]);

        Vector3 v1f0;
        Vector3 v730i7(mFootRots[index].x, mFootRots[index].y, mFootRots[index].z);
        Cross(v730i7, v210, v1f0);
        Normalize(v1f0, v1f0);

        Hmx::Matrix3 m1b0(v210, v1f0, v730i7);
        Transform tf180(m1b0, v1c0);
        mMiloWorldXfms[hip].Set(m1b0, v1c0);

        Transform tfc0;
        Invert(tf100, tfc0);
        Multiply(tf180, tfc0, tf180);
        NormalizeAboutX(tf180.m);
        Hmx::Quat q1d0(tf180.m);
        SetQuatBoneValue(MirrorBoneName(hip), q1d0);

        RndTransformable *t = mMeshPtrs[knee];
        Hmx::Matrix3 m = t->LocalXfm().m;
        MakeRotMatrixZ(dot, m);
        Multiply(Transform(m, t->LocalXfm().v), mMiloWorldXfms[hip], mMiloWorldXfms[knee]);
        SetRotzBoneValue(MirrorBoneName(knee), dot);
    }
}
