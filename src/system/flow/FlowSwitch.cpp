#include "flow/FlowSwitch.h"
#include "FlowSwitch.h"
#include "FlowSwitchCase.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Object.h"

FlowSwitch::FlowSwitch() : mFirstValidCaseOnly(1) { unk64 = DataNode(kDataUndef, 0); }
FlowSwitch::~FlowSwitch() {}

BEGIN_HANDLERS(FlowSwitch)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSwitch)
    SYNC_PROP(value, mValue)
    SYNC_PROP(first_valid_case_only, mFirstValidCaseOnly)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSwitch)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mFirstValidCaseOnly;
END_SAVES

BEGIN_COPYS(FlowSwitch)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowSwitch)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFirstValidCaseOnly)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(FlowSwitch)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mFirstValidCaseOnly;
    VerifyTypes();
    PushDrivenProperties();
    unk64 = mValue;
END_LOADS

bool FlowSwitch::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    if (IsRunning()) {
        MILO_NOTIFY(
            "FlowSwitch re-entrance error, activated when already running, deactivating and aborting, check your logic"
        );
        Deactivate(false);
        return false;
    } else {
        PushDrivenProperties();
        if (mValue.NotNull()) {
            if (unk64.Type() != mValue.Type()) {
                unk64 = mValue;
            }
        } else {
            if (mValue.Type() == kDataObject) {
                unk64 = NULL_OBJ;
            } else {
                unk64 = 0;
            }
        }
        if (!ActivateTransitionCases(mValue, unk64)) {
            ActivateValueCases(mValue, unk64);
        }
        unk64 = mValue;
        return !mRunningNodes.empty();
    }
}

void FlowSwitch::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    PushDrivenProperties();
    FlowSwitchCase *switchCase = static_cast<FlowSwitchCase *>(n);
    if (switchCase && switchCase->Op() == kTransition) {
        mRunningNodes.remove(n);
        if (mValue != unk64) {
            DataNode dupe(unk64);
            unk64 = mValue;
            if (!ActivateTransitionCases(mValue, dupe)) {
                ActivateValueCases(mValue, dupe);
            }
        } else {
            ActivateValueCases(mValue, unk64);
        }
        if (mRunningNodes.empty()) {
            mFlowParent->ChildFinished(this);
        }
    } else {
        FlowNode::ChildFinished(n);
    }
}

void FlowSwitch::VerifyTypes() {
    DrivenPropertyEntry *entry = GetDrivenEntry("value");
    if (entry) {
        DataNode n = *Property(entry->Node().Array());
        DataNode n58;
        auto &op = entry->MathOps()[0];
        Hmx::Object *obj = op.GetUnk18();
        if (obj) {
            const DataNode *prop = obj->Property(op.RHS().Array(), false);
            if (prop) {
                n58 = *prop;
            } else {
                n58 = op.GetUnk0();
            }
        } else {
            n58 = op.GetUnk0();
        }
        if (!n58.CompatibleType(n.Type())) {
            SetProperty(entry->Node().Array(), n58);
        }
    }
}

bool FlowSwitch::ActivateTransitionCases(DataNode &n1, DataNode &n2) {
    FOREACH (it, mChildNodes) {
        FlowSwitchCase *cur = static_cast<FlowSwitchCase *>((FlowNode *)*it);
        if (cur->Op() == kTransition && cur->IsValidCase(this, &n1, &n2, true)) {
            ActivateChild(cur);
            if (mRequestingStop) {
                return mRunningNodes.empty();
            }
            if (!mRunningNodes.empty()) {
                return true;
            }
        }
    }
    return false;
}

void FlowSwitch::ActivateValueCases(DataNode &n1, DataNode &n2) {
    FLOW_LOG("Activating Value Cases\n");
    int i9 = 0;
    bool b2 = false;
    FOREACH (it, mChildNodes) {
        FlowSwitchCase *cur = static_cast<FlowSwitchCase *>((FlowNode *)*it);
        if (!cur->IsRunning() && cur->Op() != kTransition) {
            bool valid = cur->IsValidCase(this, &n1, &n2, false);
            if (valid) {
                i9++;
                if (!cur->HasChildNodes()) {
                    b2 = true;
                } else {
                    ActivateChild(cur);
                }
            } else if (b2 && cur->HasChildNodes()) {
                i9++;
                ActivateChild(cur);
                b2 = false;
            }
            if (valid && mFirstValidCaseOnly && !b2) {
                return;
            }
            if (cur->Op() == 7 && i9 == 0) {
                ActivateChild(cur);
            }
            if (mRequestingStop) {
                return;
            }
        }
    }
}
