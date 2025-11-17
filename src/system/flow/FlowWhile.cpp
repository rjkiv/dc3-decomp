#include "flow/FlowWhile.h"
#include "flow/FlowNode.h"
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

BEGIN_LOADS(FlowWhile)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowSwitch)
END_LOADS

bool FlowWhile::IsRunning() { return (unk14 || !mRunningNodes.empty()) ? true : false; }

void FlowWhile::RequestStopCancel() {
    FlowNode::RequestStopCancel();
    if (!unk14)
        PropertyEventListener::RegisterEvents(this);
}

void FlowWhile::Deactivate(bool b) {
    if (!b)
        PropertyEventListener::UnregisterEvents(this);
    unk14 = false;
    FlowNode::Deactivate(b);
}
