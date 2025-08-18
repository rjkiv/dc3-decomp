#include "rndobj/AnimFilter.h"

#include "rndobj/Anim.h"
#include "utl/BinStream.h"

float RndAnimFilter::Scale() {
    float ret;
    if (mPeriod) {
        ret = (mEnd - mStart) / (mPeriod * FramesPerUnit());
    } else {
        if (mEnd >= mStart)
            ret = mScale;
        else
            ret = -mScale;
    }
    return ret;
}

float RndAnimFilter::StartFrame() {
    if (!mAnim)
        return 0.0f;
    else {
        float denom = Scale();
        if (denom == 0.0f)
            denom = 1.0f;

        return (mStart - FrameOffset()) / denom;
    }
}

float RndAnimFilter::EndFrame() {
    if (!mAnim)
        return 0.0f;
    else {
        float denom = Scale();
        if (denom == 0.0f)
            denom = 1.0f;

        float ret = (mEnd - FrameOffset()) / denom;
        if (mType == kShuttle) {
            ret *= 2.0f;
        }
        return ret;
    }
}

void RndAnimFilter::Save(BinStream &bs) {
    bs << 2;
    SAVE_SUPERCLASS(Hmx::Object);
    SAVE_SUPERCLASS(RndAnimatable);
    bs << mAnim << mScale << mOffset << mStart << mEnd << mType;
    bs << mPeriod << mSnap << mJitter;
}

void RndAnimFilter::SetAnim(RndAnimatable *anim) {
    mAnim = anim;
    if (mAnim) {
        SetRate(mAnim->GetRate());
        mStart = mAnim->StartFrame();
        mEnd = mAnim->EndFrame();
    }
}

BEGIN_COPYS(RndAnimFilter)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY(RndAnimFilter)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mScale)
            COPY_MEMBER(mOffset)
            COPY_MEMBER(mStart)
            COPY_MEMBER(mEnd)
            COPY_MEMBER(mType)
            COPY_MEMBER(mAnim)
            COPY_MEMBER(mPeriod)
            COPY_MEMBER(mSnap)
            COPY_MEMBER(mJitter)
        }
    END_COPYING_MEMBERS
END_COPYS

RndAnimFilter::RndAnimFilter()
    : mAnim(this), mPeriod(0.0f), mStart(0.0f), mEnd(0.0f), mScale(1.0f), mOffset(0.0f),
      mSnap(0.0f), mJitter(0.0f), mJitterFrame(0.0f), mType(kRange) {}

BEGIN_PROPSYNCS(RndAnimFilter)
    SYNC_PROP_SET(anim, mAnim.Ptr(), SetAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_SET(scale, mScale, mScale = std::fabs(_val.Float()))
    SYNC_PROP(offset, mOffset)
    SYNC_PROP(period, mPeriod)
    SYNC_PROP(start, mStart)
    SYNC_PROP(end, mEnd)
    SYNC_PROP(snap, mSnap)
    SYNC_PROP_MODIFY(jitter, mJitter, mJitterFrame = 0.0f)
    SYNC_PROP(type, (int &)mType)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void RndAnimFilter::ListAnimChildren(std::list<RndAnimatable *> &theList) const {
    if (mAnim)
        theList.push_back(mAnim);
}

void RndAnimFilter::Load(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    bs >> mAnim;
    bs >> mScale;
    bs >> mOffset;
    bs >> mStart;
    bs >> mEnd;
    if (gRev > 0) {
        bs >> (int &)mType;
        bs >> mPeriod;
    } else {
        bool b;
        bsrev >> b;
        mType = (RndAnimFilter::Type)(b);
    }
    if (gRev > 1) {
        bs >> mSnap >> mJitter;
    }
}

BEGIN_HANDLERS(RndAnimFilter)
    HANDLE(safe_anims, OnSafeAnims)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
