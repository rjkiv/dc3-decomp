#include "rndobj/Ribbon.h"
#include "obj/Object.h"
#include "os/File.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "utl/Loader.h"

RndRibbon::RndRibbon()
    : unk48(-1.0f), mNumSides(4), mMat(this), mWidth(1), mDirty(1), mActive(true),
      mNumSegments(0), mDecay(1), mFollowA(this), mFollowB(this), mFollowWeight(0),
      mTaper(0) {
    mMesh = Hmx::Object::New<RndMesh>();
    mMesh->SetMutable(0x1F);
}

RndRibbon::~RndRibbon() { RELEASE(mMesh); }

BEGIN_HANDLERS(RndRibbon)
    HANDLE_ACTION(expose_mesh, ExposeMesh())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndRibbon)
    SYNC_PROP_SET(active, mActive, SetActive(_val.Int()));
    SYNC_PROP_MODIFY(num_sides, mNumSides, mDirty |= 1)
    SYNC_PROP_MODIFY(num_segments, mNumSegments, mDirty |= 1)
    SYNC_PROP_MODIFY(mat, mMat, mMesh->SetMat(mMat))
    SYNC_PROP_MODIFY(width, mWidth, mDirty |= 2)
    SYNC_PROP(follow_a, mFollowA)
    SYNC_PROP(follow_b, mFollowB)
    SYNC_PROP(follow_weight, mFollowWeight)
    SYNC_PROP(taper, mTaper)
    SYNC_PROP(decay, mDecay)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndRibbon)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mNumSides;
    bs << mMat;
    bs << mActive;
    bs << mWidth;
    bs << mNumSegments;
    bs << mFollowA;
    bs << mFollowB;
    bs << mFollowWeight;
    bs << mTaper;
    bs << mDecay;
END_SAVES

BEGIN_COPYS(RndRibbon)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndRibbon)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNumSides)
        COPY_MEMBER(mMat)
        COPY_MEMBER(mActive)
        COPY_MEMBER(mWidth)
        COPY_MEMBER(mNumSegments)
        COPY_MEMBER(mFollowA)
        COPY_MEMBER(mFollowB)
        COPY_MEMBER(mFollowWeight)
        COPY_MEMBER(mTaper)
        COPY_MEMBER(mDecay)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndRibbon)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mNumSides;
    bs >> mMat;
    d >> mActive;
    bs >> mWidth;
    bs >> mNumSegments;
    bs >> mFollowA;
    bs >> mFollowB;
    bs >> mFollowWeight;
    d >> mTaper;
    bs >> mDecay;
    mDirty = 1;
    mMesh->SetMat(mMat);
END_LOADS

void RndRibbon::Poll() {
    if (mDirty & 1) {
        ConstructMesh();
        mDirty = 0;
    }
    UpdateChase();
    mDirty = 0;
}

void RndRibbon::DrawShowing() {
    if (mActive || TheLoadMgr.EditMode()) {
        mMesh->DrawShowing();
    }
}

void RndRibbon::SetActive(bool b) {
    if (mActive != b) {
        mTransforms.clear();
        unk48 = -1.0;
    }
    mActive = b;
}

void RndRibbon::ExposeMesh() {
    if (!mMesh->Dir()) {
        const char *base = FileGetBase(Name());
        mMesh->SetName(MakeString("%s_mesh.mesh", base), Dir());
    }
}
