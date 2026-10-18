#include "char/CharServoBone.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "char/CharClipDriver.h"
#include "char/CharDriver.h"
#include "char/CharPollable.h"
#include "char/Character.h"
#include "char/CharUtl.h"
#include "char/Waypoint.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

CharServoBone::CharServoBone()
    : mPelvis(0), mFacingRotDelta(0), mFacingPosDelta(0), mFacingRot(0), mFacingPos(0),
      mMoveSelf(false), mDeltaChanged(false), mWay(this) {}

CharServoBone::~CharServoBone() {}

BEGIN_HANDLERS(CharServoBone)
    HANDLE_SUPERCLASS(CharPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharServoBone)
    SYNC_PROP_SET(clip_type, mClipType, SetClipType(_val.Sym()))
    SYNC_PROP_SET(move_self, mMoveSelf, SetMoveSelf(_val.Int()))
    SYNC_PROP(delta_changed, mDeltaChanged)
    SYNC_PROP(regulate, mWay)
    SYNC_SUPERCLASS(CharBonesMeshes)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharServoBone)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mClipType;
END_SAVES

BEGIN_COPYS(CharServoBone)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharServoBone)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMoveSelf)
        SetClipType(c->mClipType);
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(CharServoBone)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    Symbol s;
    if (d.rev > 1)
        d >> s;
    SetClipType(s);
END_LOADS

void CharServoBone::Poll() {
    if (!mMeshes.empty()) {
        PoseMeshes();
        Character *cur = Character::Current();
        if (mFacingPosDelta) {
            if (!mMoveSelf) {
                if (mDeltaChanged) {
                    Transform tf48 = cur->LocalXfm();
                    MoveToDeltaFacing(tf48);
                    Transform tf78;
                    Multiply(mPelvis->LocalXfm(), tf48, tf78);
                    MoveToFacing(mPelvis->DirtyLocalXfm());
                    Transform tfa8;
                    FastInvert(mPelvis->DirtyLocalXfm(), tfa8);
                    Multiply(tfa8, tf78, cur->DirtyLocalXfm());
                } else {
                    MoveToFacing(mPelvis->DirtyLocalXfm());
                }
                CharBoneDir *dir = CharBoneDir::FindResourceFromClipType(mClipType);
                for (ObjDirItr<CharBone> it(dir, false); it != nullptr; ++it) {
                    if (it->BakeOutAsTopLevel()) {
                        String str(it->Name());
                        if (str.find(".cb") != String::npos) {
                            str = str.substr(0, str.length() - 3);
                        }
                        RndTransformable *boneTrans =
                            CharUtlFindBoneTrans(str.c_str(), Dir());
                        if (mDeltaChanged) {
                            MoveToDeltaFacing(boneTrans->DirtyLocalXfm());
                            MoveToFacing(boneTrans->DirtyLocalXfm());
                        } else {
                            MoveToFacing(boneTrans->DirtyLocalXfm());
                        }
                    }
                }
            } else {
                if (mDeltaChanged) {
                    Transform tfd8(mPelvis->LocalXfm());
                    MoveToFacing(tfd8);
                    Multiply(tfd8, cur->LocalXfm(), tfd8);
                    Transform tf108;
                    FastInvert(mPelvis->LocalXfm(), tf108);
                    Multiply(tf108, tfd8, cur->DirtyLocalXfm());
                } else {
                    MoveToDeltaFacing(cur->DirtyLocalXfm());
                }
                RegulateInternal(cur);
            }
            mDeltaChanged = false;
        }
        ZeroDeltas();
    }
}

void CharServoBone::ReallocateInternal() {
    CharBonesMeshes::ReallocateInternal();
    mFacingRotDelta = nullptr;
    mFacingPosDelta = (Vector3 *)FindPtr("bone_facing_delta.pos");
    if (mFacingPosDelta) {
        mFacingPos = (Vector3 *)FindPtr("bone_facing.pos");
        mPelvis = CharUtlFindBoneTrans("bone_pelvis", Dir());
        if (!mFacingPos) {
            MILO_NOTIFY("CharServoBone: no Facing Pos in ReallocateInternal()");
        }
        if (!mPelvis) {
            MILO_NOTIFY("CharServoBone: no pelvis bone in this dir.");
        }
        mFacingRot = (float *)FindPtr("bone_facing.rotz");
        mFacingRotDelta = (float *)FindPtr("bone_facing_delta.rotz");
    }
}

void CharServoBone::Enter() {
    ZeroDeltas();
    Regulate(nullptr);
    mDeltaChanged = false;
    mMoveSelf = mFacingPosDelta;
}

void CharServoBone::ZeroDeltas() {
    if (mFacingPosDelta) {
        mFacingPosDelta->Zero();
    }
    if (mFacingRotDelta) {
        *mFacingRotDelta = 0;
    }
}

void CharServoBone::SetClipType(Symbol sym) {
    if (sym != mClipType) {
        mClipType = sym;
        ClearBones();
        CharBoneDir::StuffBones(*this, mClipType);
    }
}

void CharServoBone::SetMoveSelf(bool b) {
    if (mMoveSelf != b) {
        mMoveSelf = b;
        mDeltaChanged = true;
    }
}

void CharServoBone::MoveToDeltaFacing(Transform &tf) {
    Vector3 v18;
    Multiply(*mFacingPosDelta, tf.m, v18);
    tf.v += v18;
    if (mFacingRotDelta) {
        RotateAboutZ(tf.m, *mFacingRotDelta, tf.m);
        Normalize(tf.m, tf.m);
    }
}

void CharServoBone::MoveToFacing(Transform &tf) {
    if (mFacingRot) {
        RotateAboutZ(tf.m, *mFacingRot, tf.m);
        RotateAboutZ(tf.v, *mFacingRot, tf.v);
        Normalize(tf.m, tf.m);
    }
    tf.v += *mFacingPos;
}

void CharServoBone::RegulateInternal(Character *c) {
    if (mWay) {
        CharDriver *charDriver = c->Driver();
        CharClipDriver *beforeLast = charDriver->Before(charDriver->Last());
        CharClipDriver *next =
            beforeLast && beforeLast->mRampIn > 0 ? beforeLast->Next() : nullptr;
        if (next) {
            DoRegulate(
                c, mWay, next, beforeLast->mRampIn, Max(2.0f, beforeLast->mRampIn / 1.5f)
            );
        }
        mWay->Constrain(c->DirtyLocalXfm());
    }
}

void CharServoBone::DoRegulate(
    Character *c, Waypoint *w, CharClipDriver *driver, float f3, float f4
) {
    Transform &charXfm = c->DirtyLocalXfm();
    ClipPredict pred(driver->Clip(), charXfm.v, GetZAngle(charXfm.m));
    pred.Predict(driver->mBeat, driver->mBeat + f3);
    float ang = pred.Angle();
    Vector3 v90 = pred.Pos();
    float delta = TheTaskMgr.DeltaBeat() / f4;
    Vector3 va0;
    w->ShapeDelta(v90, va0);
    ScaleAddEq(charXfm.v, va0, delta);
    float angle = w->ShapeDelta(ang) * delta;
    RotateAboutZ(charXfm.m, angle, charXfm.m);
}
