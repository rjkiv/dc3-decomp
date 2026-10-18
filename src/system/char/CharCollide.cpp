#include "char/CharCollide.h"
#include "CharCollide.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include <cstring>

CharCollide::CharCollide() : mShape(kSphere), mFlags(0), mMesh(this), mMeshYBias(false) {
    for (int i = 0; i < 2; i++) {
        mOriginalLengths[i] = 0;
        mOriginalRadii[i] = 0;
    }
    CopyOriginalToCur();
    for (int i = 0; i < 8; i++) {
        mVertData[i].vertIndex = 0;
        mVertData[i].offset.Zero();
    }
    mInvParentXfm.Reset();
}

CharCollide::~CharCollide() {}

BEGIN_HANDLERS(CharCollide)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharCollide)
    SYNC_PROP_MODIFY(shape, (int &)mShape, SyncShape())
    SYNC_PROP(flags, mFlags)
    SYNC_PROP_MODIFY(radius0, mOriginalRadii[0], SyncShape())
    SYNC_PROP_MODIFY(radius1, mOriginalRadii[1], SyncShape())
    SYNC_PROP_MODIFY(length0, mOriginalLengths[0], SyncShape())
    SYNC_PROP_MODIFY(length1, mOriginalLengths[1], SyncShape())
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
    bs << mOriginalRadii[0];
    bs << mOriginalLengths[0];
    bs << mOriginalLengths[1];
    bs << mFlags;
    bs << mCurRadii[0];
    bs << mOriginalRadii[1];
    bs << mCurRadii[1];
    bs << mCurLengths[0];
    bs << mCurLengths[1];
    bs << mInvParentXfm;
    bs << mMesh;
    for (int i = 0; i < 8; i++) {
        bs << mVertData[i].vertIndex;
        bs << mVertData[i].offset;
    }
    bs << mMeshHash;
    bs << mMeshYBias;
END_SAVES

BEGIN_COPYS(CharCollide)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(CharCollide)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mShape)
        COPY_MEMBER(mFlags)
        memcpy(mOriginalRadii, c->mOriginalRadii, 8);
        memcpy(mOriginalLengths, c->mOriginalLengths, 8);
        memcpy(mCurRadii, c->mCurRadii, 8);
        memcpy(mCurLengths, c->mCurLengths, 8);
        COPY_MEMBER(mInvParentXfm)
        COPY_MEMBER(mMeshYBias)
        COPY_MEMBER(mMesh)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(CharCollide)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    d >> (int &)mShape;
    d >> mOriginalRadii[0];
    if (d.rev > 4)
        d >> mOriginalLengths[0];
    if (d.rev > 2)
        d >> mOriginalLengths[1];
    if (d.rev > 1)
        d >> mFlags;
    else
        mFlags = 0;
    if (d.rev > 3)
        d >> mCurRadii[0];
    else
        mCurRadii[0] = mOriginalRadii[0];

    if (d.rev > 5) {
        d >> mOriginalRadii[1];
        d >> mCurRadii[1];
        d >> mCurLengths[0];
        d >> mCurLengths[1];
        d >> mInvParentXfm;
        d >> mMesh;
        for (int i = 0; i < 8; i++) {
            d >> mVertData[i].vertIndex;
            d >> mVertData[i].offset;
        }
        d >> mMeshHash;
        d >> mMeshYBias;
        if (d.rev < 7)
            CopyOriginalToCur();
    } else {
        mOriginalRadii[1] = mOriginalRadii[0];
        CopyOriginalToCur();
    }
END_LOADS

void CharCollide::Highlight() {
    Hmx::Color white(1, 1, 1);
    Hmx::Color red(1, 0, 0);
    switch (mShape) {
    case kPlane: {
        Plane plane(WorldXfm().v, WorldXfm().m.x);
        UtilDrawPlane(plane, WorldXfm().v, red, 1, 12, false);
        break;
    }
    case kSphere:
    case kInsideSphere: {
        UtilDrawSphere(WorldXfm().v, mOriginalRadii[0], red, nullptr);
        UtilDrawSphere(WorldXfm().v, mCurRadii[0], white, nullptr);
        break;
    }
    case kCigar:
    case kInsideCigar: {
        UtilDrawCigar(WorldXfm(), mOriginalRadii, mOriginalLengths, red, 8);
        UtilDrawCigar(WorldXfm(), mCurRadii, mCurLengths, white, 8);
        break;
    }
    default:
        break;
    }
    if (mMesh) {
        int l7 = NumSpheres(mShape) * 4;
        for (int i = 0; i < l7; i++) {
            UtilDrawSphere(
                mMesh->Verts()[mVertData[i].vertIndex].pos,
                0.1f,
                Hmx::Color(0, 0, 1),
                nullptr
            );
        }
    }
}

void CharCollide::SyncShape() {
    if (mCurLengths[0] > mCurLengths[1]) {
        mCurLengths[0] = mCurLengths[1];
    }
    CopyOriginalToCur();
}

void CharCollide::CopyOriginalToCur() {
    memcpy(mCurRadii, mOriginalRadii, 8);
    memcpy(mCurLengths, mOriginalLengths, 8);
}

int CharCollide::NumSpheres(Shape s) const {
    if (s == kCigar || s == kInsideCigar) {
        return 2;
    } else {
        return s == kSphere || s == kInsideSphere;
    }
}
