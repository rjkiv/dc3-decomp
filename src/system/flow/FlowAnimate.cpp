#include "flow/FlowAnimate.h"
#include "flow/FlowLabel.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"

FlowAnimate::FlowAnimate()
    : unk5c(this), mAnim(this), mStopMode(kStopLastFrame), unk98(0), mBlend(0), mWait(0),
      mDelay(0), mEnable(0), mRate(RndAnimatable::k30_fps), mStart(0), mEnd(0),
      mPeriod(0), mScale(1), unkc4(0), mEase(kEaseLinear), mEasePower(2), mWrap(0),
      mImmediateRelease(0) {
    static Symbol range("range");
    mType = range;
}

FlowAnimate::~FlowAnimate() { TheFlowMgr->CancelCommand(this); }

bool FlowAnimate::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &unk5c) {
        if (unk5c && unk5c->Listener() == this) {
            OnAnimEvent("interrupted");
        }
        unk5c = nullptr;
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
    SYNC_PROP(enable, mEnable)
    SYNC_PROP(rate, (int &)mRate)
    SYNC_PROP(start, mStart)
    SYNC_PROP(end, mEnd)
    SYNC_PROP(scale, mScale)
    SYNC_PROP(period, mPeriod)
    SYNC_PROP(type, mType)
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
    bs << mStopMode << mEnable;
    bs << mRate << mStart;
    bs << mEnd << mPeriod;
    bs << mType;
    bs << mScale << mEase << mEasePower;
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
        COPY_MEMBER(mEnable)
        COPY_MEMBER(mRate)
        COPY_MEMBER(mStart)
        COPY_MEMBER(mEnd)
        COPY_MEMBER(mPeriod)
        COPY_MEMBER(mType)
        COPY_MEMBER(mScale)
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
        mAnim = mAnim.LoadFromMainOrDir(d.stream);
    } else {
        mAnim.LoadFromMainOrDir(d.stream);
    }
    d >> mBlend >> mWait >> mDelay;
    d >> (int &)mStopMode >> mEnable >> (int &)mRate >> mStart;
    d >> mEnd >> mPeriod;
    d >> mType >> mScale;
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
            unk5c = nullptr;
            if (mEnable) {
                if (mPeriod) {
                    mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mRate,
                        mStart,
                        mEnd,
                        mPeriod,
                        1,
                        mType,
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
                        mRate,
                        mStart,
                        mEnd,
                        0,
                        mScale,
                        mType,
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
            TheFlowMgr->QueueCommand(this, kQueue);
            return true;
        }
    }
    return false;
}

void FlowAnimate::Deactivate(bool b1) {
    FLOW_LOG("Deactivate\n");
    if (unk5c) {
        unk5c->SetListener(nullptr);
        AnimTask *task = unk5c;
        unk5c = nullptr;
        delete task;
    }
    TheFlowMgr->CancelCommand(this);
    FlowNode::Deactivate(b1);
}

void FlowAnimate::ChildFinished(FlowNode *n) {
    FLOW_LOG("Child Finished of class:%s\n", n->ClassName());
    mRunningNodes.remove(n);
    if (mRunningNodes.empty() && !unk5c && !mImmediateRelease) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowAnimate::RequestStop() {
    if (unk5c) {
        switch (mStopMode) {
        case kStopImmediate:
            TheFlowMgr->QueueCommand(this, kIgnore);
            break;
        case kStopLastFrame:
            unkc4 = true;
            break;
        case kStopBetweenMarkers:
            if (unk94) {
                TheFlowMgr->QueueCommand(this, kIgnore);
            } else {
                unk98 = 3;
                unkc4 = true;
            }
            break;
        case kStopOnMarker:
            unk98 = 2;
            unkc4 = true;
            break;
        case kReleaseAndContinue:
            TheFlowMgr->QueueCommand(this, kIgnore);
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
    TheFlowMgr->QueueCommand(this, kQueue);
    if (unkc4) {
        unkc4 = false;
    }
    FlowNode::RequestStopCancel();
}

void FlowAnimate::Execute(QueueState state) {
    FLOW_LOG("Execute: state = %i\n", (int)state);
    if (IsRunning()) {
        if (unk5c && state == kIgnore) {
            unk5c->SetListener(nullptr);
            if (mStopMode != 4) {
                delete unk5c;
            }
            unk5c = nullptr;
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else {
        if (state == kQueue) {
            unkc4 = false;
            unk98 = 0;
            Task *task;
            if (mEnable) {
                if (mPeriod) {
                    task = mAnim->Animate(
                        mBlend,
                        mWait,
                        mDelay,
                        mRate,
                        mStart,
                        mEnd,
                        mPeriod,
                        1,
                        mType,
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
                        mRate,
                        mStart,
                        mEnd,
                        0,
                        mScale,
                        mType,
                        this,
                        mEase,
                        mEasePower,
                        mWrap
                    );
                }
            } else {
                task = mAnim->Animate(mBlend, mWait, mDelay, this);
            }
            unk5c = static_cast<AnimTask *>(task);
        } else if (state == kIgnore) {
            mFlowParent->ChildFinished(this);
        }
    }
}

bool FlowAnimate::IsRunning() {
    if (!FlowNode::IsRunning()) {
        return unk5c;
    } else {
        return true;
    }
}

void FlowAnimate::ResetAnim() {
    if (mAnim && !FlowNode::sPushDrivenProperties) {
        mRate = mAnim->GetRate();
        mStart = mAnim->StartFrame();
        mEnd = mAnim->EndFrame();
        mEase = kEaseLinear;
        mWrap = false;
        mPeriod = 0;
        mScale = 1;
        mEasePower = 2;
        static Symbol range("range");
        static Symbol loop("loop");
        mType = mAnim->Loop() ? loop : range;
    }
}

void FlowAnimate::OnAnimEvent(Symbol s) {
    FLOW_LOG("Event: %s\n", s.Str());
    FOREACH (it, mChildNodes) {
        FlowNode *cur = *it;
        if (cur->ClassName() == FlowLabel::StaticClassName()) {
            FlowLabel *label = static_cast<FlowLabel *>((FlowNode *)*it);
            if (label->Label() == s) {
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
        unk5c = nullptr;
        if (mRunningNodes.empty() && !mImmediateRelease) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    }
    if (s == ended) {
        if (unk5c) {
            unk5c->SetListener(nullptr);
        }
        unk5c = nullptr;
        if (mRunningNodes.empty() && !mImmediateRelease) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else if (s == stop) {
        if (unk98 == 2 || unk98 == 3) {
            TheFlowMgr->QueueCommand(this, kIgnore);
        }
        unk98 = 0;
        unk94 = true;
    } else if (s == no_stop) {
        unk98 = 0;
        unk94 = false;
    } else if (s == looped) {
        if (unkc4 && unk5c) {
            unk5c->SetListener(nullptr);
            delete unk5c;
            FLOW_TIMED_RELEASE_FROM_PARENT;
        } else {
            unk94 = false;
        }
    }
}
