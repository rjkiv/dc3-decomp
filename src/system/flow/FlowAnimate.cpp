#include "flow/FlowAnimate.h"
#include "flow/FlowLabel.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"

FlowAnimate::FlowAnimate()
    : mSpawnedTask(this), mAnim(this), mStopMode(kStopLastFrame),
      mQueuedStop(kStopImmediate), mBlend(0), mWait(0), mDelay(0), mFilterEnable(0),
      mFilterRate(RndAnimatable::k30_fps), mFilterStart(0), mFilterEnd(0),
      mFilterPeriod(0), mFilterScale(1), mStopOnLoop(0), mEase(kEaseLinear),
      mEasePower(2), mWrap(0), mImmediateRelease(0) {
    static Symbol range("range");
    mFilterType = range;
}

FlowAnimate::~FlowAnimate() { TheFlowMgr->CancelCommand(this); }

bool FlowAnimate::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mSpawnedTask) {
        if (mSpawnedTask && mSpawnedTask->GetEventReceiver() == this) {
            OnAnimEvent("interrupted");
        }
        mSpawnedTask = nullptr;
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(FlowAnimate)
    HANDLE_ACTION(on_anim_event, OnAnimEvent(_msg->Sym(2)))
    HANDLE_ACTION(on_flow_finished, ChildFinished(_msg->Obj<FlowNode>(2)))
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowAnimate)
    SYNC_PROP_MODIFY(anim, mAnim, ResetAnim())
    SYNC_PROP(blend, mBlend)
    SYNC_PROP(wait, mWait)
    SYNC_PROP(delay, mDelay)
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_PROP(enable, mFilterEnable)
    SYNC_PROP(rate, (int &)mFilterRate)
    SYNC_PROP(start, mFilterStart)
    SYNC_PROP(end, mFilterEnd)
    SYNC_PROP(scale, mFilterScale)
    SYNC_PROP(period, mFilterPeriod)
    SYNC_PROP(type, mFilterType)
    SYNC_PROP(ease, (int &)mEase)
    SYNC_PROP(ease_power, mEasePower)
    SYNC_PROP(wrap, mWrap)
    SYNC_PROP(immediate_release, mImmediateRelease)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowAnimate)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mAnim << mBlend << mWait << mDelay;
    bs << mStopMode << mFilterEnable;
    bs << mFilterRate << mFilterStart;
    bs << mFilterEnd << mFilterPeriod;
    bs << mFilterType;
    bs << mFilterScale << mEase << mEasePower;
    bs << mWrap;
    bs << mImmediateRelease;
END_SAVES

BEGIN_COPYS(FlowAnimate)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowAnimate)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAnim)
        COPY_MEMBER(mBlend)
        COPY_MEMBER(mDelay)
        COPY_MEMBER(mStopMode)
        COPY_MEMBER(mFilterEnable)
        COPY_MEMBER(mFilterRate)
        COPY_MEMBER(mFilterStart)
        COPY_MEMBER(mFilterEnd)
        COPY_MEMBER(mFilterPeriod)
        COPY_MEMBER(mFilterType)
        COPY_MEMBER(mFilterScale)
        COPY_MEMBER(mEase)
        COPY_MEMBER(mEasePower)
        COPY_MEMBER(mWrap)
        COPY_MEMBER(mImmediateRelease)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(FlowAnimate)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    if (d.rev < 3) {
        mAnim = mAnim.LoadFromMainOrDir(d);
    } else {
        mAnim.LoadFromMainOrDir(d);
    }
    d >> mBlend >> mWait >> mDelay;
    d >> (int &)mStopMode >> mFilterEnable >> (int &)mFilterRate >> mFilterStart;
    d >> mFilterEnd >> mFilterPeriod;
    d >> mFilterType >> mFilterScale;
    if (d.rev > 0) {
        d >> (int &)mEase >> mEasePower >> mWrap;
    }
    if (d.rev > 1) {
        d >> mImmediateRelease;
    }
END_LOADS

bool FlowAnimate::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    PushDrivenProperties();
    if (mAnim) {
        if (mImmediateRelease) {
            mSpawnedTask = nullptr;
            if (mFilterEnable) {
                if (mFilterPeriod) {
                    mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mFilterRate,
                        mFilterStart,
                        mFilterEnd,
                        mFilterPeriod,
                        1,
                        mFilterType,
                        this,
                        mEase,
                        mEasePower,
                        mWrap
                    );
                } else {
                    mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mFilterRate,
                        mFilterStart,
                        mFilterEnd,
                        0,
                        mFilterScale,
                        mFilterType,
                        this,
                        mEase,
                        mEasePower,
                        mWrap
                    );
                }
            } else {
                mAnim->Animate(mBlend, mWait, mDelay, this);
            }
        } else if (!FlowNode::IsRunning()) {
            TheFlowMgr->QueueCommand(this, kStarting);
            return true;
        }
    }
    return false;
}

void FlowAnimate::Deactivate(bool b1) {
    FLOW_LOG("Deactivate\n");
    if (mSpawnedTask) {
        mSpawnedTask->SetEventReciever(nullptr);
        AnimTask *task = mSpawnedTask;
        mSpawnedTask = nullptr;
        delete task;
    }
    TheFlowMgr->CancelCommand(this);
    FlowNode::Deactivate(b1);
}

