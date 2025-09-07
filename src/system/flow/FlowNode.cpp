#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"

float FlowNode::sIntensity = 1.0f;
bool FlowNode::sPushDrivenProperties = false;

FlowNode::FlowNode()
    : mVec1(this, (EraseMode)0, kObjListNoNull), mRunningNodes(this), mParent(nullptr),
      mDrivenPropEntries(this), unk58(0) {
    mDebugOutput = false;
}

FlowNode::~FlowNode() {
    if (!mRunningNodes.empty()) {
        Deactivate(true);
    }
    for (ObjPtrVec<FlowNode>::iterator it = mVec1.begin(); it != mVec1.end(); ++it) {
        delete *it;
    }
}

BEGIN_HANDLERS(FlowNode)
    HANDLE_ACTION(activate, Activate());
    HANDLE_ACTION(deactivate, Deactivate(false));
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(FlowNode)
    SYNC_PROP_SET(comment, Note(), SetNote(_val.Str()))
    SYNC_PROP(debug_output, mDebugOutput)
    SYNC_PROP(debug_comment, mDebugComment)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(FlowNode)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
END_SAVES

BEGIN_COPYS(FlowNode)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FlowNode)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDrivenPropEntries)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowNode)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mVec1;

    int numEntries;
    bs >> numEntries;
    mDrivenPropEntries.clear();
    mDrivenPropEntries.reserve(numEntries);
    for (int i = 0; i < numEntries; i++) {
        DrivenPropertyEntry entry(this);
        entry.Load(bs, this);
        mDrivenPropEntries.push_back(entry);
    }
END_LOADS

void FlowNode::MiloPreRun(void) {}

FlowNode *FlowNode::DuplicateChild(FlowNode *) { return nullptr; }

void FlowNode::Deactivate(bool) { FLOW_LOG("Deactivated\n"); }

void FlowNode::PushDrivenProperties(void) { sPushDrivenProperties = true; }

void FlowNode::MoveIntoDir(ObjectDir *, ObjectDir *) {}

void FlowNode::ActivateChild(FlowNode *child) {
    mRunningNodes.push_back(child);
    if (!child->Activate()) {
        FLOW_LOG(
            "Activated Child %s, which ran in full immediately.\n", child->ClassName()
        );
        mRunningNodes.remove(child);
    }
}

bool FlowNode::Activate() {
    FLOW_LOG("Activating Children\n");
    return false;
}

void FlowNode::SetParent(class FlowNode *new_parent, bool b) {
    if (mParent != new_parent) {
        if (mParent != nullptr) {
            mParent->mVec1.remove(this);
        }
        mParent = new_parent;
        if (new_parent != nullptr && b) {
            new_parent->mVec1.push_back(this);
        }
    }
}

bool FlowNode::HasRunningNode(FlowNode *node) {
    return mRunningNodes.find(node) != mRunningNodes.end();
}

void FlowNode::UpdateIntensity() {
    for (ObjPtrList<FlowNode>::iterator it = mRunningNodes.begin();
         it != mRunningNodes.end();
         ++it) {
        (*it)->UpdateIntensity();
    }
}

void FlowNode::RequestStop() {
    FLOW_LOG("RequestStop\n");
    unk58 = true;
    for (ObjPtrList<FlowNode>::iterator it = mRunningNodes.begin();
         it != mRunningNodes.end();
         ++it) {
        (*it)->RequestStop();
    }
}

void FlowNode::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    unk58 = false;
    for (ObjPtrList<FlowNode>::iterator it = mRunningNodes.begin();
         it != mRunningNodes.end();
         ++it) {
        (*it)->RequestStopCancel();
    }
}

DrivenPropertyEntry *FlowNode::GetDrivenEntry(Symbol s) {
    DataArrayPtr ptr(new DataArray(1));
    ptr->Node(0) = s;
    return GetDrivenEntry(ptr);
}

void FlowNode::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    mRunningNodes.remove(node);
    if (mRunningNodes.empty()) {
        FLOW_LOG("Releasing\n");
        if (mParent)
            mParent->ChildFinished(this);
    }
}
