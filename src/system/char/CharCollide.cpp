#include "char/CharCollide.h"
#include "CharCollide.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

CharCollide::CharCollide()
    : mShape(kCollideSphere), mFlags(0), mMesh(this), mMeshYBias(false) {
    for (int i = 0; i < 2; i++) {
        mOrigRadius[i] = 0;
        mOrigLength[i] = 0;
    }
    CopyOriginalToCur();
    for (int i = 0; i < 8; i++) {
        unkStructs[i].unk0 = 0;
        unkStructs[i].unk4.Zero();
    }
    unk1a0.Reset();
}

CharCollide::~CharCollide() {}

BEGIN_HANDLERS(CharCollide)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharCollide)
    SYNC_PROP_MODIFY(shape, (int &)mShape, SyncShape())
    SYNC_PROP(flags, mFlags)
    SYNC_PROP_MODIFY(radius0, mOrigRadius[0], SyncShape())
    SYNC_PROP_MODIFY(radius1, mOrigRadius[1], SyncShape())
    SYNC_PROP_MODIFY(length0, mOrigLength[0], SyncShape())
    SYNC_PROP_MODIFY(length1, mOrigLength[1], SyncShape())
    SYNC_PROP_MODIFY(mesh, mMesh, SyncShape())
    SYNC_PROP_MODIFY(mesh_y_bias, mMeshYBias, SyncShape())
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharCollide)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mShape;
    bs << mOrigRadius[0];
    bs << mOrigLength[0];
    bs << mOrigLength[1];
    bs << mFlags;
    bs << mCurRadius[0];
    bs << mOrigRadius[1];
    bs << mCurRadius[1];
    bs << mCurLength[0];
    bs << mCurLength[1];
    bs << unk1a0;
    bs << mMesh;
    for (int i = 0; i < 8; i++) {
        bs << unkStructs[i].unk0;
        bs << unkStructs[i].unk4;
    }
    bs << mDigest;
    bs << mMeshYBias;
END_SAVES

BEGIN_LOADS(CharCollide)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    bs >> (int &)mShape;
    bs >> mOrigRadius[0];
    if (d.rev > 4)
        bs >> mOrigLength[0];
    if (d.rev > 2)
        bs >> mOrigLength[1];
    if (d.rev > 1)
        bs >> mFlags;
    else
        mFlags = 0;
    if (d.rev > 3)
        bs >> mCurRadius[0];
    else
        mCurRadius[0] = mOrigRadius[0];

    if (d.rev > 5) {
        bs >> mOrigRadius[1];
        bs >> mCurRadius[1];
        bs >> mCurLength[0];
        bs >> mCurLength[1];
        bs >> unk1a0;
        bs >> mMesh;
        for (int i = 0; i < 8; i++) {
            bs >> unkStructs[i].unk0;
            bs >> unkStructs[i].unk4;
        }
        bs >> mDigest;
        d >> mMeshYBias;
        if (d.rev < 7)
            CopyOriginalToCur();
    } else {
        mOrigRadius[1] = mOrigRadius[0];
        CopyOriginalToCur();
    }
END_LOADS

BEGIN_COPYS(CharCollide)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(CharCollide)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mShape)
        COPY_MEMBER(mFlags)
        memcpy(mOrigRadius, c->mOrigRadius, 8);
        memcpy(mOrigLength, c->mOrigLength, 8);
        memcpy(mCurRadius, c->mCurRadius, 8);
        memcpy(mCurLength, c->mCurLength, 8);
        COPY_MEMBER(unk1a0)
        COPY_MEMBER(mMeshYBias)
        COPY_MEMBER(mMesh)
    END_COPYING_MEMBERS
END_COPYS

void CharCollide::SyncShape() {
    mCurLength[0] = Min(mCurLength[1], mCurLength[0]);
    CopyOriginalToCur();
}

void CharCollide::CopyOriginalToCur() {
    memcpy(mCurRadius, mOrigRadius, 8);
    memcpy(mCurLength, mOrigLength, 8);
}
