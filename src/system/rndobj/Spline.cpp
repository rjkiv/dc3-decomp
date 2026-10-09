#include "rndobj/Spline.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "math/Rot.h"
#include "os/Debug.h"
#include "rndobj/Poll.h"
#include "rndobj/ShaderMgr.h"
#include "utl/BinStream.h"

#pragma region CtrlPoint

RndSpline::CtrlPoint::CtrlPoint()
    : mPos(Vector3::GetZero()), mRoll(0), unk14(1), mDirtyConstants(1),
      unk18(Vector4::GetZero()), unk28(Vector4::GetZero()), unk38(Vector4::GetZero()),
      unk48(Vector4::GetZero()) {}

void RndSpline::CtrlPoint::Save(BinStream &bs) const {
    bs << mPos;
    bs << mRoll;
}

void RndSpline::CtrlPoint::Load(BinStreamRev &d) {
    d >> mPos;
    d >> mRoll;
    unk14 = false;
}

void RndSpline::CtrlPoint::Interp(const CtrlPoint &p1, const CtrlPoint &p2, float r) {
    ::Interp(p1.mPos, p2.mPos, r, mPos);
    ::Interp(p1.mRoll, p2.mRoll, r, mRoll);
}

#pragma endregion
#pragma region RndSpline

static const float sSplineFloats[3] = { 10, 10, 4 };

RndSpline::RndSpline()
    : mManual(false), mPulseLength(sSplineFloats[0]), mPulseAmplitude(sSplineFloats[0]),
      mStartCtrlPoint(-1), mEndCtrlPoint(-1), mYOffset(0),
      mYPerCtrlPoint(sSplineFloats[0]), unk144(0), unk145(0), mPulse(0), unk148(-1000),
      unk14c(0) {}

BEGIN_HANDLERS(RndSpline)
    HANDLE(test_pulse, OnTestPulse)
    HANDLE(set_global_default, OnSetGlobalDefaultSpline)
    HANDLE(clear_global_default, OnClearGlobalDefaultSpline)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(RndSpline::CtrlPoint)
    SYNC_PROP(pos, o.mPos)
    SYNC_PROP_SET(roll, o.mRoll * RAD2DEG, o.mRoll = _val.Float() * DEG2RAD)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(RndSpline)
    SYNC_PROP_MODIFY(ctrl_points, mCtrlPoints, SyncPristineCtrlPoints())
    SYNC_PROP(manual, mManual)
    SYNC_PROP(pulse_length, mPulseLength)
    SYNC_PROP(pulse_amplitude, mPulseAmplitude)
    SYNC_PROP_SET(start_ctrl_point, mStartCtrlPoint, SetStartCtrlPoint(_val.Int()))
    SYNC_PROP_SET(end_ctrl_point, mEndCtrlPoint, SetEndCtrlPoint(_val.Int()))
    SYNC_PROP_SET(y_offset, mYOffset, mYOffset = _val.Float())
    SYNC_PROP_SET(
        y_per_ctrl_point, mYPerCtrlPoint, mYPerCtrlPoint = Min(0.1f, _val.Float())
    )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const RndSpline::CtrlPoint &pt) {
    pt.Save(bs);
    return bs;
}

BEGIN_SAVES(RndSpline)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(RndPollable)
    bs << mCtrlPoints;
    bs << mManual;
    bs << mPulseLength;
    bs << mPulseAmplitude;
    bs << mStartCtrlPoint;
    bs << mEndCtrlPoint;
    bs << mYOffset;
    bs << mYPerCtrlPoint;
    bs << (this == sGlobalDefaultSpline);
END_SAVES

BEGIN_COPYS(RndSpline)
    if (this != o) {
        COPY_SUPERCLASS(RndPollable)
        CREATE_COPY(RndSpline)
        BEGIN_COPYING_MEMBERS
            COPY_MEMBER(mCtrlPoints)
            COPY_MEMBER(mManual)
            COPY_MEMBER(mPulseLength)
            COPY_MEMBER(mPulseAmplitude)
            COPY_MEMBER(mStartCtrlPoint)
            COPY_MEMBER(mEndCtrlPoint)
            COPY_MEMBER(mYOffset)
            COPY_MEMBER(mYPerCtrlPoint)
            SyncPristineCtrlPoints();
        END_COPYING_MEMBERS
    }
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, RndSpline::CtrlPoint &pt) {
    pt.Load(d);
    return d;
}

INIT_REVS(1, 0)

BEGIN_LOADS(RndSpline)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(RndPollable)
    d >> mCtrlPoints;
    d >> mManual;
    if (d.rev >= 1) {
        d >> mPulseLength;
        d >> mPulseAmplitude;
    }
    d >> mStartCtrlPoint;
    d >> mEndCtrlPoint;
    d >> mYOffset;
    d >> mYPerCtrlPoint;
    bool sync;
    d >> sync;
    if (sync) {
        sGlobalDefaultSpline = this;
    }
    SyncPristineCtrlPoints();
END_LOADS

void RndSpline::Poll() {
    if (unk14c && mPulse) {
        unk148 += 0.033333335f;
        if (unk148 > (float)mCtrlPoints.size()) {
            unk14c = false;
            mPulse = false;
            unk148 = -1000;
        }
    }
}

void RndSpline::SetStartCtrlPoint(int idx) {
    if (idx != -1) {
        idx = Clamp(0, (int)mCtrlPoints.size() - 2, idx);
    }
    if (idx != mStartCtrlPoint) {
        mStartCtrlPoint = idx;
        if (mStartCtrlPoint != -1) {
            if (mEndCtrlPoint != -1) {
                if (mEndCtrlPoint <= mStartCtrlPoint) {
                    mEndCtrlPoint = mStartCtrlPoint + 1;
                }
            }
        }
    }
}