void FlowAnimate::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty() && !mSpawnedTask && !mImmediateRelease) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowAnimate::RequestStop() {
    if (mSpawnedTask) {
        switch (mStopMode) {
        case kStopImmediate:
            TheFlowMgr->QueueCommand(this, kRequestStop);
            break;
        case kStopLastFrame:
            mStopOnLoop = true;
            break;
        case kStopBetweenMarkers:
            if (mPassedStopMarker) {
                TheFlowMgr->QueueCommand(this, kRequestStop);
            } else {
                mQueuedStop = kStopBetweenMarkers;
                mStopOnLoop = true;
            }
            break;
        case kStopOnMarker:
            mQueuedStop = kStopOnMarker;
            mStopOnLoop = true;
            break;
        case kReleaseAndContinue:
            TheFlowMgr->QueueCommand(this, kRequestStop);
            break;
        default:
            MILO_NOTIFY_ONCE("Bad Stop Mode value in animate case!");
            break;
        }
    }
    FlowNode::RequestStop();
}

void FlowAnimate::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    TheFlowMgr->QueueCommand(this, kStarting);
    if (mStopOnLoop) {
        mStopOnLoop = false;
    }
    FlowNode::RequestStopCancel();
}

void FlowAnimate::Execute(QueueState state) {
    FLOW_LOG("Execute: state = %i\n", (int)state);
    if (IsRunning()) {
        if (mSpawnedTask && state == kRequestStop) {
            mSpawnedTask->SetEventReciever(nullptr);
            if (mStopMode != 4) {
                delete mSpawnedTask;
            }
            mSpawnedTask = nullptr;
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else {
        if (state == kStarting) {
            mStopOnLoop = false;
            mQueuedStop = kStopImmediate;
            Task *task;
            if (mFilterEnable) {
                if (mFilterPeriod) {
                    task = mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mFilterRate,
                        mFilterStart,
                        mFilterEnd,
                        mFilterPeriod,
                        1,
                        mFilterType,
                        this,
                        mEase,
                        mEasePower,
                        mWrap
                    );
                } else {
                    task = mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mFilterRate,
                        mFilterStart,
                        mFilterEnd,
                        0,
                        mFilterScale,
                        mFilterType,
                        this,
                        mEase,
                        mEasePower,
                        mWrap
                    );
                }
            } else {
                task = mAnim->Animate(mBlend, mWait, mDelay, this);
            }
            mSpawnedTask = static_cast<AnimTask *>(task);
        } else if (state == kRequestStop) {
            mFlowParent->ChildFinished(this);
        }
    }
}

bool FlowAnimate::IsRunning() {
    if (!FlowNode::IsRunning()) {
        return mSpawnedTask;
    } else {
        return true;
    }
}

void FlowAnimate::ResetAnim() {
    if (mAnim && !FlowNode::sPushDrivenProperties) {
        mFilterRate = mAnim->GetRate();
        mFilterStart = mAnim->StartFrame();
        mFilterEnd = mAnim->EndFrame();
        mEase = kEaseLinear;
        mWrap = false;
        mFilterPeriod = 0;
        mFilterScale = 1;
        mEasePower = 2;
        static Symbol range("range");
        static Symbol loop("loop");
        mFilterType = mAnim->Loop() ? loop : range;
    }
}

void FlowAnimate::OnAnimEvent(Symbol s) {
    FLOW_LOG("Event: %s\n", s.Str());
    FOREACH (it, mChildNodes) {
        FlowNode *cur = *it;
        if (cur->ClassName() == FlowLabel::StaticClassName()) {
            FlowLabel *label = static_cast<FlowLabel *>((FlowNode *)*it);
            if (label->GetLabel() == s) {
                ActivateLabel(label);
                break;
            }
        }
    }
    static Symbol ended("ended");
    static Symbol stop("stop");
    static Symbol no_stop("no_stop");
    static Symbol interrupted("interrupted");
    static Symbol looped("looped");
    if (s == interrupted) {
        mSpawnedTask = nullptr;
        if (mRunningNodes.empty() && !mImmediateRelease) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    }
    if (s == ended) {
        if (mSpawnedTask) {
            mSpawnedTask->SetEventReciever(nullptr);
        }
        mSpawnedTask = nullptr;
        if (mRunningNodes.empty() && !mImmediateRelease) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else if (s == stop) {
        if (mQueuedStop == kStopOnMarker || mQueuedStop == kStopBetweenMarkers) {
            TheFlowMgr->QueueCommand(this, kRequestStop);
        }
        mQueuedStop = kStopImmediate;
        mPassedStopMarker = true;
    } else if (s == no_stop) {
        mQueuedStop = kStopImmediate;
        mPassedStopMarker = false;
    } else if (s == looped) {
        if (mStopOnLoop && mSpawnedTask) {
            mSpawnedTask->SetEventReciever(nullptr);
            delete mSpawnedTask;
            FLOW_TIMED_RELEASE_FROM_PARENT;
        } else {
            mPassedStopMarker = false;
        }
    }
}
