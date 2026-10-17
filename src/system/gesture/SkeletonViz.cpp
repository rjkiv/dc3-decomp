#include "gesture/SkeletonViz.h"
#include "SkeletonViz.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamCharacter.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Line.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"

static const float kNatalVerticalFOV = 45.58f;

float dummyskeletonvizlmao() { return kNatalVerticalFOV; }

SkeletonViz::SkeletonViz()
    : mUsePhysicalCam(0), mPhysicalCamRotation(0), mDesiredCamRotation(0),
      mAxesCoordSys(kCoordCamera), mUtlLine(0), mResource(nullptr), mEnviron(0),
      mCamMesh(0), mJointMesh(0), mJointMat(0), mPhysicalCam(0), mDrawScale(1),
      mUseParentXfm(true) {
    // what even is happening here
    Vector3 x, y, z;
    x.Set(1, 0, 0);
    y.Set(0, 0, 1);
    z.Set(0, 1, 0);
    mDefaultNatalToWorld.Reset();
    Hmx::Matrix3 rot(x, y, z);
    Multiply(rot, mDefaultNatalToWorld.m, mDefaultNatalToWorld.m);
    mNatalToWorld = mDefaultNatalToWorld;
    for (int i = 0; i < kNumBones; i++) {
        mLines[i] = nullptr;
    }
}

SkeletonViz::~SkeletonViz() {
    for (int i = 0; i < kNumBones; i++) {
        delete mLines[i];
    }
}

BEGIN_HANDLERS(SkeletonViz)
    HANDLE_ACTION(rotate, Rotate(_msg->Float(2)))
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(SkeletonViz)
    SYNC_PROP(use_physical_cam, mUsePhysicalCam)
    SYNC_PROP_SET(
        physical_cam_rotation, mPhysicalCamRotation, SetPhysicalCamRotation(_val.Float())
    )
    SYNC_PROP_SET(
        axes_coord_sys, mAxesCoordSys, SetAxesCoordSys((SkeletonCoordSys)_val.Int())
    )
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SkeletonViz)
    SAVE_REVS(6, 1)
    SAVE_SUPERCLASS(RndPollable)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mUsePhysicalCam;
    bs << mAxesCoordSys;
    bs << mPhysicalCamRotation;
END_SAVES

BEGIN_COPYS(SkeletonViz)
    COPY_SUPERCLASS(RndPollable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(SkeletonViz)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mUsePhysicalCam)
        COPY_MEMBER(mAxesCoordSys)
        COPY_MEMBER(mPhysicalCamRotation)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(SkeletonViz)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(6, 1)

void SkeletonViz::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(6, 1)
    if (d.rev > 5) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndDrawable)
    LOAD_SUPERCLASS(RndTransformable)
    if (d.rev > 0) {
        d >> mUsePhysicalCam;
    }
    if (d.rev > 1) {
        int cs;
        d >> cs;
        mAxesCoordSys = (SkeletonCoordSys)cs;
    }
    if (d.rev > 2 && d.rev < 4) {
        int x, y;
        d >> x;
        d >> y;
    }
    if (d.altRev > 0) {
        d >> mPhysicalCamRotation;
    }
    mDesiredCamRotation = mPhysicalCamRotation;
    if (d.rev > 4 && d.altRev < 1) {
        ObjPtr<HamCharacter> hChar(this);
        d >> hChar;
    }
    if (TheLoadMgr.EditMode()) {
        LoadResource(true);
    }
}

void SkeletonViz::PostLoad(BinStream &bs) {
    if (TheLoadMgr.EditMode()) {
        mResource.PostLoad(nullptr);
        UpdateResource();
    }
}

