#include "flow/FlowSwitchCase.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"

FlowSwitchCase::FlowSwitchCase()
    : mToValue(0), mFromValue(0), mOperator(kEqual), mUseLastValue(0),
      mUnregisterParent(0), unk9a(0) {
    mParent = nullptr;
}

FlowSwitchCase::~FlowSwitchCase() { TheFlowMgr->CancelCommand(this); }

BEGIN_HANDLERS(FlowSwitchCase)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSwitchCase)
    SYNC_PROP_MODIFY(use_last_value, mUseLastValue, UseLastValueChanged())
    SYNC_PROP(operator,(int &) mOperator)
    SYNC_PROP(to_value, mToValue)
    SYNC_PROP(from_value, mFromValue)
    SYNC_PROP(unregister_parent, mUnregisterParent)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSwitchCase)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FlowNode)
    if (mToValue.Node().Type() == kDataObject) {
        bs << (mToValue.Node().Type() == kDataObject);
        bs << mToValue.Node();
    } else {
        bs << mToValue.Node();
    }
    if (mFromValue.Node().Type() == kDataObject) {
        bs << (mFromValue.Node().Type() == kDataObject);
        bs << mFromValue.Node();
    } else {
        bs << mFromValue.Node();
    }
    bs << mUseLastValue;
    bs << mUnregisterParent;
END_SAVES

BEGIN_LOADS(FlowSwitchCase)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (gRev < 2) {
        DataNode n;
        bs >> n;
        mFromValue = n;
    }
END_LOADS
