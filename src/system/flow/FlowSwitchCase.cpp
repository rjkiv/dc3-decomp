#include "flow/FlowSwitchCase.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/FlowWhile.h"
#include "flow/Flow.h"
#include "obj/Data.h"
#include "obj/Object.h"

FlowSwitchCase::FlowSwitchCase()
    : mToValue(0), mFromValue(0), mOperator(kEqual), mUseLastValue(0),
      mUnregisterParent(0), unk9a(0) {
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
            DirLoader *dl = flow->Loader();
            ObjectDir *dir = dl ? dl->ProxyDir() : flow->Dir();
            mToValue = FlowNode::LoadObjectFromMainOrDir(bs, dir);
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
            DirLoader *dl = flow->Loader();
            ObjectDir *dir = dl ? dl->ProxyDir() : flow->Dir();
            mFromValue = FlowNode::LoadObjectFromMainOrDir(bs, dir);
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
    unk58 = false;
    if (mFlowParent->ClassName() == FlowWhile::StaticClassName()
        && mOperator != kTransition) {
        unk9a = true;
        FlowNode::Activate();
        if (mUnregisterParent) {
            TheFlowMgr->QueueCommand(this, kQueue);
        }
        return true;
    } else {
        return FlowNode::Activate();
    }
}

void FlowSwitchCase::Deactivate(bool b1) {
    unk9a = false;
    FlowNode::Deactivate(b1);
}

void FlowSwitchCase::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    if (unk9a && mOperator != kTransition) {
        mRunningNodes.remove(n);
    } else {
        unk9a = false;
        FlowNode::ChildFinished(n);
    }
}

void FlowSwitchCase::RequestStop() {
    FLOW_LOG("RequestStop\n");
    FlowNode::RequestStop();
    if (unk9a) {
        TheFlowMgr->QueueCommand(this, kIgnore);
    }
}

void FlowSwitchCase::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    TheFlowMgr->CancelCommand(this);
    FlowNode::RequestStopCancel();
}

void FlowSwitchCase::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (qs == kQueue) {
        FlowWhile *propEventListener = static_cast<FlowWhile *>(mFlowParent);
        propEventListener->UnregisterSelf();
        if (!unk9a)
            return;
    } else if (qs != kIgnore) {
        return;
    }
    unk9a = false;
    if (!FlowNode::IsRunning() && mFlowParent->HasRunningNode(this)) {
        mFlowParent->ChildFinished(this);
    }
}

bool FlowSwitchCase::IsRunning() {
    if (unk9a)
        return true;
    else
        return FlowNode::IsRunning();
}

void FlowSwitchCase::UseLastValueChanged() {
    if (mUseLastValue) {
        DrivenPropertyEntry *entry = GetDrivenEntry("to_value");
        if (entry) {
            auto it = mDrivenPropEntries.begin();
            for (; it != mDrivenPropEntries.end() && entry != it; ++it)
                ;
            if (it != mDrivenPropEntries.end()) {
                mDrivenPropEntries.erase(it);
            }
        }
    }
}

bool FlowSwitchCase::IsValidCase(
    FlowNode *flownode, DataNode *n1, const DataNode *n2, bool b4
) {
    PushDrivenProperties();
    if (mOperator == kTransition) {
        if (mUseLastValue) {
            mFromValue = *n2;
        }
        if (n1->Type() != mFromValue.Node().Type()
            || n2->Type() != mToValue.Node().Type()) {
            return false;
        } else if (n1->Equal(mFromValue.Node(), nullptr, true)
                   && n2->Equal(mToValue.Node(), nullptr, true)) {
            return true;
        } else {
            return false;
        }
    } else {
        if (mUseLastValue) {
            mToValue = *n2;
        }
        switch (mOperator) {
        case kEqual: {
            return n1->Equal(mToValue.Node(), nullptr, true);
        }
        case kNotEqual: {
            return *n1 != mToValue.Node();
        }
        case kGreaterThan: {
            DataNode to = mToValue.Node();
            if ((n1->Type() == kDataInt || n1->Type() == kDataFloat)
                && (to.Type() == kDataInt || to.Type() == kDataFloat)) {
                return n1->LiteralFloat() > to.LiteralFloat();
            } else {
                return false;
            }
        }
        case kGreaterThanOrEqual: {
            DataNode to = mToValue.Node();
            if ((n1->Type() == kDataInt || n1->Type() == kDataFloat)
                && (to.Type() == kDataInt || to.Type() == kDataFloat)) {
                return n1->LiteralFloat() >= to.LiteralFloat();
            } else {
                return false;
            }
        }
        case kLessThan: {
            DataNode to = mToValue.Node();
            if ((n1->Type() == kDataInt || n1->Type() == kDataFloat)
                && (to.Type() == kDataInt || to.Type() == kDataFloat)) {
                return n1->LiteralFloat() < to.LiteralFloat();
            } else {
                return false;
            }
        }
        case kLessThanOrEqual: {
            DataNode to = mToValue.Node();
            if ((n1->Type() == kDataInt || n1->Type() == kDataFloat)
                && (to.Type() == kDataInt || to.Type() == kDataFloat)) {
                return n1->LiteralFloat() <= to.LiteralFloat();
            } else {
                return false;
            }
        }
        default:
            return false;
        }
    }
}
