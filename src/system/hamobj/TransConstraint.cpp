#include "hamobj/TransConstraint.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Highlight.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"

TransConstraint::TransConstraint()
    : mParent(this), mChild(this), mSpeed(10), mAffectScale(0), mUseUITime(0), unk52(1) {
    mStaticCube.Zero();
    for (int i = 0; i < 3; i++) {
        mTracks[i] = false;
    }
}

BEGIN_HANDLERS(TransConstraint)
    HANDLE_ACTION(snap_to_parent, SnapToParent())
    HANDLE_SUPERCLASS(RndHighlightable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(TransConstraint)
    SYNC_PROP(parent, mParent)
    SYNC_PROP(child, mChild)
    SYNC_PROP(static_cube, mStaticCube)
    SYNC_PROP(speed, mSpeed)
    SYNC_PROP(affect_scale, mAffectScale)
    SYNC_PROP(use_ui_time, mUseUITime)
    SYNC_PROP(track_x, mTracks[0])
    SYNC_PROP(track_y, mTracks[1])
    SYNC_PROP(track_z, mTracks[2])
    SYNC_SUPERCLASS(RndHighlightable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(TransConstraint)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndPollable)
    SAVE_SUPERCLASS(RndHighlightable)
    bs << mParent;
    bs << mChild;
    bs << mStaticCube;
    for (int i = 0; i < 3; i++) {
        bs << mTracks[i];
    }
    bs << mSpeed;
    bs << mAffectScale;
    bs << mUseUITime;
END_SAVES

BEGIN_COPYS(TransConstraint)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndPollable)
    COPY_SUPERCLASS(RndHighlightable)
    CREATE_COPY(TransConstraint)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mParent)
        COPY_MEMBER(mChild)
        COPY_MEMBER(mStaticCube)
        for (int i = 0; i < 3; i++) {
            COPY_MEMBER(mTracks[i])
        }
        COPY_MEMBER(mSpeed)
        COPY_MEMBER(mAffectScale)
        COPY_MEMBER(mUseUITime)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BEGIN_LOADS(TransConstraint)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndPollable)
    LOAD_SUPERCLASS(RndHighlightable)
    d >> mParent;
    d >> mChild;
    d >> mStaticCube;
    for (int i = 0; i < 3; i++) {
        d >> mTracks[i];
    }
    if (d.rev > 0) {
        d >> mSpeed;
    }
    if (d.rev > 1) {
        if (d.rev <= 3) {
            bool b;
            d >> b;
        }
        d >> mAffectScale;
    }
    if (d.rev > 2) {
        d >> mUseUITime;
    }
END_LOADS

void TransConstraint::Highlight() {
    if (mParent && mChild) {
        Transform trans = mParent->WorldXfm();
        trans.m.Identity();

        Box b;
        for (int i = 0; i < 3; i++) {
            float f = mStaticCube[i] / 2.0f;
            b.mMin[i] = -f;
            b.mMax[i] = f;
        }

        UtilDrawAxes(mParent->WorldXfm(), 10.0f, Hmx::Color(1, 1, 1));
        UtilDrawAxes(mChild->WorldXfm(), 10.0f, Hmx::Color(1, 1, 1));
        UtilDrawBox(trans, b, Hmx::Color(1, 1, 0), true);
    }
}

void TransConstraint::Poll() {
    if (mParent && mChild && unk52) {
        float f12 = mUseUITime ? TheTaskMgr.DeltaUISeconds() : TheTaskMgr.DeltaSeconds();
        Vector3 va0 = mParent->WorldXfm().v;
        Vector3 ve0 = mChild->WorldXfm().v;
        Vector3 vd0;
        Subtract(va0, ve0, vd0);
        for (int i = 0; i < 3; i++) {
            if (!mTracks[i]) {
                vd0[i] = 0;
            }
        }
        if (Length(vd0) > mSpeed * 3) {
            SnapToParent();
        } else {
            Vector3 v90;
            Normalize(vd0, v90);
            for (int i = 0; i < 3; i++) {
                if (mTracks[i]) {
                    float f10 = mStaticCube[i] / 2;
                    float f11 = va0[i] - f10;
                    f10 += va0[i];
                    if (ve0[i] < f11 || ve0[i] > f10) {
                        float f9 = v90[i] * mSpeed;
                        if (ve0[i] < f11) {
                            ve0[i] += f9 * f12;
                            MinEq(ve0[i], f11);
                        } else if (ve0[i] > f10) {
                            ve0[i] += f9 * f12;
                            MaxEq(ve0[i], f10);
                        }
                    }
                }
            }
            mChild->SetWorldPos(ve0);
            if (mAffectScale) {
                Vector3 vb0;
                MakeScale(mParent->WorldXfm().m, vb0);
                Vector3 vf0;
                MakeScale(mChild->WorldXfm().m, vf0);
                Vector3 v70;
                MakeScale(mChild->LocalXfm().m, v70);
                Vector3 vc0;
                Subtract(vb0, vf0, vc0);
                for (int i = 0; i < 3; i++) {
                    if (!mTracks[i]) {
                        vc0[i] = 0;
                    }
                }
                float f7 = 1;
                float f8 = Length(vd0);
                if (f8 > 0) {
                    f7 = Length(vc0) / f8;
                }
                Vector3 v80;
                Normalize(vc0, v80);
                f7 *= mSpeed;
                for (int i = 0; i < 3; i++) {
                    if (mTracks[i]) {
                        float f8 = vb0[i];
                        float f11 = vb0[i];
                        if (vf0[i] < f8 || vf0[i] > f11) {
                            float f10 = v80[i] * f7;
                            if (vf0[i] < f8) {
                                vf0[i] += f10 * f12;
                                MinEq(vf0[i], f8);
                            } else if (vf0[i] > f11) {
                                vf0[i] += f10 * f12;
                                MaxEq(vf0[i], f11);
                            }
                        }
                    } else {
                        vf0[i] = v70[i] + vb0[i];
                    }
                }
                SetScaleVectorOnTransform(mChild, vf0);
            }
        }
    }
}

void TransConstraint::Enter() {
    RndPollable::Enter();
    SnapToParent();
}

void TransConstraint::SetScaleVectorOnTransform(RndTransformable *trans, Vector3 &v) {
    MILO_ASSERT(trans, 0x80);
    Vector3 va0;
    Hmx::Matrix3 m90;
    Transform tf60 = trans->WorldXfm();
    MakeEuler(tf60.m, va0);
    MakeRotMatrix(va0, m90, true);
    Scale(m90, v, m90);
    tf60.m = m90;
    trans->SetWorldXfm(tf60);
}

void TransConstraint::SnapToParent() {
    if (mParent && mChild) {
        Vector3 v50 = mParent->WorldXfm().v;
        Vector3 v70 = mChild->WorldXfm().v;
        for (int i = 0; i < 3; i++) {
            if (mTracks[i]) {
                v70[i] = v50[i];
            }
        }
        mChild->SetWorldPos(v70);
        if (mAffectScale) {
            Vector3 v40;
            MakeScale(mParent->WorldXfm().m, v40);
            Vector3 v60;
            MakeScale(mChild->WorldXfm().m, v60);
            for (int i = 0; i < 3; i++) {
                if (mTracks[i]) {
                    v60[i] = v40[i];
                }
            }
            SetScaleVectorOnTransform(mChild, v60);
        }
    }
}
