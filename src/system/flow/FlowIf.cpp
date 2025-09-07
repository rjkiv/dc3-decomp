#include "flow/FlowIf.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowIf::FlowIf() : mValue1(0), mValue2(0), mOperator(kEqual) {}
FlowIf::~FlowIf() {}

BEGIN_HANDLERS(FlowIf)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowIf)
    SYNC_PROP(first_value, mValue1)
    SYNC_PROP(second_value, mValue2)
    SYNC_PROP(operator,(int &) mOperator)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS
