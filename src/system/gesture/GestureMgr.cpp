#include "gesture/GestureMgr.h"
#include "GestureMgr.h"
#include "SkeletonViz.h"
#include "gesture/CameraTilt.h"
#include "gesture/DrawUtl.h"
#include "IdentityInfo.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SpeechMgr.h"
#include "gesture/WaveToTurnOnLight.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "xdk/NUI.h"
#include "xdk/nuiapi/nuidiagnostics.h"

float GestureMgr::sMaxRecoveryDistance = 0.3;
float GestureMgr::sMinRecoveryTime = 0.7;
float GestureMgr::sMaxRecoveryTime = 1;
float GestureMgr::sConfidenceLossThreshold = 12;
float GestureMgr::sConfidenceRegainThreshold = 16;

GestureMgr *TheGestureMgr;
bool GestureMgr::sIdentityOpInProgress;

GestureMgr::GestureMgr()
    : mLiveCamInput(LiveCameraInput::sInstance), unk425c(2), mIDEnabled(1),
      mInControllerMode(0), mInVoiceMode(0), mGesturingWithVoice(0), mInDoubleUserMode(0),
      unk4271(0), unk4274(0) {
    MILO_ASSERT(mLiveCamInput, 0x40);
    mPlayerSkeletonIDs[0] = -1;
    mPlayerSkeletonIDs[1] = -1;
    for (int i = 0; i < 6; i++) {
        mSkeletons[i].Init();
        mFilters[i].Init(sConfidenceLossThreshold, sConfidenceRegainThreshold);
        mIdentityInfos[i].Init();
        mCallbacks[i] = nullptr;
    }
    mTrackingAllSkeletons = false;
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.AddCallback(this);
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
}

GestureMgr::~GestureMgr() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.RemoveCallback(this);
    RELEASE(unk4274);
}

