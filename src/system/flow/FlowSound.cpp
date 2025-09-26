#include "flow/FlowSound.h"
#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

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
