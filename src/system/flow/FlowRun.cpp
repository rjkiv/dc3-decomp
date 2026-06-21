#include "flow/FlowRun.h"
#include "FlowRun.h"
#include "flow/FlowNode.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"

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

INIT_REVS(2, 0)

BEGIN_LOADS(FlowRun)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (d.rev < 2) {
        Hmx::Object *obj = FlowNode::LoadObjectFromMainOrDir(bs, Dir());
        if (obj) {
            mTargetDir = dynamic_cast<ObjectDir *>(obj);
        }
        mTarget = mTarget.LoadFromMainOrDir(bs);
    } else {
        mTargetDir.LoadFromMainOrDir(bs);
        d >> mTargetName;
        mTarget.Reset();
    }
    d >> mStop;
    d >> mImmediateRelease;
END_LOADS

bool FlowRun::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    PushDrivenProperties();
    ResolveTarget();
    if (mTarget) {
        if (mStop) {
            mTarget->RequestStop();
        } else if (mImmediateRelease) {
            mTarget->Activate(nullptr);
        } else {
            mRunningNodes.push_back(mTarget);
            if (mTarget->Activate(this)) {
                return true;
            } else {
                mRunningNodes.remove(mTarget);
            }
        }
    }
    return false;
}

void FlowRun::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    if (!mRunningNodes.empty()) {
        FlowNode::ChildFinished(node);
    }
}

void FlowRun::RequestStop() {
    FLOW_LOG("RequestStop\n");
    mRequestingStop = true;
    mTarget->RequestStop();
}

void FlowRun::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    mRequestingStop = false;
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

void FlowRun::ResolveTarget() {
    if (!mTarget && !mTargetName.empty()) {
        ObjectDir *targetDir = mTargetDir;
        if (!targetDir) {
            Flow *flow = GetOwnerFlow();
            DirLoader *dl = flow->Loader();
            if (dl) {
                targetDir = dl->ProxyDir();
            } else {
                targetDir = flow->Dir();
            }
            MILO_ASSERT(targetDir, 0x72);
        }
        mTarget = targetDir->Find<Flow>(mTargetName.c_str(), false);
    }
}
