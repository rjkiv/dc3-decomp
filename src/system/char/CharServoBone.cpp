#include "char/CharServoBone.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "char/CharPollable.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

CharServoBone::CharServoBone()
    : unk84(0), mFacingRotDelta(0), mFacingPosDelta(0), mFacingRot(0), mFacingPos(0),
      mMoveSelf(false), mDeltaChanged(false), mRegulate(this) {}

CharServoBone::~CharServoBone() {}

BEGIN_PROPSYNCS(CharServoBone)
    SYNC_PROP_SET(clip_type, mClipType, SetClipType(_val.Sym()))
    SYNC_PROP_SET(move_self, mMoveSelf, SetMoveSelf(_val.Int()))
    SYNC_PROP(delta_changed, mDeltaChanged)
    SYNC_PROP(regulate, mRegulate)
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

BEGIN_LOADS(CharServoBone)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    Symbol s;
    if (d.rev > 1)
        bs >> s;
    SetClipType(s);
END_LOADS

void CharServoBone::Poll() {}

void CharServoBone::ReallocateInternal() {}

void CharServoBone::Enter() {
    ZeroDeltas();
    SetRegulateWaypoint(nullptr);
    mDeltaChanged = false;
    mMoveSelf = mFacingPosDelta;
}

void CharServoBone::ZeroDeltas() {
    if (mFacingPosDelta)
        mFacingPosDelta->Zero();
    if (!mFacingRotDelta)
        return;
    *mFacingRotDelta = 0.0f;
}

void CharServoBone::SetClipType(Symbol sym) {
    if (sym != mClipType) {
        mClipType = sym;
        ClearBones();
        CharBoneDir::StuffBones(*this, mClipType);
    }
}

void CharServoBone::SetMoveSelf(bool b) {
    if (mMoveSelf == b)
        return;

    mMoveSelf = b;
    mDeltaChanged = true;
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

BEGIN_HANDLERS(CharServoBone)
    HANDLE_SUPERCLASS(CharPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
