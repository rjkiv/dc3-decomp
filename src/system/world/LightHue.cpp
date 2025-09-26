#include "world/LightHue.h"
#include "obj/Object.h"

LightHue::LightHue() : mLoader(nullptr) {}
LightHue::~LightHue() { delete mLoader; }

BEGIN_HANDLERS(LightHue)
    HANDLE(save_default, OnSaveDefault)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(LightHue)
    SYNC_PROP_MODIFY(path, mPath, Sync())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(LightHue)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPath;
    if (bs.Cached()) {
        bs << mKeys;
    }
END_SAVES

BEGIN_COPYS(LightHue)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(LightHue)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPath)
        COPY_MEMBER(mKeys)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(LightHue)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS
