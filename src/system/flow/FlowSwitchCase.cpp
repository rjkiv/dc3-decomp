#include "flow/FlowSwitchCase.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/FlowWhile.h"
#include "flow/Flow.h"
#include "obj/Data.h"
#include "obj/Object.h"

FlowSwitchCase::FlowSwitchCase()
    : mToValue(0), mFromValue(0), mOperator(kEquals), mUseLastValue(0),
      mUnregisterParent(0), mIsRunning(0) {
    mFlowParent = nullptr;
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
        mToValue.Node().Save(bs);
    } else {
        bs << mToValue.Node().Type();
        mToValue.Node().Save(bs);
    }
    bs << mOperator;
    if (mFromValue.Node().Type() == kDataObject) {
        mFromValue.Node().Save(bs);
    } else {
        bs << mFromValue.Node().Type();
        mFromValue.Node().Save(bs);
    }
    bs << mUseLastValue;
    bs << mUnregisterParent;
END_SAVES

BEGIN_COPYS(FlowSwitchCase)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSwitchCase)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mToValue)
        COPY_MEMBER(mOperator)
        COPY_MEMBER(mFromValue)
        COPY_MEMBER(mUseLastValue)
        COPY_MEMBER(mUnregisterParent)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(FlowSwitchCase)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (d.rev < 2) {
        DataNode n;
        d >> n;
        mToValue = n;
    } else {
        DataType t;
        d >> (int &)t;
        if (t == kDataObject) {
            Flow *flow = GetOwnerFlow();
            if (!flow) {
                flow = dynamic_cast<Flow *>(this);
            }
            mToValue = FlowNode::LoadObjectFromMainOrDir(bs, flow->LoadingDir());
        } else {
            DataNode n;
            d >> n;
            mToValue = n;
        }
    }
    d >> (int &)mOperator;
    if (d.rev < 2) {
        DataNode n;
        d >> n;
        mFromValue = n;
    } else {
        DataType t;
        d >> (int &)t;
        if (t == kDataObject) {
            Flow *flow = GetOwnerFlow();
            if (!flow) {
                flow = dynamic_cast<Flow *>(this);
            }
            mFromValue = FlowNode::LoadObjectFromMainOrDir(bs, flow->LoadingDir());
        } else {
            DataNode n;
            d >> n;
            mFromValue = n;
        }
    }
    if (d.rev > 0) {
        d >> mUseLastValue;
    }
    if (d.rev > 2) {
        d >> mUnregisterParent;
    }
END_LOADS

bool FlowSwitchCase::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (mFlowParent->ClassName() == FlowWhile::StaticClassName()
        && mOperator != kTransition) {
        mIsRunning = true;
        FlowNode::Activate();
        if (mUnregisterParent) {
            TheFlowMgr->QueueCommand(this, kStarting);
        }
        return true;
    } else {
        return FlowNode::Activate();
    }
}

void FlowSwitchCase::Deactivate(bool b1) {
    mIsRunning = false;
    FlowNode::Deactivate(b1);
}

void FlowSwitchCase::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    if (mIsRunning && mOperator != kTransition) {
        mRunningNodes.remove(n);
    } else {
        mIsRunning = false;
        FlowNode::ChildFinished(n);
    }
}

void FlowSwitchCase::RequestStop() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStop();
    if (mIsRunning) {
        TheFlowMgr->QueueCommand(this, kRequestStop);
    }
}

void FlowSwitchCase::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    TheFlowMgr->CancelCommand(this);
    FlowNode::RequestStopCancel();
}

void FlowSwitchCase::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (qs == kStarting) {
        FlowWhile *propEventListener = static_cast<FlowWhile *>(mFlowParent);
        propEventListener->UnregisterSelf();
        if (!mIsRunning)
            return;
    } else if (qs != kRequestStop) {
        return;
    }
    mIsRunning = false;
    if (!FlowNode::IsRunning() && mFlowParent->HasRunningNode(this)) {
        mFlowParent->ChildFinished(this);
    }
}

bool FlowSwitchCase::IsRunning() {
    if (mIsRunning)
        return true;
    else
        return FlowNode::IsRunning();
}

void FlowSwitchCase::UseLastValueChanged() {
    if (mUseLastValue) {
        DrivenPropertyEntry *entry = GetDrivenEntry("to_value");
        if (entry) {
            auto it = mDrivenProperties.begin();
            for (; it != mDrivenProperties.end() && entry != it; ++it)
                ;
            if (it != mDrivenProperties.end()) {
                mDrivenProperties.erase(it);
            }
        }
    }
}

// should call the DataNode operator> overload, but it's used elsewhere
FORCEINLINE bool GTHack(const DataNode &n1, const DataNode &n2) {
    if ((n1.Type() == kDataInt || n1.Type() == kDataFloat)
        && (n2.Type() == kDataInt || n2.Type() == kDataFloat)) {
        return n1.LiteralFloat() > n2.LiteralFloat();
    }
    return false;
}

bool FlowSwitchCase::IsValidCase(
    FlowNode *parent, DataNode *lhs, const DataNode *lastValue, bool trans
) {
    bool ret;
    PushDrivenProperties();
    if (mOperator == kTransition) {
        if (mUseLastValue) {
            mFromValue = *lastValue;
        }
        if (lhs->Type() != mToValue.Node().Type()
            || lhs->Type() != mFromValue.Node().Type()) {
            return false;
        } else {
            ret = *lhs == mToValue.Node() && *lastValue == mFromValue.Node();
        }
    } else {
        if (mUseLastValue) {
            mToValue = *lastValue;
        }
        switch (mOperator) {
        case kEquals: {
            ret = *lhs == mToValue.Node();
            break;
        }
        case kNotEquals: {
            ret = *lhs != mToValue.Node();
            break;
        }
        case kGreaterThan: {
            ret = GTHack(*lhs, mToValue.Node());
            break;
        }
        case kGreaterThanOrEquals: {
            ret = *lhs >= mToValue.Node();
            break;
        }
        case kLessThan: {
            ret = *lhs < mToValue.Node();
            break;
        }
        case kLessThanOrEquals: {
            ret = *lhs <= mToValue.Node();
            break;
        }
        default:
            return false;
        }
    }
    return ret;
}
