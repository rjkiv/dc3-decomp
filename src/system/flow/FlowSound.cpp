#include "flow/FlowSound.h"
#include "FlowNode.h"
#include "flow/FlowLabel.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "math/Decibels.h"
#include "obj/Object.h"
#include "os/Timer.h"
#include "synth/Sound.h"

FlowSound::FlowSound()
    : mImmediateRelease(true), mStopMode(kStopLastFrame), unk64(false), unk68(0),
      unk6c(false), mSound(this), mVolume(0), mTranspose(0), mPan(0), unk9c(false),
      mForceStop(false), mUseIntensity(true), unka0(1) {}

FlowSound::~FlowSound() { TheFlowMgr->CancelCommand(this); }

BEGIN_HANDLERS(FlowSound)
    HANDLE_ACTION(on_flow_finished, ChildFinished(_msg->Obj<FlowNode>(2)))
    HANDLE_ACTION(on_marker_event, OnMarkerEvent(_msg->Sym(2)))
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowSound)
    SYNC_PROP(immediate_release, mImmediateRelease)
    SYNC_PROP(volume, mVolume)
    SYNC_PROP(pan, mPan)
    SYNC_PROP(transpose, mTranspose)
    SYNC_PROP_MODIFY(sound, mSound, OnSoundSelected())
    SYNC_PROP(stop_mode, (int &)mStopMode)
    SYNC_PROP(force_stop, mForceStop)
    SYNC_PROP(use_intensity, mUseIntensity)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowSound)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mImmediateRelease;
    bs << mSound;
    bs << mVolume;
    bs << mPan;
    bs << mTranspose;
    bs << mStopMode;
    bs << mForceStop;
    bs << mUseIntensity;
END_SAVES

INIT_REVS(3, 0)

BEGIN_LOADS(FlowSound)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> mImmediateRelease;
    if (d.rev < 2) {
        mSound = mSound.LoadFromMainOrDir(bs);
    } else {
        mSound.LoadFromMainOrDir(bs);
    }
    d >> mVolume >> mPan >> mTranspose >> (int &)mStopMode;
    if (d.rev > 0)
        d >> mForceStop;
    if (d.rev > 2)
        d >> mUseIntensity;
END_LOADS

BEGIN_COPYS(FlowSound)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY_AS(FlowSound, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mImmediateRelease)
        COPY_MEMBER(mSound)
        COPY_MEMBER(mVolume)
        COPY_MEMBER(mPan)
        COPY_MEMBER(mTranspose)
        COPY_MEMBER(mStopMode)
        COPY_MEMBER(mForceStop)
        COPY_MEMBER(mUseIntensity)
    END_COPYING_MEMBERS
END_COPYS

bool FlowSound::Activate() {
    FLOW_LOG("Activate\n");
    if (!mSound)
        return false;
    else {
        PushDrivenProperties();
        unka0 = FlowNode::sIntensity;
        if (mImmediateRelease && !mForceStop) {
            unk9c = false;
            float db = RatioToDb(unka0) + mVolume;
            mSound->Play(db, mPan, mTranspose, nullptr, 0);
            return false;
        } else if (mForceStop) {
            unk9c = false;
            mSound->Stop(nullptr, false);
            return false;
        } else {
            TheFlowMgr->QueueCommand(this, kQueue);
            return true;
        }
    }
}

void FlowSound::Deactivate(bool b1) {
    FLOW_LOG("Deactivated\n");
    unk9c = false;
    if (mSound) {
        mSound->Stop(this, false);
    }
    FlowNode::Deactivate(b1);
}

void FlowSound::ChildFinished(FlowNode *child) {
    FLOW_LOG("Child Finished of class:%s\n", child->ClassName());
    mRunningNodes.remove(child);
    if (!unk9c) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowSound::RequestStop() {
    FLOW_LOG("RequestStop\n");
    switch (mStopMode) {
    case FlowNode::kStopImmediate:
    case FlowNode::kReleaseAndContinue:
        unk6c = true;
        TheFlowMgr->QueueCommand(this, kIgnore);
        break;
    case FlowNode::kStopLastFrame:
        unk6c = true;
        break;
    case FlowNode::kStopBetweenMarkers:
        if (unk64) {
            TheFlowMgr->QueueCommand(this, kIgnore);
        } else {
            unk68 = 3;
            unk6c = true;
        }
        break;
    case FlowNode::kStopOnMarker:
        unk68 = 2;
        unk6c = true;
        break;
    default:
        break;
    }
    FlowNode::RequestStop();
}

void FlowSound::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
    if (unk6c) {
        unk6c = false;
        TheFlowMgr->QueueCommand(this, kQueue);
    }
}

void FlowSound::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (IsRunning()) {
        if (qs == kIgnore) {
            unk9c = false;
            if (mStopMode == kReleaseAndContinue) {
                mSound->EndLoop(this);
            } else {
                mSound->Stop(this, true);
            }
            if (!mImmediateRelease) {
                FLOW_TIMED_RELEASE_FROM_PARENT;
            }
            FlowNode::Deactivate(false);
        }
    } else {
        if (qs == kQueue) {
            unk6c = false;
            unk68 = 0;
            if (!unk9c) {
                unk9c = true;
                float db = RatioToDb(unka0) + mVolume;
                mSound->Play(db, mPan, mTranspose, this, 0);
            }
        } else if (qs == kIgnore) {
            mFlowParent->ChildFinished(this);
        }
    }
}

bool FlowSound::IsRunning() { return unk9c || FlowNode::IsRunning(); }

void FlowSound::UpdateIntensity() {
    FLOW_LOG("Updating Intensity: %0.2f\n", FlowNode::sIntensity);
    if (mUseIntensity) {
        float db = RatioToDb(FlowNode::sIntensity);
        unka0 = FlowNode::sIntensity;
        db += mVolume;
        mSound->SetVolume(db, this);
    }
    FlowNode::UpdateIntensity();
}

void FlowSound::OnSoundSelected() {
    if (mSound) {
        if (mSound->Property("loop", true)->Int() == 1) {
            mImmediateRelease = false;
        }
    }
}

void FlowSound::OnMarkerEvent(Symbol event) {
    FLOW_LOG("Event: %s\n", event.Str());
    FOREACH (it, mChildNodes) {
        if ((*it)->ClassName() == FlowLabel::StaticClassName()) {
            FlowLabel *label = static_cast<FlowLabel *>((FlowNode *)*it);
            if (label->Label() == event) {
                ActivateLabel(label);
                break;
            }
        }
    }
    static Symbol ended("ended");
    static Symbol stop("stop");
    static Symbol no_stop("no_stop");
    static Symbol looped("looped");
    static Symbol interrupted("interrupted");
    static Symbol release("release");
    if (unk9c && (event == ended || event == interrupted)) {
        unk9c = false;
        if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else if (event == looped) {
        if (!unk6c || !unk9c) {
            unk64 = false;
        } else {
            mSound->Stop(this, false);
            if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
                FLOW_TIMED_RELEASE_FROM_PARENT;
            }
        }
    } else if (event == stop) {
        if (unk68 == 2 || unk68 == 3) {
            TheFlowMgr->QueueCommand(this, kIgnore);
        }
        unk68 = 0;
        unk64 = true;
    } else if (event == no_stop) {
        unk64 = false;
        unk68 = 0;
    } else if (event == release) {
        unk9c = false;
        mSound->EndLoop(this);
        if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    }
}