BEGIN_HANDLERS(GestureMgr)
    HANDLE_EXPR(pause_on_skeleton_loss, unk425c)
    HANDLE_EXPR(toggle_pause_on_skeleton_loss, TogglePauseOnSkeletonLoss())
    HANDLE_EXPR(get_max_snapshots, mLiveCamInput->MaxSnapshots())
    HANDLE_ACTION(init_snapshots, mLiveCamInput->InitSnapshots(_msg->Int(2)))
    HANDLE_ACTION(clear_snapshots, mLiveCamInput->ClearSnapshots())
    HANDLE_EXPR(num_snapshots, mLiveCamInput->NumSnapshots())
    HANDLE_EXPR(snapshot_tex, GetSnapshotTex(_msg->Int(2)))
    HANDLE_ACTION(start_snapshot_batch, mLiveCamInput->StartSnapshotBatch())
    HANDLE_EXPR(num_snapshot_batches, mLiveCamInput->NumSnapshotBatches())
    HANDLE_EXPR(
        get_snapshot_batch_index,
        mLiveCamInput->GetSnapshotBatchStartingIndex(_msg->Int(2))
    )
    HANDLE_EXPR(
        toggle_autoexposure_tweak,
        mLiveCamInput->SetTweakedAutoexposure(!mLiveCamInput->GetTweakedAutoexposure())
    )
    HANDLE_EXPR(using_autoexposure_tweak, mLiveCamInput->GetTweakedAutoexposure())
    HANDLE_ACTION(
        set_autoexposure_region,
        mLiveCamInput->SetExposureRegion(
            _msg->Float(2), _msg->Float(3), _msg->Float(4), _msg->Float(5)
        )
    )
    HANDLE_ACTION(set_autoexposure, mLiveCamInput->SetAutoexposure(_msg->Int(2)))
    HANDLE_EXPR(
        toggle_autoexposure,
        mLiveCamInput->SetAutoexposure(!mLiveCamInput->GetAutoexposure())
    )
    HANDLE_EXPR(is_identification_enabled, mIDEnabled)
    HANDLE_ACTION(set_identification_enabled, SetIdentificationEnabled(_msg->Int(2)))
    HANDLE_ACTION(auto_tilt, AutoTilt())
    HANDLE_EXPR(get_player_index_by_tracking_id, mPlayerSkeletonIDs[1] == _msg->Int(2))
    HANDLE_EXPR(
        get_skeleton_index_by_tracking_id, GetSkeletonIndexByTrackingID(_msg->Int(2))
    )
    HANDLE_EXPR(get_tracking_id_by_skeleton_index, GetSkeleton(_msg->Int(2)).TrackingID())
    HANDLE_EXPR(is_tracked_skeleton_index, GetSkeleton(_msg->Int(2)).IsTracked())
    HANDLE_ACTION(set_tracked_skeletons, SetTrackedSkeletons(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(draw_skeletons, TheSkeletonViz->SetShowing(_msg->Int(2)))
    HANDLE_ACTION(dump_camera_properties, mLiveCamInput->DumpProperties())
    HANDLE_EXPR(get_max_recovery_dist, sMaxRecoveryDistance)
    HANDLE_ACTION(set_max_recovery_dist, sMaxRecoveryDistance = _msg->Float(2))
    HANDLE_EXPR(get_max_recovery_time, sMaxRecoveryTime)
    HANDLE_ACTION(set_max_recovery_time, sMaxRecoveryTime = _msg->Float(2))
    HANDLE_EXPR(get_min_recovery_time, sMinRecoveryTime)
    HANDLE_ACTION(set_min_recovery_time, sMinRecoveryTime = _msg->Float(2))
    HANDLE_ACTION(set_in_voice_mode, SetInVoiceMode(_msg->Int(2)))
    HANDLE_ACTION(set_gesturing_with_voice, SetGesturingWithVoice(_msg->Int(2)))
    HANDLE_ACTION(show_gesture_guide, ShowGestureGuide())
    HANDLE_ACTION(show_gesture_troubleshooter, XShowNuiTroubleshooterUI())
    HANDLE_MESSAGE(KinectHardwareStatusMsg)
    HANDLE_MESSAGE(KinectUserBindingChangedMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void GestureMgr::Init() {
    LiveCameraInput::PreInit();
    TheGestureMgr = new GestureMgr();
    TheGestureMgr->SetName("gesture_mgr", ObjectDir::Main());
    InitDrawUtl(*TheGestureMgr);
    ThePlatformMgr.AddSink(TheGestureMgr, "kinect_status_changed");
    ThePlatformMgr.AddSink(TheGestureMgr, "kinect_user_binding_changed");
    TheDebug.AddExitCallback(GestureMgr::Terminate);
}

void GestureMgr::DebugInit() {
    const char *debugStr = nullptr;
    if (SystemConfig("kinect")->FindData("gesture_debug", debugStr, false) && debugStr) {
        ObjectDir *dir = DirLoader::LoadObjects(debugStr, nullptr, nullptr);
        TheGestureMgr->unk4274 = dynamic_cast<RndDir *>(dir);
        if (!TheGestureMgr->unk4274 && dir) {
            delete dir;
        }
    }
}

void GestureMgr::Terminate() {
    TerminateDrawUtl();
    ThePlatformMgr.RemoveSink(TheGestureMgr);
    RELEASE(TheGestureMgr);
}

void GestureMgr::Poll() {
    if (TheSpeechMgr)
        TheSpeechMgr->Poll();
    TheCameraTilt->Poll();
    TheWaveToTurnOnLight->Poll();
}

IdentityInfo *GestureMgr::GetIdentityInfo(int idx) {
    if (idx >= 0 && idx < 6)
        return &mIdentityInfos[idx];
    else
        return nullptr;
}

int GestureMgr::GetSkeletonIndexByTrackingID(int id) const {
    if (id > 0) {
        for (int i = 0; i < 6; i++) {
            if (mSkeletons[i].TrackingID() == id)
                return i;
        }
    }
    return -1;
}

Skeleton *GestureMgr::GetSkeletonByTrackingID(int id) {
    if (id > 0) {
        for (int i = 0; i < 6; i++) {
            if (mSkeletons[i].TrackingID() == id
                && mSkeletons[i].TrackingState() != kSkeletonNotTracked)
                return &mSkeletons[i];
        }
    }
    return nullptr;
}

Skeleton *GestureMgr::GetSkeletonByEnrollmentIndex(int idx) {
    if (idx >= 0) {
        for (int i = 0; i < 6; i++) {
            if (mSkeletons[i].GetEnrollmentIndex() == idx) {
                return &mSkeletons[i];
            }
        }
    }
    return nullptr;
}

Skeleton *GestureMgr::GetActiveSkeleton() { return GetSkeletonByTrackingID(unk4260); }

Skeleton &GestureMgr::GetSkeleton(int idx) {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x99);
    return mSkeletons[idx];
}

const Skeleton &GestureMgr::GetSkeleton(int idx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x9F);
    return mSkeletons[idx];
}

SkeletonQualityFilter &GestureMgr::GetSkeletonQualityFilter(int idx) {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x122);
    return mFilters[idx];
}

