#include "hamobj/HamList.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "ui/UIList.h"
#include "utl/BinStream.h"

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

INIT_REVS(19, 0)

void HamList::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(19, 0)
    if (d.rev <= 17) {
        UIList::PreLoadWithRev(d);
    } else {
        UIList::PreLoad(d.stream);
    }
    d.PushRev(this);
}

void HamList::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    UIList::PostLoad(d.stream);
    if (d.rev >= 18 && d.rev < 19) {
        ObjPtr<RndAnimatable> anim(this);
        d >> anim;
        d >> anim;
    }
}
