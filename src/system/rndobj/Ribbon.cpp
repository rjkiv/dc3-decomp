#include "rndobj/Ribbon.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "utl/Loader.h"

void RndRibbon::DrawShowing() {
    if (!unk_0x68)
        if (!TheLoadMgr.EditMode())
            return;
    // unk_0x4c->DrawShowing(); // SOMEWHERE is a mesh.
}

void RndRibbon::ExposeMesh() {}

BEGIN_HANDLERS(RndRibbon)
    HANDLE_ACTION(expose_mesh, ExposeMesh())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_COPYS(RndRibbon)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndRibbon)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDecay)
        COPY_MEMBER(mMaterial)
        COPY_MEMBER(mActive)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndRibbon)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bsrev >> unk_0x4c;
    bsrev >> mMaterial;
END_LOADS

void RndRibbon::SetActive(bool b) {
    if (mActive != b) {
        mTransforms.clear();
        mDecay = -1.0;
    }
    mActive = b;
}

BEGIN_PROPSYNCS(RndRibbon)
    SYNC_PROP_SET(active, mActive, SetActive(_val.Int()));
    SYNC_PROP(num_sides, mNumSides)
    SYNC_PROP(num_segments, mNumSegments)
    SYNC_PROP(mat, mMaterial)
    SYNC_PROP(width, mWidth)

    SYNC_PROP(follow_weight, mFollowWeight)
    SYNC_PROP(taper, mTaper)
    SYNC_PROP(decay, mDecay)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

RndRibbon::RndRibbon() : mDecay(-1.0f), mMaterial(this) {}
