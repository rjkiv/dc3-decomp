#include "char/CharMirror.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

CharMirror::CharMirror() : mServo(this), mMirrorServo(this), mBones(), mOps() {}

BEGIN_PROPSYNCS(CharMirror)
    SYNC_PROP_SET(servo, (Hmx::Object *)mServo, SetServo(_val.Obj<CharServoBone>()));
    SYNC_PROP_SET(
        mirror_servo,
        (Hmx::Object *)mMirrorServo,
        SetMirrorServo(_val.Obj<CharServoBone>())
    );
    SYNC_SUPERCLASS(CharWeightable);
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharMirror)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mMirrorServo;
    bs << mServo;
END_SAVES

BEGIN_COPYS(CharMirror)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharMirror)
    BEGIN_COPYING_MEMBERS
        SetMirrorServo(c->mMirrorServo);
        SetServo(c->mServo);
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharMirror)
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> mMirrorServo;
    bs >> mServo;
    SyncBones();
END_LOADS

void CharMirror::Poll() {
    static Symbol x("x");
    static Symbol xy("xy");
    static Symbol zw("zw");
    static Symbol mirror_x("mirror_x");

    mBones.ScaleDown(*mServo, 1.0f - Weight());
    MirrorOp *curMirrorOp = &mOps[0];
    for (Vector3 *it = (Vector3 *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_POS]);
         it < (Vector3 *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_SCALE]);
         curMirrorOp++, it++) {
        *it = *(Vector3 *)curMirrorOp->ptr;
        if (!curMirrorOp->op.Null() && curMirrorOp->op == x) {
            it->x = -it->x;
        }
    }
    for (Hmx::Quat *it = (Hmx::Quat *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_QUAT]);
         it < (Hmx::Quat *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_ROTX]);
         curMirrorOp++, it++) {
        *it = *(Hmx::Quat *)curMirrorOp->ptr;
        if (!curMirrorOp->op.Null()) {
            if (curMirrorOp->op == zw) {
                it->w = -it->w;
                it->z = -it->z;
            } else if (curMirrorOp->op == xy) {
                it->x = -it->x;
                it->y = -it->y;
            } else if (curMirrorOp->op == mirror_x) {
                it->Set(it->z, it->w, it->x, it->y);
            } else
                MILO_WARN("Unknown operation %s", curMirrorOp->op);
        }
    }
    for (float *it = (float *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_ROTX]);
         it < (float *)(mBones.mStart + mBones.mOffsets[CharBones::TYPE_END]);
         curMirrorOp++, it++) {
        *it = *(float *)curMirrorOp->ptr;
    }
    mBones.ScaleAdd(*mServo, Weight());
}

void CharMirror::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mServo);
}

void CharMirror::SyncBones() {
    mBones.ClearBones();
    if (!mServo || !mMirrorServo || !TypeDef())
        return;
    else {
        std::list<CharBones::Bone> bones;
        DataArray *mapArr = TypeDef()->FindArray("mappings");
        for (int i = 1; i < mapArr->Size(); i++) {
            bones.push_back(CharBones::Bone(mapArr->Array(i)->Sym(0), 1));
        }
        mBones.AddBones(bones);
        int numBones = mBones.GetBones().size();
        mOps.resize(numBones);
        for (int i = 0; i < mOps.size(); i++) {
            Symbol boneName = mBones.GetBonesAt(i).name;
            DataArray *boneArr = mapArr->FindArray(boneName);
            mOps[i].ptr = mMirrorServo->FindPtr(boneArr->Sym(1));
            mOps[i].op = boneArr->Size() > 2 ? boneArr->Sym(2) : Symbol();
        }
    }
}

void CharMirror::SetServo(CharServoBone *bone) {
    if (bone != mServo) {
        mServo = bone;
        SyncBones();
    }
}

void CharMirror::SetMirrorServo(CharServoBone *bone) {
    if (bone != mMirrorServo) {
        mMirrorServo = bone;
        SyncBones();
    }
}

BEGIN_HANDLERS(CharMirror)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
