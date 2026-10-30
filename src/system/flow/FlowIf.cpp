#include "flow/FlowIf.h"
#include "FlowSwitchCase.h"
#include "flow/FlowNode.h"
#include "flow/Flow.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"

FlowIf::FlowIf() : mFirstValue(0), mSecondValue(0), mOperator(FlowSwitchCase::kEquals) {}
FlowIf::~FlowIf() {}

BEGIN_HANDLERS(FlowIf)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowIf)
    SYNC_PROP(first_value, mFirstValue)
    SYNC_PROP(second_value, mSecondValue)
    SYNC_PROP(operator,(int &) mOperator)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowIf)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FlowNode)
    if (mFirstValue.Node().Type() == kDataObject) {
        mFirstValue.Node().Save(bs);
    } else {
        bs << mFirstValue.Node().Type();
        mFirstValue.Node().Save(bs);
    }
    if (mSecondValue.Node().Type() == kDataObject) {
        mSecondValue.Node().Save(bs);
    } else {
        bs << mSecondValue.Node().Type();
        mSecondValue.Node().Save(bs);
    }
    bs << mOperator;
END_SAVES

BEGIN_COPYS(FlowIf)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowIf)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFirstValue)
        COPY_MEMBER(mSecondValue)
        COPY_MEMBER(mOperator)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(FlowIf)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (d.rev < 3) {
        DataNode n1, n2;
        n1.Load(bs);
        n2.Load(bs);
        mFirstValue = n1;
        mSecondValue = n2;
    } else {
        int type1;
        bs >> type1;
        if (type1 == kDataObject) {
            Flow *owner = GetOwnerFlow();
            if (!owner) {
                owner = dynamic_cast<Flow *>(this);
            }
            mFirstValue = LoadObjectFromMainOrDir(d, owner->LoadingDir());
        } else {
            DataNode val;
            val.Load(bs);
            mFirstValue = val;
        }
        int type2;
        bs >> type2;
        if (type2 == kDataObject) {
            Flow *owner = GetOwnerFlow();
            if (!owner) {
                owner = dynamic_cast<Flow *>(this);
            }
            mSecondValue = LoadObjectFromMainOrDir(d, owner->LoadingDir());
        } else {
            DataNode val;
            val.Load(bs);
            mSecondValue = val;
        }
    }
    bs >> (int &)mOperator;
    if (d.rev == 1) {
        int x;
        bs >> x;
    }
END_LOADS

// should call the DataNode operator> overload, but it's used elsewhere
FORCEINLINE bool GTHack(const DataNode &n1, const DataNode &n2) {
    if ((n1.Type() == kDataInt || n1.Type() == kDataFloat)
        && (n2.Type() == kDataInt || n2.Type() == kDataFloat)) {
        return n1.LiteralFloat() > n2.LiteralFloat();
    }
    return false;
}

bool FlowIf::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (IsRunning()) {
        MILO_NOTIFY(
            "FlowIf re-entrance error, activated when already running, deactivating and aborting, check your logic"
        );
        Deactivate(false);
        return false;
    }
    PushDrivenProperties();
    bool activate = false;
    switch (mOperator) {
    case FlowSwitchCase::kEquals:
        activate = mFirstValue.Node() == mSecondValue.Node();
        break;
    case FlowSwitchCase::kNotEquals:
        activate = mFirstValue.Node() != mSecondValue.Node();
        break;
    case FlowSwitchCase::kGreaterThan: {
        activate = GTHack(mFirstValue.Node(), mSecondValue.Node());
        break;
    }
    case FlowSwitchCase::kGreaterThanOrEquals: {
        activate = mFirstValue.Node() >= mSecondValue.Node();
        break;
    }
    case FlowSwitchCase::kLessThan: {
        activate = mFirstValue.Node() < mSecondValue.Node();
        break;
    }
    case FlowSwitchCase::kLessThanOrEquals: {
        activate = mFirstValue.Node() <= mSecondValue.Node();
        break;
    }
    default:
        break;
    }
    if (activate) {
        FlowNode::Activate();
    }
    return !mRunningNodes.empty();
}
