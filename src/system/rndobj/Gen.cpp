#include "rndobj/Gen.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"

RndGenerator::RndGenerator()
    : mPath(this), mPathStartFrame(0), mPathEndFrame(0), mMesh(this), mMultiMesh(this),
      mParticleSys(this), mNextFrameGen(-9999999), mRateGenLow(100), mRateGenHigh(100),
      mScaleGenLow(1), mScaleGenHigh(1), mPathVarMaxX(0), mPathVarMaxY(0),
      mPathVarMaxZ(0) {}

RndGenerator::~RndGenerator() { ResetInstances(); }

BEGIN_HANDLERS(RndGenerator)
    HANDLE(set_path, OnSetPath)
    HANDLE(set_ratevar, OnSetRateVar)
    HANDLE(set_scalevar, OnSetScaleVar)
    HANDLE(set_pathvar, OnSetPathVar)
    HANDLE(generate, OnGenerate)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndGenerator)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndGenerator)
    SAVE_REVS(0xB, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mMesh << mPath;
    bs << mRateGenLow << mRateGenHigh << mScaleGenLow << mScaleGenHigh;
    bs << mPathVarMaxX << mPathVarMaxY << mPathVarMaxZ;
    bs << mPathEndFrame << mPathStartFrame;
    bs << mMultiMesh << mParticleSys;
END_SAVES

BEGIN_COPYS(RndGenerator)
    CREATE_COPY_AS(RndGenerator, d)
    MILO_ASSERT(d, 48);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndAnimatable)
    ResetInstances();
    if (ty == kCopyFromMax)
        return;
    COPY_MEMBER_FROM(d, mMesh)
    COPY_MEMBER_FROM(d, mPath)
    COPY_MEMBER_FROM(d, mRateGenLow)
    COPY_MEMBER_FROM(d, mRateGenHigh)
    COPY_MEMBER_FROM(d, mScaleGenLow)
    COPY_MEMBER_FROM(d, mScaleGenHigh)
    COPY_MEMBER_FROM(d, mPathVarMaxX)
    COPY_MEMBER_FROM(d, mPathVarMaxY)
    COPY_MEMBER_FROM(d, mPathVarMaxZ)
    COPY_MEMBER_FROM(d, mPathEndFrame)
    COPY_MEMBER_FROM(d, mPathStartFrame)
    COPY_MEMBER_FROM(d, mMultiMesh)
    COPY_MEMBER_FROM(d, mParticleSys)
END_COPYS

BEGIN_LOADS(RndGenerator)
    LOAD_REVS(bs)
    ASSERT_REVS(0xB, 0)
    if (d.rev > 9) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    if (d.rev > 1) {
        LOAD_SUPERCLASS(RndTransformable)
        LOAD_SUPERCLASS(RndDrawable)
        LOAD_SUPERCLASS(RndAnimatable)
    }
    ResetInstances();
    d.stream >> mMesh >> mPath;
    if (d.rev < 7) {
        bool bd0;
        d >> bd0;
        if (!bd0) {
            MILO_NOTIFY("%s no longer supports childOfGen", Name());
        }
    }
    if (d.rev < 1) {
        d.stream >> mRateGenHigh;
        d.stream >> mScaleGenHigh;
    }
    if (d.rev < 8) {
        ObjPtr<RndCam> cam(this);
        bool bd0;
        int ic0;
        d >> bd0 >> ic0 >> cam;
    }

    if (d.rev > 0) {
        d.stream >> mRateGenLow;
        d.stream >> mRateGenHigh;
        d.stream >> mScaleGenLow;
        d.stream >> mScaleGenHigh;
        d.stream >> mPathVarMaxX;
        d.stream >> mPathVarMaxY;
        d.stream >> mPathVarMaxZ;
        if (d.rev < 9) {
            mPathVarMaxX *= DEG2RAD;
            mPathVarMaxY *= DEG2RAD;
            mPathVarMaxZ *= DEG2RAD;
        }
    } else {
        mRateGenLow = mRateGenHigh;
        mScaleGenLow = mScaleGenHigh;
        mPathVarMaxX = mPathVarMaxY = mPathVarMaxZ = 0;
    }
    if (d.rev == 3) {
        int x;
        ObjPtr<Hmx::Object> obj(this);
        d.stream >> obj >> x;
    }
    if (d.rev > 3 && d.rev < 0xB) {
        ObjPtr<Hmx::Object> obj(this);
        d.stream >> obj;
    }
    if (d.rev > 4 && d.rev < 0xB) {
        bool bd0;
        d >> bd0;
    }
    if (d.rev > 5) {
        d.stream >> mPathEndFrame;
        d.stream >> mPathStartFrame;
    } else {
        if (mPath)
            mPathEndFrame = mPath->EndFrame();
        mPathStartFrame = 0;
    }
    if (d.rev > 6) {
        d.stream >> mMultiMesh >> mParticleSys;
    }
