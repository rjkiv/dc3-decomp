#include "flow/FlowLabel.h"
#include "flow/FlowNode.h"
#include "flow/FlowQueueable.h"
#include "obj/Object.h"

FlowLabel::FlowLabel() {}
FlowLabel::~FlowLabel() {}

BEGIN_HANDLERS(FlowLabel)
    HANDLE_SUPERCLASS(FlowQueueable)
END_HANDLERS

BEGIN_PROPSYNCS(FlowLabel)
    SYNC_PROP(label, mLabel)
    SYNC_SUPERCLASS(FlowQueueable)
END_PROPSYNCS

BEGIN_SAVES(FlowLabel)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowQueueable)
    bs << mLabel;
    ObjPtr<FlowNode> node(this);
    node = mFlowParent;
    bs << node;
END_SAVES

BEGIN_COPYS(FlowLabel)
    COPY_SUPERCLASS(FlowQueueable)
    CREATE_COPY(FlowLabel)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLabel)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowLabel)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowQueueable)
    d >> mLabel;
    if (d.rev > 0) {
        ObjPtr<FlowNode> node(this);
        d >> node;
        if (mFlowParent != node) {
        }
    }
END_LOADS

bool FlowLabel::Activate(FlowNode *node) {
    unk58 = false;
    return FlowQueueable::Activate(node);
}
