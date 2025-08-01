#include "flow/FlowNode.h"
#include "obj/ObjMacros.h"
#include "obj/ObjPtr_p.h"
#include "os/Debug.h"

BEGIN_HANDLERS(FlowNode)
    HANDLE_ACTION("activate", Activate());
    HANDLE_ACTION("deactivate", Deactivate(false));
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(FlowNode)
    SYNC_PROP_SET("comment", mNote, SetNote(_val.Str()))
    SYNC_PROP("debug_output", mDebugOutput)
    SYNC_PROP("debug_comment", mDebugComment)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void FlowNode::Deactivate(bool) {
    if (mDebugOutput) {
        MILO_LOG("%s%s", ClassName(), MakeString("Deactivated\n"));
        if (!mDebugComment.empty()) {
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());
        }
    }
}

void FlowNode::ActivateChild(FlowNode *child) {
    mChildren.insert(mChildren.end(), child);
    if (child->Activate()) {
        return;
    }
    if (mDebugOutput) {
        MILO_LOG(
            "%s: %s",
            ClassName(),
            MakeString(
                "Activated Child %s, which ran in full immediately.\n", child->ClassName()
            )
        );
        if (!mDebugComment.empty()) {
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());
        }
    }
}

bool FlowNode::Activate() {
    if (mDebugOutput) {
        MILO_LOG("%s: %s", ClassName(), MakeString("Activating Children\n"));
        if (!mDebugComment.empty()) {
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());
        }
    }
    return false;
}

FlowNode::FlowNode()
    : mVec1(this, (EraseMode)0, kObjListNoNull), mChildren(this, kObjListNoNull) {}

FlowNode::~FlowNode() {}

void FlowNode::Save(BinStream &bs) {
    bs << 2;
    Hmx::Object::Save(bs);
}

void FlowNode::Load(BinStream &bs) {}