int GestureMgr::GetActiveSkeletonIndex() const {
    if (unk4260 > 0) {
        for (int i = 0; i < 6; i++) {
            if (mSkeletons[i].TrackingID() == unk4260) {
                return i;
            }
        }
    }
    return -1;
}

void GestureMgr::SetTrackedSkeletons(int i1, int i2) {
    mPlayerSkeletonIDs[0] = i1;
    mPlayerSkeletonIDs[1] = i2;
    UpdateTrackedSkeletons();
}

void GestureMgr::SetIdentificationEnabled(bool enabled) {
    if (enabled != mIDEnabled) {
        if (!enabled) {
            NuiIdentityAbort();
        }
        mIDEnabled = enabled;
    }
}

void GestureMgr::SetInControllerMode(bool mode) { mInControllerMode = mode; }
void GestureMgr::SetInVoiceMode(bool mode) { mInVoiceMode = mode; }
void GestureMgr::SetGesturingWithVoice(bool gesturing) {
    mGesturingWithVoice = gesturing;
}
void GestureMgr::SetInDoubleUserMode(bool mode) { mInDoubleUserMode = mode; }

void GestureMgr::StartTrackAllSkeletons() {
    mTrackingAllSkeletons = true;
    for (int i = 0; i < 6; i++) {
        mFilters[i].Init(10, 12);
        mFilters[i].SetSidewaysCutoffThreshold(0.9);
    }
}

void GestureMgr::CancelTrackAllSkeletons() {
    mTrackingAllSkeletons = false;
    for (int i = 0; i < 6; i++) {
        mFilters[i].Init(sConfidenceLossThreshold, sConfidenceRegainThreshold);
        mFilters[i].RestoreDefaultSidewaysCutoffThreshold();
    }
}

bool GestureMgr::IsTrackingAllSkeletons() const { return mTrackingAllSkeletons; }

bool GestureMgr::IsSkeletonValid(int idx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x128);
    return mFilters[idx].Valid();
}

bool GestureMgr::IsSkeletonSitting(int idx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x12E);
    return mFilters[idx].Sitting();
}

bool GestureMgr::IsSkeletonSideways(int idx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (6), 0x134);
    return mFilters[idx].Sideways();
}

void GestureMgr::UpdateTrackedSkeletons() {
    LiveCameraInput *cameraInput = mLiveCamInput;
    MILO_ASSERT(cameraInput, 0x14F);
    cameraInput->SetTrackedSkeletons(mPlayerSkeletonIDs[0], mPlayerSkeletonIDs[1]);
}

int GestureMgr::GetPlayerSkeletonID(int playerIndex) {
    MILO_ASSERT((0) <= (playerIndex) && (playerIndex) < (2), 0x2DA);
    return mPlayerSkeletonIDs[playerIndex];
}

void GestureMgr::SetPlayerSkeletonID(int playerIndex, int id) {
    MILO_ASSERT((0) <= (playerIndex) && (playerIndex) < (2), 0x2E0);
    mPlayerSkeletonIDs[playerIndex] = id;
    UpdateTrackedSkeletons();
}

int GestureMgr::GetPlayerFilteredSkeletonID(int playerIndex, bool b2) {
    MILO_ASSERT((0) <= (playerIndex) && (playerIndex) < (2), 0x2E8);
    int id = GetPlayerSkeletonID(playerIndex);
    if (id >= 0) {
        Skeleton *skeleton = GetSkeletonByTrackingID(id);
        if (!skeleton || (b2 && skeleton->IsSitting())) {
            id = -1;
        }
    }
    return id;
}

DataNode GestureMgr::OnMsg(const KinectHardwareStatusMsg &msg) {
    if (msg->Int(2) == 1) {
        MILO_ASSERT(mLiveCamInput, 0x21B);
        mLiveCamInput->SetAutoexposure(true);
    }
    return 1;
}

DataNode GestureMgr::OnMsg(const KinectUserBindingChangedMsg &msg) {
    static SkeletonEnrollmentChangedMsg enrollmentChangedMsg;
    Export(enrollmentChangedMsg, true);
    return 0;
}
