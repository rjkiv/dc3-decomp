#include "flow/FlowOutPort.h"
#include "flow/Flow.h"
#include "flow/FlowLabel.h"
#include "flow/FlowNode.h"
#include "obj/Dir.h"
#include "obj/Msg.h"

FlowOutPort::FlowOutPort() : mImmediateRelease(0), mStop(0), mExposed(0) {}
FlowOutPort::~FlowOutPort() {}

BEGIN_HANDLERS(FlowOutPort)
    HANDLE_ACTION(on_flow_finished, ChildFinished(_msg->Obj<FlowNode>(2)))
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowOutPort)
    SYNC_PROP(immediate_release, mImmediateRelease)
    SYNC_PROP_MODIFY(label, mLabel, UpdatePortMapping())
    SYNC_PROP(stop, mStop)
    SYNC_PROP(exposed, mExposed)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowOutPort)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mLabel;
    bs << mImmediateRelease;
    bs << mStop;
    bs << mExposed;
END_SAVES

BEGIN_COPYS(FlowOutPort)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowOutPort)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLabel)
        COPY_MEMBER(mImmediateRelease)
        COPY_MEMBER(mStop)
        COPY_MEMBER(mExposed)
    END_COPYING_MEMBERS
    UpdatePortMapping();
END_COPYS

BEGIN_LOADS(FlowOutPort)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(FlowNode)
    bs >> mLabel;
    bsrev >> mImmediateRelease;
    if (gRev > 0) {
        bsrev >> mStop;
    }
    if (gRev > 1) {
        bsrev >> mExposed;
    }
    UpdatePortMapping();
END_LOADS

bool FlowOutPort::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    PushDrivenProperties();
    if (GetOwnerFlow()) {
        FlowLabel *label = GetOwnerFlow()->GetLabelForSym(mLabel.c_str());
        if (label) {
            if (mStop) {
                label->RequestStop();
                return false;
            }
            mRunningNodes.push_back(label);
            if (!label->Activate(this) || mImmediateRelease) {
                mRunningNodes.remove(label);
            }
        }
    }
    if (mExposed) {
        Message msg(MakeString("port_%s", mLabel.c_str()));
        GetTopFlow()->Dir()->Handle(msg, false);
    }
    return !mRunningNodes.empty();
}

void FlowOutPort::Deactivate(bool b1) {
    FLOW_LOG("Deactivated\n");
    if (GetOwnerFlow() && !mStop) {
        FlowLabel *label = GetOwnerFlow()->GetLabelForSym(mLabel.c_str());
        if (label) {
            label->Deactivate(b1);
        }
    }
    mRunningNodes.clear();
}

void FlowOutPort::ChildFinished(FlowNode *node) {
    FLOW_LOG("ChildFinished, class: %s\n", node->ClassName());
    if (!mRunningNodes.empty()) {
        FlowNode::ChildFinished(node);
    }
}

void FlowOutPort::RequestStop() {
    FLOW_LOG("RequestStop\n");
    unk58 = true;
    if (GetOwnerFlow() && !mStop) {
        FlowLabel *label = GetOwnerFlow()->GetLabelForSym(mLabel.c_str());
        if (label) {
            label->RequestStop();
        }
    }
}

void FlowOutPort::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    unk58 = false;
    if (GetOwnerFlow() && !mStop) {
        FlowLabel *label = GetOwnerFlow()->GetLabelForSym(mLabel.c_str());
        if (label) {
            label->RequestStopCancel();
        }
    }
}

void FlowOutPort::UpdatePortMapping() {
    if (GetOwnerFlow()) {
        GetOwnerFlow()->RefreshPortLabelLists();
    }
}
