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
    : mBones(this), unk28(0), unk2c(this), unk750(0), unk751(0), unk754(0) {}

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
    unk2c = dynamic_cast<HamCharacter *>(dir);
}

void HamSkeletonConverter::Enter() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (!handle.HasCallback(this)) {
        handle.AddCallback(this);
    }
    unk6cc = unk2c->Find<RndTransformable>("bone_pelvis.mesh", true);
    unk758 = unk6cc->LocalXfm().v.z;
    mBoneTrans.clear();
    mBoneTrans.resize(kNumJoints);
    for (int i = 0; i < kNumJoints; i++) {
        RndTransformable *t =
            unk2c->Find<RndTransformable>(MirrorBoneName((SkeletonJoint)i), true);
        mBoneTrans[i] = t;
    }
    Vector3 z = mBoneTrans[kJointHipLeft]->WorldXfm().m.z;
    unk730[0] = z;
    z = mBoneTrans[kJointHipRight]->WorldXfm().m.z;
    unk730[1] = z;
    unk710[0] = unk730[0];
    unk710[1] = unk730[1];
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
        Vector3 curV = mJointPos[i];
        UtilDrawSphere(curV, 1, Hmx::Color(0, 0, 1), nullptr);
        Transform curXfm = mBoneXfms[i];
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
    if (unk750 && data) {
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
    RndTransformable *t, Transform &xfm, SkeletonJoint parent
) {
    RndTransformable *meshParent = t->TransParent();
    if (streq(meshParent->Name(), "bone_pelvis.mesh")) {
        xfm.Set(unk6d0.m, unk6d0.v);
    } else if (IsSkeletonBone(meshParent->Name())) {
        MILO_ASSERT(streq(meshParent->Name(), CharBoneName(parent)), 0x2B2);
        xfm.Set(mBoneXfms[parent].m, mBoneXfms[parent].v);
    } else {
        GetParentWorldXfm(meshParent, xfm, parent);
        Multiply(meshParent->LocalXfm(), xfm, xfm);
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
    SkeletonJoint j1,
    SkeletonJoint j2,
    SkeletonCoordSys cs3,
    const Vector3 &v4,
    const Vector3 &v5,
    const Vector3 &v6,
    Vector3 &v7
) {
    float len = Distance(v4, v5);
    RndTransformable *t1 = mBoneTrans[j1];
    RndTransformable *t2 = mBoneTrans[j2];
    float tlen = Distance(t1->WorldXfm().v, t2->WorldXfm().v);
    Vector3 diff;
    Subtract(v5, v4, diff);
    Scale(diff, tlen / len, diff);
    Add(diff, v6, v7);
}

void HamSkeletonConverter::CalcRotzBone(
    SkeletonJoint j1, SkeletonJoint j2, SkeletonJoint j3
) {
    Vector3 vd0;
    Subtract(mJointPos[j2], mJointPos[j1], vd0);
    Normalize(vd0, vd0);
    Vector3 vc0;
    Subtract(mJointPos[j3], mJointPos[j2], vc0);
    Normalize(vc0, vc0);
    float dot = -acosf(Dot(vc0, vd0));
    if (!IsNaN(dot)) {
        RndTransformable *t = mBoneTrans[j2];
        Hmx::Matrix3 mb0 = t->LocalXfm().m;
        MakeRotMatrixZ(dot, mb0);
        Multiply(Transform(mb0, t->LocalXfm().v), mBoneXfms[j1], mBoneXfms[j2]);
        SetRotzBoneValue(MirrorBoneName(j2), dot);
    }
}

void HamSkeletonConverter::CalcQuatBone(
    SkeletonJoint j1, SkeletonJoint j2, SkeletonJoint j3
) {
    Vector3 v1b0;
    Subtract(mJointPos[j3], mJointPos[j2], v1b0);
    Normalize(v1b0, v1b0);
    RndTransformable *t = mBoneTrans[j2];
    Transform tf180 = t->LocalXfm();
    Transform tff0;
    GetParentWorldXfm(t, tff0, j1);
    Multiply(tf180, tff0, tf180);
    Hmx::Quat q1a0;
    MakeRotQuat(tf180.m.x, v1b0, q1a0);
    Transform tf130;
    Multiply(tf180.m.x, q1a0, tf130.m.x);
    Multiply(tf180.m.y, q1a0, tf130.m.y);
    Multiply(tf180.m.z, q1a0, tf130.m.z);
    Normalize(tf130.m, tf130.m);
    tf130.v = tf180.v;
    mBoneXfms[j2].Set(tf130.m, tf180.v);
    Transform tfb0;
    Invert(tff0, tfb0);
    Transform tf70;
    Multiply(tf130, tfb0, tf70);
    Hmx::Quat q190(tf70.m);
    SetQuatBoneValue(CharBoneName(j2), q190);
}

void HamSkeletonConverter::RotateTowards(
    const Vector3 &v1, const Vector3 &v2, float f, Vector3 &vout
) {
    if (v1 == v2)
        return;
    else {
        Hmx::Quat q50;
        q50.Reset();
        Hmx::Quat q40;
        MakeRotQuat(v1, v2, q40);
        float angle = acos(Dot(v1, v2));
        if (IsNaN(angle) || fabsf(angle) < 1e-9) {
            vout.x = v1.x;
            vout.y = v1.y;
            vout.z = v1.z;
        } else {
            float fabsed = fabsf(f / angle);
            if (fabsed >= 1.0f) {
                vout.x = v2.x;
                vout.y = v2.y;
                vout.z = v2.z;
            } else {
                Interp(q50, q40, fabsed, q40);
                Multiply(v1, q40, vout);
            }
        }
    }
}

void HamSkeletonConverter::SetArm(
    SkeletonJoint j1, SkeletonJoint j2, SkeletonJoint j3, SkeletonJoint j4
) {
    Vector3 v1d0;
    Subtract(mJointPos[j3], mJointPos[j2], v1d0);
    Normalize(v1d0, v1d0);

    Vector3 vsub;
    Subtract(mJointPos[j3], mJointPos[j4], vsub);
    Vector3 v1c0;
    Cross(v1d0, vsub, v1c0);
    Normalize(v1c0, v1c0);
    Vector3 v1b0;
    Cross(v1c0, v1d0, v1b0);
    Normalize(v1b0, v1b0);

    Transform tf110;
    GetParentWorldXfm(mBoneTrans[j2], tf110, j1);
    Vector3 v190;
    Multiply(mBoneTrans[j2]->LocalXfm().v, tf110, v190);

    Hmx::Matrix3 m180(v1d0, v1b0, v1c0);
    Transform tf150(m180, v190);
    mBoneXfms[j2].Set(m180, v190);

    Transform tfd0;
    Invert(tf110, tfd0);
    Multiply(tf150, tfd0, tf150);
    NormalizeAboutX(tf150.m);
    Hmx::Quat q1a0(tf150.m);
    SetQuatBoneValue(MirrorBoneName(j2), q1a0);
    CalcRotzBone(j2, j3, j4);
}

void HamSkeletonConverter::SetLeg(
    SkeletonJoint j1,
    SkeletonJoint j2,
    SkeletonJoint j3,
    SkeletonJoint j4,
    SkeletonJoint j5,
    const BaseSkeleton *skeleton,
    int i7
) {
    Vector3 v210;
    Subtract(mJointPos[j3], mJointPos[j2], v210);
    Normalize(v210, v210);

    Vector3 v200;
    Subtract(mJointPos[j4], mJointPos[j3], v200);
    Normalize(v200, v200);
    float dot = -acosf(Dot(v200, v210));
    if (!IsNaN(dot)) {
        Transform tf100;
        GetParentWorldXfm(mBoneTrans[j2], tf100, j1);

        Plane p200;
        p200.Set(mJointPos[j2], mJointPos[j3], mJointPos[j4]);
        int cmp = dot < 0.2;
        if (abs(cmp)) {
            Vector3 vsub;
            Subtract(mJointPos[j2], mJointPos[j3], vsub);
            p200.Set(vsub, unk6d0.m.z);
        }

        unk710[i7].Set(p200.a * -1, p200.b * -1, p200.c * -1);

        Vector3 v1c0;
        Multiply(mBoneTrans[j2]->LocalXfm().v, tf100, v1c0);

        Subtract(mJointPos[j3], mJointPos[j2], v210);
        Normalize(v210, v210);
        RotateTowards(unk730[i7], unk710[i7], 1000, unk730[i7]);

        Vector3 v1f0;
        Vector3 v730i7(unk730[i7].x, unk730[i7].y, unk730[i7].z);
        Cross(v730i7, v210, v1f0);
        Normalize(v1f0, v1f0);

        Hmx::Matrix3 m1b0(v210, v1f0, v730i7);
        Transform tf180(m1b0, v1c0);
        mBoneXfms[j2].Set(m1b0, v1c0);

        Transform tfc0;
        Invert(tf100, tfc0);
        Multiply(tf180, tfc0, tf180);
        NormalizeAboutX(tf180.m);
        Hmx::Quat q1d0(tf180.m);
        SetQuatBoneValue(MirrorBoneName(j2), q1d0);

        RndTransformable *t = mBoneTrans[j3];
        Hmx::Matrix3 m = t->LocalXfm().m;
        MakeRotMatrixZ(dot, m);
        Multiply(Transform(m, t->LocalXfm().v), mBoneXfms[j2], mBoneXfms[j3]);
        SetRotzBoneValue(MirrorBoneName(j3), dot);
    }
}
