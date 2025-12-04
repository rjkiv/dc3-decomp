#include "gesture/SkeletonViz.h"
#include "SkeletonViz.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamCharacter.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Line.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

SkeletonViz::SkeletonViz()
    : mUsePhysicalCam(0), mPhysicalCamRotation(0), unk110(0), mAxesCoordSys(kCoordCamera),
      mUtlLine(0), mSkeletonEnv(0), mCamMesh(0), mJointMesh(0), mJointMat(0),
      mPhysicalCam(0), unk214(0), unk218(true) {
    unk194.Reset();
    Multiply(Hmx::Matrix3(1, 0, 0, 0, 0, 1, 0, 1, 0), unk194.m, unk194.m);
    unk1d4 = unk194;
    for (int i = 0; i < kNumBones; i++) {
        mBoneLines[i] = nullptr;
    }
}

SkeletonViz::~SkeletonViz() {
    for (int i = 0; i < kNumBones; i++) {
        delete mBoneLines[i];
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

void SkeletonViz::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(6, 1)
    if (d.rev > 5) {
        Hmx::Object::Load(bs);
    }
    RndDrawable::Load(bs);
    RndTransformable::Load(bs);
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
    unk110 = mPhysicalCamRotation;
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
    unk110 = rotation;
}
void SkeletonViz::Rotate(float amt) { unk110 += amt; }
void SkeletonViz::SetAxesCoordSys(SkeletonCoordSys cs) { mAxesCoordSys = cs; }

void SkeletonViz::Init() {
    if (!mResource) {
        for (int i = 0; i < kNumBones; i++) {
            mBoneLines[i] = Hmx::Object::New<RndLine>();
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
    mSkeletonEnv = mResource->Find<RndEnviron>("skeleton.env", true);
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
    mSphereMesh = mResource->Find<RndMesh>("sphere.mesh", true);
    RndLine *boneLine = mResource->Find<RndLine>("bone.line", true);
    for (int i = 0; i < kNumBones; i++) {
        if (!mBoneLines[i])
            mBoneLines[i] = Hmx::Object::New<RndLine>();
        mBoneLines[i]->Copy(boneLine, kCopyShallow);
        mBoneLines[i]->SetTransParent(this, false);
        mBoneLines[i]->SetLocalXfm(xfm);
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
    Multiply(vec1, unk1d4, localVec1);
    Multiply(vec2, unk1d4, localVec2);
    mUtlLine->SetPointPos(0, localVec1);
    mUtlLine->SetPointPos(1, localVec2);
    RndMat *mat = mUtlLine->Mat();
    MILO_ASSERT(mat, 0x178);

    if (!color2) {
        mat->SetColor(color2->red, color2->green, color2->blue);
    } else {
        mUtlLine->SetMat(0);
        mUtlLine->SetPointColor(0, *color2, true);
        mUtlLine->SetPointColor(1, color1, true);
    }
    mUtlLine->SetWidth(unk214 * f);
    mUtlLine->DrawShowing();
    mUtlLine->SetMat(mat);
}
