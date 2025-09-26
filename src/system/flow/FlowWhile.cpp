#include "flow/FlowWhile.h"
#include "flow/FlowSwitch.h"
#include "flow/PropertyEventListener.h"
#include "obj/Object.h"

FlowWhile::FlowWhile() : PropertyEventListener(this), unk88(0) {}
FlowWhile::~FlowWhile() {}

BEGIN_HANDLERS(FlowWhile)
    HANDLE_ACTION(reactivate, ReActivate())
    HANDLE_SUPERCLASS(FlowSwitch)
END_HANDLERS

BEGIN_PROPSYNCS(FlowWhile)
    SYNC_SUPERCLASS(FlowSwitch)
END_PROPSYNCS

BEGIN_SAVES(FlowWhile)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowSwitch)
END_SAVES
