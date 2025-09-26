#include "flow/FlowRun.h"
#include "FlowRun.h"
#include "flow/FlowNode.h"
#include "obj/Dir.h"
#include "obj/Object.h"

FlowRun::FlowRun()
    : mTargetDir(this), mTarget(this), mTargetName(""), mStop(false),
      mImmediateRelease(false) {}

FlowRun::~FlowRun() {}

BEGIN_HANDLERS(FlowRun)
    HANDLE_ACTION(on_flow_finished, ChildFinished(_msg->Obj<FlowNode>(2)))
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowRun)
    SYNC_PROP_MODIFY(target_dir, mTargetDir, OnTargetDirChange())
    SYNC_PROP_MODIFY(target, mTarget, OnTargetChange())
    SYNC_PROP(stop, mStop)
    SYNC_PROP(immediate_release, mImmediateRelease)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowRun)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mTargetDir;
    ResolveTarget();
    bs << mTargetName;
    bs << mStop;
    bs << mImmediateRelease;
END_SAVES

BEGIN_COPYS(FlowRun)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowRun)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTargetDir)
        COPY_MEMBER(mTargetName)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mStop)
        COPY_MEMBER(mImmediateRelease)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FlowRun)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (gRev < 2) {
        Hmx::Object *obj = LoadObjectFromMainOrDir(bs, Dir());
        if (obj) {
            mTargetDir = dynamic_cast<ObjectDir *>(obj);
        }
        mTarget = mTarget.LoadFromMainOrDir(bs);
    } else {
        mTargetDir.LoadFromMainOrDir(bs);
        bs >> mTargetName;
        mTarget.Reset();
    }
    bsrev >> mStop;
    bsrev >> mImmediateRelease;
END_LOADS

void FlowRun::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    if (!mRunningNodes.empty()) {
        FlowNode::ChildFinished(node);
    }
}

void FlowRun::RequestStop() {
    FLOW_LOG("RequestStop\n");
    unk58 = true;
    mTarget->RequestStop();
}

void FlowRun::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    unk58 = false;
    mTarget->RequestStopCancel();
}

void FlowRun::OnTargetDirChange() {
    mTargetDir.Reset();
    mTargetName = "";
}

void FlowRun::OnTargetChange() {
    if (mTarget)
        mTargetName = mTarget->Name();
    else
        mTargetName = "";
    return;
}
