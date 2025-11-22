#include "world/SpotlightEnder.h"
#include "SpotlightDrawer.h"
#include "SpotlightEnder.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"

SpotlightEnder::SpotlightEnder() { mOrder = -900; }

BEGIN_HANDLERS(SpotlightEnder)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(SpotlightEnder)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SpotlightEnder)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
END_SAVES

BEGIN_COPYS(SpotlightEnder)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
END_COPYS

BEGIN_LOADS(SpotlightEnder)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
END_LOADS

void SpotlightEnder::DrawShowing() { SpotlightDrawer::Current()->UpdateBoxMap(); }
