#include "flow/PropertyEventProvider.h"
#include "obj/Object.h"

PropertyEventProvider::PropertyEventProvider() {}

BEGIN_HANDLERS(PropertyEventProvider)
    HANDLE_VIRTUAL_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PropertyEventProvider)
    SYNC_VIRTUAL_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(PropertyEventProvider)
    SAVE_REVS(1, 0)
    SAVE_VIRTUAL_SUPERCLASS(Hmx::Object)
END_SAVES

BEGIN_COPYS(PropertyEventProvider)
    COPY_VIRTUAL_SUPERCLASS(Hmx::Object)
    CREATE_COPY(PropertyEventProvider)
END_COPYS

BEGIN_LOADS(PropertyEventProvider)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    if (gRev < 1 || ClassName() == StaticClassName())
        LOAD_SUPERCLASS(Hmx::Object)
END_LOADS