float SkeletonViz::PhysicalCamRotation() const { return mPhysicalCamRotation; }
void SkeletonViz::SetUsePhysicalCam(bool use) { mUsePhysicalCam = use; }
void SkeletonViz::SetPhysicalCamRotation(float rotation) {
    mPhysicalCamRotation = rotation;
    mDesiredCamRotation = rotation;
}
void SkeletonViz::Rotate(float amt) { mDesiredCamRotation += amt; }
void SkeletonViz::SetAxesCoordSys(SkeletonCoordSys cs) { mAxesCoordSys = cs; }

void SkeletonViz::Init() {
    if (!mResource) {
        for (int i = 0; i < kNumBones; i++) {
            mLines[i] = Hmx::Object::New<RndLine>();
        }
        LoadResource(false);
        UpdateResource();
    }
}

void SkeletonViz::LoadResource(bool postload) {
    static Symbol objects("objects");
    mResource.LoadFile(
        FilePath(FileSystemRoot(), "ham/skeleton.milo"), postload, true, kLoadFront, false
    );
    if (!postload) {
        mResource.PostLoad(nullptr);
    }
}

void SkeletonViz::UpdateResource() {
    Transform xfm;
    xfm.Reset();
    MILO_ASSERT(mResource.IsLoaded(), 0x1E8);
    mEnviron = mResource->Find<RndEnviron>("skeleton.env", true);
    mCamMesh = mResource->Find<RndMesh>("camera.mesh", true);
    mCamMesh->SetTransParent(this, false);
    mCamMesh->SetLocalPos(xfm.v);
    mPhysicalCam = mResource->Find<RndCam>("physical.cam", true);
    mPhysicalCam->SetTransParent(this, false);
    mPhysicalCam->SetLocalXfm(xfm);
    mJointMesh = mResource->Find<RndMesh>("joint.mesh", true);
    mJointMesh->SetTransParent(this, false);
    mJointMesh->SetLocalPos(xfm.v);
    mJointMat = mResource->Find<RndMat>("joint.mat", true);
    mUtlLine = mResource->Find<RndLine>("utl.line", true);
    mUtlLine->SetTransParent(this, false);
    mUtlLine->SetLocalXfm(xfm);
    mUtlSphere = mResource->Find<RndMesh>("sphere.mesh", true);
    RndLine *boneLine = mResource->Find<RndLine>("bone.line", true);
    for (int i = 0; i < kNumBones; i++) {
        if (!mLines[i])
            mLines[i] = Hmx::Object::New<RndLine>();
        mLines[i]->Copy(boneLine, kCopyShallow);
        mLines[i]->SetTransParent(this, false);
        mLines[i]->SetLocalXfm(xfm);
    }
}

void SkeletonViz::SetPhysicalCamScreenRect(const Hmx::Rect &r) {
    MILO_ASSERT(r.x >= 0 && r.y >= 0 && r.w > 0 && r.h > 0, 0x64);
    MILO_ASSERT(mPhysicalCam, 0x65);
    mPhysicalCam->SetScreenRect(r);
}

void SkeletonViz::DrawLine3D(
    const Vector3 &vec1,
    const Vector3 &vec2,
    float f,
    const Hmx::Color &color1,
    Hmx::Color *color2
) {
    Vector3 localVec1, localVec2;
    Multiply(vec1, mNatalToWorld, localVec1);
    Multiply(vec2, mNatalToWorld, localVec2);
    mUtlLine->SetPointPos(0, localVec1);
    mUtlLine->SetPointPos(1, localVec2);
    RndMat *mat = mUtlLine->Mat();
    MILO_ASSERT(mat, 0x178);

    if (!color2) {
        mat->SetColor(color1);
    } else {
        mUtlLine->SetMat(0);
        mUtlLine->SetPointColor(0, color1, true);
        mUtlLine->SetPointColor(1, *color2, true);
    }
    mUtlLine->SetWidth(mDrawScale * f);
    mUtlLine->DrawShowing();
    mUtlLine->SetMat(mat);
}