void RndSpline::SetEndCtrlPoint(int idx) {
    if (idx != -1) {
        idx = Clamp(1, (int)mCtrlPoints.size() - 1, idx);
    }
    if (idx != mEndCtrlPoint) {
        mEndCtrlPoint = idx;
        if (mEndCtrlPoint != -1) {
            if (mEndCtrlPoint <= mStartCtrlPoint) {
                mStartCtrlPoint = mEndCtrlPoint - 1;
            }
        }
    }
}

const RndSpline::CtrlPoint &RndSpline::GetDeformedCtrlPoint(int iIndex) const {
    MILO_ASSERT_RANGE(iIndex, 0, (int)mDeformedCtrlPoints.size(), 0x56);
    return mDeformedCtrlPoints[iIndex];
}

const RndSpline::CtrlPoint &RndSpline::GetDeformedCtrlPointOrDummy(int iIndex) const {
    MILO_ASSERT_RANGE_EQ(iIndex, -1, (int)(mDeformedCtrlPoints.size()) + 1, 0x2F7);
    if (iIndex == -1) {
        return unk3c;
    } else if (iIndex == (int)mDeformedCtrlPoints.size()) {
        return unk94;
    } else if (iIndex == (int)mDeformedCtrlPoints.size() + 1) {
        return unkec;
    } else {
        return mDeformedCtrlPoints[iIndex];
    }
}

void RndSpline::SyncDeformedDummyCtrlPoints(int iStartIndex, int iEndIndex) const {
    MILO_ASSERT_RANGE(iStartIndex, 0, (int)mDeformedCtrlPoints.size(), 0x2C5);
    MILO_ASSERT_RANGE(iEndIndex, 0, (int)mDeformedCtrlPoints.size(), 0x2C6);
    MILO_ASSERT(iStartIndex <= iEndIndex, 0x2C7);
    if (mDeformedCtrlPoints.size() >= 2) {
        if (unk144 && iStartIndex == 0) {
            RndSpline *me = const_cast<RndSpline *>(this);
            CtrlPoint &cur = me->mDeformedCtrlPoints[0];
            CtrlPoint &next = me->mDeformedCtrlPoints[1];
            me->unk144 = false;
            Vector3 posDiff;
            Subtract(cur.mPos, next.mPos, posDiff);
            Add(cur.mPos, posDiff, cur.mPos);
            cur.mRoll = next.mRoll;
            cur.mDirtyConstants = true;
        }
        if (unk145 && mDeformedCtrlPoints.size() - 2 <= iEndIndex) {
            RndSpline *me = const_cast<RndSpline *>(this);
            CtrlPoint &last = me->mDeformedCtrlPoints[mDeformedCtrlPoints.size() - 1];
            CtrlPoint &prev = me->mDeformedCtrlPoints[mDeformedCtrlPoints.size() - 2];
            me->unk145 = false;
            Vector3 posDiff;
            Subtract(last.mPos, prev.mPos, posDiff);
            Add(last.mPos, posDiff, last.mPos);
            Subtract(unk94.mPos, last.mPos, posDiff);
            Add(unk94.mPos, posDiff, (Vector3 &)unkec.mPos);
            prev.mDirtyConstants = true;
            last.mDirtyConstants = true;
        }
    }
}

void RndSpline::PrepareShader(float f1, float f2) const {
    if (mDeformedCtrlPoints.size() >= 2) {
        int endCtrlPt = mEndCtrlPoint;
        int startCtrlPt = Max(mStartCtrlPoint, 0);
        if (endCtrlPt == -1) {
            endCtrlPt = mCtrlPoints.size() - 1;
        }
        SyncDeformedCtrlPoints(startCtrlPt, endCtrlPt);
        MILO_ASSERT(((endCtrlPt - startCtrlPt) + 1) < kVShader_SplineMaxCtrlPoints, 0x1C1);
        int shaderConstant = 0xAE;
        for (int i = startCtrlPt; i <= endCtrlPt; i++, shaderConstant += 4) {
            const CtrlPoint &pt = GetDeformedCtrlPoint(i);
            MILO_ASSERT(!pt.mDirtyConstants, 0x1CF);
            TheShaderMgr.SetVConstant((VShaderConstant)(shaderConstant - 1), pt.unk18);
            TheShaderMgr.SetVConstant((VShaderConstant)(shaderConstant), pt.unk28);
            TheShaderMgr.SetVConstant((VShaderConstant)(shaderConstant + 1), pt.unk38);
            TheShaderMgr.SetVConstant((VShaderConstant)(shaderConstant + 2), pt.unk48);
        }
        TheShaderMgr.SetVConstant(
            (VShaderConstant)0x19, Vector4(endCtrlPt - startCtrlPt, f1, 1.0f / f2, 0)
        );
        if (mPulse) {
            TheShaderMgr.SetVConstant(
                (VShaderConstant)0x1A,
                Vector4(
                    unk148 - startCtrlPt,
                    (mYPerCtrlPoint / mPulseLength) * 2.0f,
                    mPulseAmplitude,
                    0
                )
            );
        }
    }
}

DataNode RndSpline::OnTestPulse(DataArray *) {
    if (!unk14c) {
        unk14c = true;
        mPulse = true;
        unk148 = -1;
    }
    return 0;
}

DataNode RndSpline::OnSetGlobalDefaultSpline(DataArray *) {
    sGlobalDefaultSpline = this;
    return 0;
}

DataNode RndSpline::OnClearGlobalDefaultSpline(DataArray *) {
    sGlobalDefaultSpline = nullptr;
    return 0;
}

#pragma endregion
