#include "world/PostProcer.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd.h"

PostProcer::PostProcer() {}

BEGIN_HANDLERS(PostProcer)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PostProcer)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(PostProcer)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
END_SAVES

BEGIN_COPYS(PostProcer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
END_COPYS

BEGIN_LOADS(PostProcer)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
END_LOADS

void PostProcer::DrawShowing() {
    if (!TheRnd.WorldEnded()) {
        RndCam *cur = RndCam::Current();
        TheRnd.EndWorld();
        if (cur)
            cur->Select();
    }
}
