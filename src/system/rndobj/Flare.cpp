#include "rndobj/Flare.h"
#include "Rnd.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"

RndFlare::RndFlare()
    : mPointTest(true), mAreaTest(true), mVisible(false), mSizes(0.1, 0.1), mMat(this),
      mRange(0, 0), mOffset(0), mSteps(1), mStep(0), unk144(0), mTestDone(false),
      mLastDone(false), unk17c(1, 1) {
    mMatrix.Identity();
}

RndFlare::~RndFlare() { TheRnd.RemovePointTest(this); }

BEGIN_HANDLERS(RndFlare)
    HANDLE_ACTION(set_steps, SetSteps(_msg->Int(2)))
    HANDLE_ACTION(set_point_test, SetPointTest(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndFlare)
    SYNC_PROP(mat, mMat)
    SYNC_PROP(sizes, mSizes)
    SYNC_PROP(steps, mSteps)
    SYNC_PROP(range, mRange)
    SYNC_PROP(offset, mOffset)
    SYNC_PROP_MODIFY(point_test, mPointTest, TheRnd.RemovePointTest(this))
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndFlare)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mMat << mSizes << mRange << mSteps;
    bs << mPointTest;
    bs << mOffset;
END_SAVES

BEGIN_COPYS(RndFlare)
    CREATE_COPY_AS(RndFlare, f)
    MILO_ASSERT(f, 0x19);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_MEMBER_FROM(f, mSizes)
    COPY_MEMBER_FROM(f, mMat)
    COPY_MEMBER_FROM(f, mVisible)
    COPY_MEMBER_FROM(f, mRange)
    COPY_MEMBER_FROM(f, mOffset)
    COPY_MEMBER_FROM(f, mSteps)
    COPY_MEMBER_FROM(f, mPointTest)
    mLastDone = false;
    mTestDone = false;
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(RndFlare)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    if (d.rev > 3) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndTransformable)
    LOAD_SUPERCLASS(RndDrawable)
    if (d.rev > 0) {
        d >> mMat;
    }
    if (d.rev > 2) {
        d >> mSizes;
    } else {
        d >> mSizes.x;
        mSizes.y = mSizes.x;
    }
    if (d.rev > 1) {
        d.stream >> mRange >> mSteps;
    }
    if (d.rev > 4) {
        d >> mPointTest;
    }
    if (d.rev > 6) {
        d >> mOffset;
    }
    mLastDone = false;
    mTestDone = false;
    CalcScale();
END_LOADS

void RndFlare::Print() {
    TheDebug << "   mat: " << mMat << "\n";
    TheDebug << "   sizes: " << mSizes << "\n";
    TheDebug << "   range: " << mRange << "\n";
    TheDebug << "   offset:" << mOffset << "\n";
    TheDebug << "   steps: " << mSteps << "\n";
    TheDebug << "   point test: " << mPointTest << "\n";
}

void RndFlare::Mats(std::list<RndMat *> &mats, bool b2) {
    if (mMat) {
        MatShaderOptions opts = GetDefaultMatShaderOpts(this, mMat);
        mMat->SetShaderOpts(opts);
        mats.push_back(mMat);
    }
}

void RndFlare::SetMat(RndMat *mat) { mMat = mat; }

void RndFlare::SetPointTest(bool test) {
    if (!test && mPointTest) {
        TheRnd.RemovePointTest(this);
    }
    mPointTest = test;
}

bool RndFlare::RectOffscreen(const Hmx::Rect &r) const {
    if (r.x + r.w < 0)
        return true;
    else if (r.y + r.h < 0)
        return true;
    else if (r.x > TheRnd.Width())
        return true;
    else if (r.y > TheRnd.Height())
        return true;
    else
        return false;
}

void RndFlare::CalcScale() {
    if (mMatrix != WorldXfm().m) {
        mMatrix = WorldXfm().m;
        float len = Length(mMatrix.z);
        Vector3 vtmp;
        Cross(mMatrix.x, mMatrix.y, vtmp);
        unk17c.Set(Length(mMatrix.x), Dot(vtmp, mMatrix.z) > 0.0f ? len : -len);
    }
}

void RndFlare::SetSteps(int steps) {
    steps = Max(steps, 1);
    if (mStep == mSteps) {
        mStep = steps;
    } else {
        mStep *= ((float)steps / (float)mSteps);
    }
    mSteps = steps;
}
