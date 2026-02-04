#include "rndobj/SoftParticles.h"
#include "Rnd_NG.h"
#include "obj/Object.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"
#include "rndobj/SoftParticleBuffer.h"

RndSoftParticles::RndSoftParticles()
    : mParticles(this), mBlend(BaseMaterial::kBlendSrcAlphaAdd) {}

BEGIN_HANDLERS(RndSoftParticles)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndSoftParticles)
    SYNC_PROP(particles, mParticles)
    SYNC_PROP(blend_mode, (int &)mBlend)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndSoftParticles)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mParticles;
    bs << mBlend;
END_SAVES

BEGIN_COPYS(RndSoftParticles)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndSoftParticles)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mParticles)
        COPY_MEMBER(mBlend)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(RndSoftParticles)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mParticles;
    bs >> (int &)mBlend;
END_LOADS

void RndSoftParticles::DrawShowing() {
    RndSoftParticleBuffer *buffer = TheNgRnd.ParticleBuffer();
    if (buffer) {
        FOREACH (it, mParticles) {
            buffer->Queue(*it, mBlend);
        }
    }
}

void RndSoftParticles::ListDrawChildren(std::list<RndDrawable *> &draws) {
    FOREACH (it, mParticles) {
        draws.push_back(*it);
    }
}