void SkeletonViz::Poll() {
    if (mPhysicalCamRotation < mDesiredCamRotation) {
        mPhysicalCamRotation += TheTaskMgr.DeltaUISeconds() * 120.0f;
        if (mPhysicalCamRotation <= mDesiredCamRotation) {
            return;
        }
    } else {
        if (mPhysicalCamRotation <= mDesiredCamRotation) {
            return;
        }
        mPhysicalCamRotation -= TheTaskMgr.DeltaUISeconds() * 120.0f;
        if (mPhysicalCamRotation >= mDesiredCamRotation) {
            return;
        }
    }
    mPhysicalCamRotation = mDesiredCamRotation;
}

void SkeletonViz::Visualize(
    const CameraInput &camInput,
    const BaseSkeleton &skeleton,
    std::vector<SkeletonCallback *> *callbacks,
    bool darken
) {
    if (!mResource) {
        MILO_ASSERT(TheLoadMgr.EditMode(), 0x72);
        Init();
    }

    MILO_ASSERT(mResource.IsLoaded(), 0x76);

    RndEnvironTracker tracker(mEnviron, 0);
    mUseParentXfm = camInput.NatalToWorld(mNatalToWorld) == 0;
    if (mUseParentXfm) {
        mNatalToWorld = mDefaultNatalToWorld;
    }
    mDrawScale = camInput.DrawScale();
    Transform trans = mUseParentXfm ? WorldXfm() : mNatalToWorld;

    const SkeletonFrame &cachedFrame = camInput.CachedFrame();
    RndCam *currCam = RndCam::Current();
    if (skeleton.IsTracked()) {
        Vector3 cam_joints[kNumJoints];
        Vector3 milo_joints[kNumJoints];
        for (int i = 0; i < kNumJoints; i++) {
            skeleton.JointPos(kCoordCamera, (SkeletonJoint)i, cam_joints[i]);
            Multiply(cam_joints[i], mDefaultNatalToWorld, milo_joints[i]);
        }
        SetCamera(cachedFrame, trans, milo_joints[0].y);
        DrawJoints(skeleton, cam_joints, milo_joints, darken);

        if (callbacks) {
            FOREACH_PTR (it, callbacks) {
                SkeletonCallback *callback = *it;
                callback->Draw(skeleton, *this);
            }
        }
    } else {
        SetCamera(cachedFrame, trans, 0);
    }
    if (currCam) {
        currCam->Select();
    }
}

void SkeletonViz::DrawPoint3D(
    const Vector3 &p, float radius, const Hmx::Color &color, float alpha
) {
    Vector3 tmp;
    Multiply(p, mNatalToWorld, tmp);
    if (mUseParentXfm) {
        Multiply(tmp, WorldXfm(), tmp);
    }
    float scale = mDrawScale * radius;
    mUtlSphere->Mat()->SetColor(color);
    mUtlSphere->Mat()->SetAlpha(alpha);
    mUtlSphere->Mat()->SetCull(BaseMaterial::kCullNone);

    Transform xfm;
    xfm.Reset();
    xfm.v = tmp;
    Scale(Vector3(scale, scale, scale), xfm.m, xfm.m);

    mUtlSphere->SetLocalXfm(xfm);
    mUtlSphere->SetSphere(Sphere(Vector3(0, 0, 0), scale));
    mUtlSphere->DrawShowing();
}