END_LOADS

void RndGenerator::Print() {
    TheDebug << "   path: " << mPath << "\n";
    TheDebug << "   mesh: " << mMesh << "\n";
    TheDebug << "   rateGenLow: " << mRateGenLow << "\n";
    TheDebug << "   rateGenHigh: " << mRateGenHigh << "\n";
    TheDebug << "   scaleGenLow: " << mScaleGenLow << "\n";
    TheDebug << "   scaleGenHigh:" << mScaleGenHigh << "\n";
    TheDebug << "   pathVarMax: (" << mPathVarMaxX << ", " << mPathVarMaxY << ", "
             << mPathVarMaxZ << ")\n";
    TheDebug << "   multiMesh: " << mMultiMesh << "\n";
    TheDebug << "   particleSys: " << mParticleSys << "\n";
}

float RndGenerator::StartFrame() {
    if (mPath)
        return mPath->StartFrame();
    return 0;
}

float RndGenerator::EndFrame() {
    if (mPath)
        return mPath->EndFrame();
    return 0;
}

void RndGenerator::ListAnimChildren(std::list<RndAnimatable *> &list) const {
    if (mPath)
        list.push_back(mPath);
}

void RndGenerator::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    Transform xfm;
    FastInvert(WorldXfm(), xfm);
    Multiply(s, xfm, s);
    SetSphere(s);
}

void RndGenerator::ListDrawChildren(std::list<RndDrawable *> &list) {
    if (mMesh)
        list.push_back(mMesh);
    if (mMultiMesh)
        list.push_back(mMultiMesh);
    if (mParticleSys)
        list.push_back(mParticleSys);
}

void RndGenerator::ResetInstances() {
    mInstances.clear();
    if (mParticleSys)
        mParticleSys->Exit();
    if (mMultiMesh)
        mMultiMesh->Instances().clear();
}

void RndGenerator::DrawMesh(Transform &t, float) {
    mMesh->SetWorldXfm(t);
    mMesh->Draw();
}

void RndGenerator::DrawMultiMesh(Transform &t, float f) {
    *mCurMultiMesh++ = RndMultiMesh::Instance(t);
}

void RndGenerator::SetPath(RndTransAnim *path, float start, float end) {
    mPath = path;
    if (mPath && start == -1) {
        mPathStartFrame = mPath->StartFrame();
    } else
        mPathStartFrame = start;
    if (mPath && end == -1) {
        mPathEndFrame = mPath->EndFrame();
    } else
        mPathEndFrame = end;
}

DataNode RndGenerator::OnSetPath(const DataArray *da) {
    SetPath(da->Obj<RndTransAnim>(2), -1, -1);
    return 0;
}

DataNode RndGenerator::OnSetRateVar(const DataArray *da) {
    SetRateVar(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndGenerator::OnSetScaleVar(const DataArray *da) {
    SetScaleVar(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndGenerator::OnSetPathVar(const DataArray *da) {
    SetPathVar(da->Float(2), da->Float(3), da->Float(4));
    return 0;
}

DataNode RndGenerator::OnGenerate(const DataArray *da) {
    Generate(GetFrame());
    return 0;
}
