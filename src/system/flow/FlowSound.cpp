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
    : mImmediateRelease(true), mStopMode(kStopLastFrame), mPassedStopMarker(false),
      mQueuedStop(kStopImmediate), mStopOnLoop(false), mSound(this), mVolume(0),
      mTranspose(0), mPan(0), mPlaying(false), mForceStop(false), mUseIntensity(true),
      mIntensity(1) {}

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
        mIntensity = FlowNode::sIntensity;
        if (mImmediateRelease && !mForceStop) {
            mPlaying = false;
            float db = RatioToDb(mIntensity) + mVolume;
            mSound->Play(db, mPan, mTranspose, nullptr, 0);
            return false;
        } else if (mForceStop) {
            mPlaying = false;
            mSound->Stop(nullptr, false);
            return false;
        } else {
            TheFlowMgr->QueueCommand(this, kStarting);
            return true;
        }
    }
}

void FlowSound::Deactivate(bool b1) {
    FLOW_LOG("Deactivated\n");
    mPlaying = false;
    if (mSound) {
        mSound->Stop(this, false);
    }
    FlowNode::Deactivate(b1);
}

void FlowSound::ChildFinished(FlowNode *child) {
    FLOW_LOG("Child Finished of class:%s\n", child->ClassName());
    mRunningNodes.remove(child);
    if (!mPlaying) {
        FLOW_TIMED_RELEASE_FROM_PARENT;
    }
}

void FlowSound::RequestStop() {
    FLOW_LOG("RequestStop\n");
    switch (mStopMode) {
    case FlowNode::kStopImmediate:
    case FlowNode::kReleaseAndContinue:
        mStopOnLoop = true;
        TheFlowMgr->QueueCommand(this, kRequestStop);
        break;
    case FlowNode::kStopLastFrame:
        mStopOnLoop = true;
        break;
    case FlowNode::kStopBetweenMarkers:
        if (mPassedStopMarker) {
            TheFlowMgr->QueueCommand(this, kRequestStop);
        } else {
            mQueuedStop = kStopBetweenMarkers;
            mStopOnLoop = true;
        }
        break;
    case FlowNode::kStopOnMarker:
        mQueuedStop = kStopOnMarker;
        mStopOnLoop = true;
        break;
    default:
        break;
    }
    FlowNode::RequestStop();
}

void FlowSound::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    FlowNode::RequestStopCancel();
    if (mStopOnLoop) {
        mStopOnLoop = false;
        TheFlowMgr->QueueCommand(this, kStarting);
    }
}

void FlowSound::Execute(QueueState qs) {
    FLOW_LOG("Execute: state = %i\n", (int)qs);
    if (IsRunning()) {
        if (qs == kRequestStop) {
            mPlaying = false;
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
        if (qs == kStarting) {
            mStopOnLoop = false;
            mQueuedStop = kStopImmediate;
            if (!mPlaying) {
                mPlaying = true;
                float db = RatioToDb(mIntensity) + mVolume;
                mSound->Play(db, mPan, mTranspose, this, 0);
            }
        } else if (qs == kRequestStop) {
            mFlowParent->ChildFinished(this);
        }
    }
}

bool FlowSound::IsRunning() { return mPlaying || FlowNode::IsRunning(); }

void FlowSound::UpdateIntensity() {
    FLOW_LOG("Updating Intensity: %0.2f\n", FlowNode::sIntensity);
    if (mUseIntensity) {
        float db = RatioToDb(FlowNode::sIntensity);
        mIntensity = FlowNode::sIntensity;
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
            if (label->GetLabel() == event) {
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
    if (mPlaying && (event == ended || event == interrupted)) {
        mPlaying = false;
        if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    } else if (event == looped) {
        if (!mStopOnLoop || !mPlaying) {
            mPassedStopMarker = false;
        } else {
            mSound->Stop(this, false);
            if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
                FLOW_TIMED_RELEASE_FROM_PARENT;
            }
        }
    } else if (event == stop) {
        if (mQueuedStop == kStopOnMarker || mQueuedStop == kStopBetweenMarkers) {
            TheFlowMgr->QueueCommand(this, kRequestStop);
        }
        mQueuedStop = kStopImmediate;
        mPassedStopMarker = true;
    } else if (event == no_stop) {
        mPassedStopMarker = false;
        mQueuedStop = kStopImmediate;
    } else if (event == release) {
        mPlaying = false;
        mSound->EndLoop(this);
        if (mRunningNodes.empty() && mFlowParent->HasRunningNode(this)) {
            FLOW_TIMED_RELEASE_FROM_PARENT;
        }
    }
}