void SkeletonViz::SetCamera(
    const SkeletonFrame &frame, const Transform &cam_origin, float hip_to_cam
) {
    if (mUsePhysicalCam) {
        if (mUseParentXfm) {
            Vector3 v(0, -hip_to_cam, 0);
            RotateAboutZ(v, mPhysicalCamRotation * DEG2RAD, v);
            v.y += hip_to_cam;
            mPhysicalCam->SetLocalPos(v);
            float angle = frame.TiltAngle();
            mPhysicalCam->SetLocalRot(Vector3(angle * RAD2DEG, 0, mPhysicalCamRotation));
            mPhysicalCam->SetFrustum(0.01f, 10, 0.7955211f, 1);
        } else {
            Transform xfm;
            xfm.Reset();
            xfm.v = mNatalToWorld.v;
            mPhysicalCam->SetFrustum(0.5f, 1000, mPhysicalCam->YFov(), 1);
            mPhysicalCam->SetLocalXfm(xfm);
        }
        mPhysicalCam->Select();
    } else {
        if (mAxesCoordSys == kCoordCamera || !mUseParentXfm) {
            UtilDrawAxes(cam_origin, 5 / mDrawScale, Hmx::Color(1, 1, 1));
        }
        if (mUseParentXfm) {
            mCamMesh->SetWorldPos(cam_origin.v);
            mCamMesh->DrawShowing();
            Vector3 end;
            Multiply(frame.mUpVector, mNatalToWorld.m, end);
            Add(end, cam_origin.v, end);
            TheRnd.DrawLine(cam_origin.v, end, Hmx::Color(1, 1, 0), false);
        }
    }
    if (mUseParentXfm) {
        Plane p = frame.mFloorPlane;
        Transform world = mNatalToWorld;
        world.v = cam_origin.v;
        Multiply(p, world, p);
        Vector3 v3 = cam_origin.v;
        v3.y += hip_to_cam;
        UtilDrawPlane(p, v3, Hmx::Color(1, 1, 0), 5, 0.5f, false);
    }
}

void SkeletonViz::DrawJoints(
    const BaseSkeleton &skeleton, Vector3 *cam_joints, Vector3 *milo_joints, bool darken
) {
    float f19 = darken ? 0.5f : 1.0f;
    Hmx::Color brightness_color(f19, f19, f19);
    float boneLens = skeleton.BoneLength(kBoneArmLowerRight, kCoordCamera)
        + skeleton.BoneLength(kBoneHandRight, kCoordCamera)
        + skeleton.BoneLength(kBoneArmUpperRight, kCoordCamera);
    float f16 = kHugeFloat;
    for (int i = 0; i < kNumBones; i++) {
        MinEq(f16, cam_joints[i].x);
    }
    float f20 = f16 + boneLens;
    float diff = f16 - f20;

    for (int i = 0; i < DIM(BaseSkeleton::sBones); i++) {
        Hmx::Color bone_color;
        bone_color.alpha *= brightness_color.alpha;
        float f15 = (cam_joints[BaseSkeleton::sBones[i].mStartJoint].z - f20) / diff;
        ClampEq(f15, 0.0f, 1.0f);
        float scale = f15 * 0.8f + 0.2f;
        // bone_color.r/g/b = brightness_color.r/g/b * scale;
        mLines[i]->SetPointColor(0, brightness_color, true);
    }

    int flags = skeleton.QualityFlags();
    Hmx::Color qual_color(1, 0, 0);
    Vector2 place(0.1f, 0.1f);
    if (mUsePhysicalCam) {
        place.x = mPhysicalCam->ScreenRect().x;
        place.y = mPhysicalCam->ScreenRect().y;
    }
    if (flags & 1) {
        place.y = TheRnd.DrawStringScreen("clipped right", place, qual_color, true).y;
    }
    if (flags & 2) {
        place.y = TheRnd.DrawStringScreen("clipped left", place, qual_color, true).y;
    }
    if (flags & 4) {
        place.y = TheRnd.DrawStringScreen("clipped top", place, qual_color, true).y;
    }
    if (flags & 8) {
        place.y = TheRnd.DrawStringScreen("clipped bottom", place, qual_color, true).y;
    }
    if (mAxesCoordSys != kCoordCamera && mUseParentXfm) {
        Transform cam_to_player;
        skeleton.CameraToPlayerXfm(mAxesCoordSys, cam_to_player);
        Transform axes;
        Multiply(cam_to_player, mNatalToWorld, axes);
        Multiply(axes, WorldXfm(), axes);
        std::swap(axes.m.y, axes.m.z);
        UtilDrawAxes(axes, mDrawScale / 4, Hmx::Color(1, 1, 1));
    }
}
