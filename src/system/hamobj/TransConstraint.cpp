#include "hamobj/TransConstraint.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Highlight.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"

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

BEGIN_LOADS(TransConstraint)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndPollable)
    LOAD_SUPERCLASS(RndHighlightable)
    bs >> mParent;
    bs >> mChild;
    bs >> mStaticCube;
    for (int i = 0; i < 3; i++) {
        bsrev >> mTracks[i];
    }
    if (gRev > 0) {
        bs >> mSpeed;
    }
    if (gRev > 1) {
        if (gRev <= 3) {
            bool b;
            bsrev >> b;
        }
        bsrev >> mAffectScale;
    }
    if (gRev > 2) {
        bsrev >> mUseUITime;
    }
END_LOADS

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
