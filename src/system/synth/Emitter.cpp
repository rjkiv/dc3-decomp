#include "synth/Emitter.h"
#include "Emitter.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/Loader.h"

namespace {
    RndDir *gIconDir;
}

SynthEmitter::SynthEmitter()
    : mSfx(this, 0), mInst(this, 0), mListener(this, 0), mEnabled(true), mRadInner(10.0f),
      mRadOuter(100.0f), mVolInner(0.0f), mVolOuter(-40.0f) {}

SynthEmitter::~SynthEmitter() { delete mInst; }

BEGIN_HANDLERS(SynthEmitter)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(SynthEmitter)
    SYNC_PROP_MODIFY(sfx, mSfx, delete mInst)
    SYNC_PROP_MODIFY(listener, mListener, delete mInst)
    SYNC_PROP_MODIFY(enabled, mEnabled, delete mInst)
    SYNC_PROP(outer_radius, mRadOuter)
    SYNC_PROP(inner_radius, mRadInner)
    SYNC_PROP(outer_volume, mVolOuter)
    SYNC_PROP(inner_volume, mVolInner)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SynthEmitter)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mSfx << mListener << mEnabled;
    bs << mRadOuter << mRadInner << mVolOuter << mVolInner;
END_SAVES

BEGIN_COPYS(SynthEmitter)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(SynthEmitter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSfx)
        COPY_MEMBER(mListener)
        COPY_MEMBER(mEnabled)
        COPY_MEMBER(mRadOuter)
        COPY_MEMBER(mRadInner)
        COPY_MEMBER(mVolOuter)
        COPY_MEMBER(mVolInner)
        delete mInst;
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(SynthEmitter)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    Hmx::Object::Load(bs);
    RndTransformable::Load(bs);
    RndDrawable::Load(bs);
    bs >> mSfx;
    bs >> mListener;
    bs >> mEnabled;
    if (d.rev >= 2) {
        bs >> mRadOuter;
        bs >> mRadInner;
    }
    if (d.rev >= 3) {
        bs >> mVolOuter;
        bs >> mVolInner;
    }
    delete mInst;
END_LOADS

void SynthEmitter::DrawShowing() {
    if (TheLoadMgr.EditMode()) {
        CheckLoadResources();
        gIconDir->SetLocalXfm(WorldXfm());
        gIconDir->DrawShowing();
    }
}

RndDrawable *SynthEmitter::CollideShowing(const Segment &s, float &dist, Plane &plane) {
    if (TheLoadMgr.EditMode()) {
        CheckLoadResources();
        RndDrawable *dirDraw = gIconDir->CollideShowing(s, dist, plane);
        if (dirDraw) {
            return this;
        }
    }
    return 0;
}

int SynthEmitter::CollidePlane(const Plane &plane) {
    if (TheLoadMgr.EditMode()) {
        CheckLoadResources();
        return gIconDir->CollidePlane(plane);
    } else
        return 0;
}

void SynthEmitter::CheckLoadResources() {
    MILO_ASSERT(TheLoadMgr.EditMode(), 0x84);
    if (!gIconDir) {
        FilePath fp(FileSystemRoot(), "milo/emitter.milo");
        ObjectDir *loadedDir = DirLoader::LoadObjects(fp, nullptr, nullptr);
        gIconDir = dynamic_cast<RndDir *>(loadedDir);
        MILO_ASSERT(gIconDir, 0x8C);
    }
}
