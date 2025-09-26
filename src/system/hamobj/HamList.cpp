#include "hamobj/HamList.h"
#include "obj/Object.h"
#include "ui/UIList.h"

HamList::HamList() {}

BEGIN_HANDLERS(HamList)
    HANDLE_SUPERCLASS(UIList)
END_HANDLERS

BEGIN_PROPSYNCS(HamList)
    SYNC_SUPERCLASS(UIList)
END_PROPSYNCS

BEGIN_SAVES(HamList)
    SAVE_REVS(0x13, 0)
    SAVE_SUPERCLASS(UIList)
END_SAVES

BEGIN_COPYS(HamList)
    COPY_SUPERCLASS(UIList)
    CREATE_COPY(HamList)
END_COPYS

BEGIN_LOADS(HamList)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void HamList::Init() { REGISTER_OBJ_FACTORY(HamList); }
