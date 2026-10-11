#include "gesture/GestureMgr.h"
#include "GestureMgr.h"
#include "SkeletonViz.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/CameraTilt.h"
#include "gesture/DrawUtl.h"
#include "IdentityInfo.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SpeechMgr.h"
#include "gesture/WaveToTurnOnLight.h"
#include "hamobj/Ham.h"
#include "hamobj/HamGameData.h"
#include "math/Trig.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Rnd.h"
#include "ui/UILabel.h"
#include "utl/Loader.h"
#include "xdk/NUI.h"

float GestureMgr::sMaxRecoveryDistance = 0.3;
float GestureMgr::sMinRecoveryTime = 0.7;
float GestureMgr::sMaxRecoveryTime = 1;
float GestureMgr::sConfidenceLossThreshold = 12;
float GestureMgr::sConfidenceRegainThreshold = 16;

GestureMgr *TheGestureMgr;
bool GestureMgr::sIdentityOpInProgress;

GestureMgr::GestureMgr()
    : mLiveCamInput(LiveCameraInput::sInstance), mPauseOnSkeletonLoss(kSkelLoss_Pause),
      mIdentificationEnabled(1), mInControllerMode(0), mInVoiceMode(0),
      mGesturingWithVoice(0), mInDoubleUserMode(0), mUIMode(0), mSkeletonDebugDisplay(0) {
    MILO_ASSERT(mLiveCamInput, 0x40);
    mPlayerSkeletonID[0] = -1;
    mPlayerSkeletonID[1] = -1;
    for (int i = 0; i < NUM_SKELETONS; i++) {
        mSkeletons[i].Init();
        mSkeletonQuality[i].Init(sConfidenceLossThreshold, sConfidenceRegainThreshold);
        mIdentityInfo[i].mSkeletonIdx = i;
        mSkeletonTrackingStatus[i] = 0;
    }
    mTrackAllSkeletons = false;
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.AddCallback(this);
    memset(&mTiltOverlapped, 0, sizeof(XOVERLAPPED));
}

GestureMgr::~GestureMgr() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.RemoveCallback(this);
    RELEASE(mSkeletonDebugDisplay);
}

BEGIN_HANDLERS(GestureMgr)
    HANDLE_EXPR(pause_on_skeleton_loss, mPauseOnSkeletonLoss)
    HANDLE_EXPR(toggle_pause_on_skeleton_loss, TogglePauseOnSkeletonLoss())
    HANDLE_EXPR(get_max_snapshots, mLiveCamInput->MaxSnapshots())
    HANDLE_ACTION(init_snapshots, mLiveCamInput->InitSnapshots(_msg->Int(2)))
    HANDLE_ACTION(clear_snapshots, mLiveCamInput->ClearSnapshots())
    HANDLE_EXPR(num_snapshots, mLiveCamInput->NumSnapshots())
    HANDLE_EXPR(snapshot_tex, SnapshotTex(_msg->Int(2)))
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
    HANDLE_EXPR(is_identification_enabled, mIdentificationEnabled)
    HANDLE_ACTION(set_identification_enabled, SetIdentificationEnabled(_msg->Int(2)))
    HANDLE_ACTION(auto_tilt, AutoTilt())
    HANDLE_EXPR(get_player_index_by_tracking_id, mPlayerSkeletonID[1] == _msg->Int(2))
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

void GestureMgr::PostUpdate(const SkeletonUpdateData *data) {
    if (data) {
        static bool sLmao; // not in the pdb! :)
        if (!UsingCD() && !data->mCamInput.IsConnected() && !sLmao) {
            sLmao = true;
            mPauseOnSkeletonLoss = kSkelLoss_NoPauseFlashcardsDontDropOut;
        }

        for (int i = 0; i < NUM_SKELETONS; i++) {
            bool b3 = true;
            mSkeletonTrackingStatus[i] = 0;
            if (mTrackAllSkeletons
                && data->mAllSkeletons[i]->TrackingID() == mSkeletons[i].TrackingID()) {
                b3 = data->mAllSkeletons[i]->TrackingState() != kSkeletonPositionOnly
                    ? b3
                    : false;
            }
            if (b3) {
                mSkeletons[i] = *data->mAllSkeletons[i];
            }
            mSkeletonQuality[i].Update(mSkeletons[i], mUIMode);
            mIdentityInfo[i].PostUpdate();
        }

        if (TheLoadMgr.EditMode()) {
            if (GetActiveSkeletonIndex() < 0) {
                for (int i = 0; i < NUM_SKELETONS; i++) {
                    Skeleton &skel = GetSkeleton(i);
                    if (skel.IsTracked()) {
                        mActiveTrackingID = skel.TrackingID();
                        break;
                    }
                }
            }
        }

        if (mTrackAllSkeletons) {
            int id0 = mPlayerSkeletonID[0];
            int id1 = mPlayerSkeletonID[1];

            int idx0 = TheGestureMgr->GetSkeletonIndexByTrackingID(id0);
            int idx1 = TheGestureMgr->GetSkeletonIndexByTrackingID(id1);

            int i10 = -1;
            int start0 = 0;
            int end0 = 5;
            if (idx0 != -1) {
                start0 = idx0 + 1;
                end0 = idx0 + 5;
            }
            for (int i = start0; i <= end0; i++) {
                int cur = GetSkeleton(i % 6).TrackingID();
                if (cur > 0 && cur != id1) {
                    i10 = id0;
                    id0 = cur;
                    break;
                }
            }

            int start1 = 0;
            int end1 = 5;
            if (idx1 != -1) {
                start1 = idx1 + 1;
                end1 = idx1 + 5;
            }
            for (int i = start1; i <= end1; i++) {
                int cur = GetSkeleton(i % 6).TrackingID();
                if (cur > 0 && cur != id0 && cur != i10) {
                    id1 = cur;
                    break;
                }
            }

            SetTrackedSkeletons(id0, id1);
        }

        mSkeletonRecoverer.Poll();
    }
}

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
        TheGestureMgr->mSkeletonDebugDisplay = dynamic_cast<RndDir *>(dir);
        if (!TheGestureMgr->mSkeletonDebugDisplay && dir) {
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
    if (TheSpeechMgr) {
        TheSpeechMgr->Poll();
    }
    TheCameraTilt->Poll();
    TheWaveToTurnOnLight->Poll();
}

IdentityInfo *GestureMgr::GetIdentityInfo(int idx) {
    if (idx >= 0 && idx < NUM_SKELETONS)
        return &mIdentityInfo[idx];
    else
        return nullptr;
}

int GestureMgr::GetSkeletonIndexByTrackingID(int id) const {
    if (id <= 0) {
        return -1;
    } else {
        for (int i = 0; i < NUM_SKELETONS; i++) {
            if (mSkeletons[i].TrackingID() == id)
                return i;
        }
        return -1;
    }
}

Skeleton *GestureMgr::GetSkeletonByTrackingID(int id) {
    if (id > 0) {
        for (int i = 0; i < NUM_SKELETONS; i++) {
            if (mSkeletons[i].TrackingID() == id
                && mSkeletons[i].TrackingState() != kSkeletonNotTracked)
                return &mSkeletons[i];
        }
    }
    return nullptr;
}

Skeleton *GestureMgr::GetSkeletonByEnrollmentIndex(int idx) {
    if (idx >= 0) {
        for (int i = 0; i < NUM_SKELETONS; i++) {
            if (mSkeletons[i].GetEnrollmentIndex() == idx) {
                return &mSkeletons[i];
            }
        }
    }
    return nullptr;
}

Skeleton *GestureMgr::GetActiveSkeleton() {
    return GetSkeletonByTrackingID(mActiveTrackingID);
}

Skeleton &GestureMgr::GetSkeleton(int idx) {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x99);
    return mSkeletons[idx];
}

const Skeleton &GestureMgr::GetSkeleton(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x9F);
    return mSkeletons[idx];
}

SkeletonQualityFilter &GestureMgr::GetSkeletonQualityFilter(int idx) {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x122);
    return mSkeletonQuality[idx];
}

int GestureMgr::GetActiveSkeletonIndex() const {
    return GetSkeletonIndexByTrackingID(mActiveTrackingID);
}

void GestureMgr::SetTrackedSkeletons(int i1, int i2) {
    mPlayerSkeletonID[0] = i1;
    mPlayerSkeletonID[1] = i2;
    UpdateTrackedSkeletons();
}

void GestureMgr::SetIdentificationEnabled(bool enabled) {
    if (enabled != mIdentificationEnabled) {
        if (!enabled) {
            NuiIdentityAbort();
        }
        mIdentificationEnabled = enabled;
    }
}

void GestureMgr::SetInControllerMode(bool mode) { mInControllerMode = mode; }
void GestureMgr::SetInVoiceMode(bool mode) { mInVoiceMode = mode; }
void GestureMgr::SetGesturingWithVoice(bool gesturing) {
    mGesturingWithVoice = gesturing;
}
void GestureMgr::SetInDoubleUserMode(bool mode) { mInDoubleUserMode = mode; }

void GestureMgr::StartTrackAllSkeletons() {
    mTrackAllSkeletons = true;
    for (int i = 0; i < NUM_SKELETONS; i++) {
        mSkeletonQuality[i].Init(10, 12);
        mSkeletonQuality[i].SetSidewaysCutoffThreshold(0.9);
    }
}

void GestureMgr::CancelTrackAllSkeletons() {
    mTrackAllSkeletons = false;
    for (int i = 0; i < NUM_SKELETONS; i++) {
        mSkeletonQuality[i].Init(sConfidenceLossThreshold, sConfidenceRegainThreshold);
        mSkeletonQuality[i].RestoreDefaultSidewaysCutoffThreshold();
    }
}

bool GestureMgr::IsTrackingAllSkeletons() const { return mTrackAllSkeletons; }

bool GestureMgr::IsSkeletonValid(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x128);
    return mSkeletonQuality[idx].IsSkeletonValid();
}

bool GestureMgr::IsSkeletonSitting(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x12E);
    return mSkeletonQuality[idx].IsSitting();
}

bool GestureMgr::IsSkeletonSideways(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, 6, 0x134);
    return mSkeletonQuality[idx].IsSideways();
}

void GestureMgr::UpdateTrackedSkeletons() {
    LiveCameraInput *cameraInput = mLiveCamInput;
    MILO_ASSERT(cameraInput, 0x14F);
    cameraInput->SetTrackedSkeletons(mPlayerSkeletonID[0], mPlayerSkeletonID[1]);
}

int GestureMgr::GetPlayerSkeletonID(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x2DA);
    return mPlayerSkeletonID[playerIndex];
}

void GestureMgr::SetPlayerSkeletonID(int playerIndex, int id) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x2E0);
    mPlayerSkeletonID[playerIndex] = id;
    UpdateTrackedSkeletons();
}

int GestureMgr::GetPlayerFilteredSkeletonID(int playerIndex, bool b2) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x2E8);
    int id = GetPlayerSkeletonID(playerIndex);
    if (id >= 0) {
        Skeleton *skeleton = GetSkeletonByTrackingID(id);
        if (!skeleton || (b2 && skeleton->IsSitting())) {
            id = -1;
        }
    }
    return id;
}

int GestureMgr::GetSecondarySkeletonIndex(bool b) const {
    for (int i = 0; i < NUM_SKELETONS; i++) {
        if (i != GetActiveSkeletonIndex() && GetSkeleton(i).IsTracked()) {
            if (!b || GetSkeleton(i).IsValid()) {
                return i;
            }
        }
    }
    return -1;
}

void GestureMgr::DrawSkeletonKinectData() {
    Vector2 v2(0.15f, 0.2);
    for (int i = 0; i < NUM_SKELETONS; i++) {
        Skeleton &skeleton = GetSkeleton(i);
        SkeletonTracking tracking = skeleton.TrackingState();
        if (!mSkeletonDebugDisplay) {
            const char *state;
            if (tracking == kSkeletonTracked) {
                state = "tracked";
            } else if (tracking == kSkeletonPositionOnly) {
                state = "position only";
            } else {
                state = "not tracked";
            }
            int trackingID = skeleton.TrackingID();
            const char *c138 = i == GetActiveSkeletonIndex() ? "(active)" : "";
            v2.y = TheRnd
                       .DrawString(
                           MakeString("%i: %s %s %d", i, state, c138, trackingID),
                           v2,
                           Hmx::Color(1, 1, 1),
                           true
                       )
                       .y;
        } else {
            RndDir *marker =
                mSkeletonDebugDisplay->Find<RndDir>(MakeString("marker%d", i), false);
            if (marker) {
                marker->SetShowing(tracking != kSkeletonNotTracked);
                if (tracking != kSkeletonNotTracked) {
                    marker->SetLocalPos(
                        Vector3(skeleton.Center().x, -skeleton.Center().z, 0)
                    );
                    UILabel *lbl = marker->Find<UILabel>("id.lbl", false);
                    if (lbl) {
                        lbl->SetInt(skeleton.TrackingID(), false);
                    }
                    lbl = marker->Find<UILabel>("index.lbl", false);
                    if (lbl) {
                        lbl->SetInt(i, false);
                    }
                    RndAnimatable *anim =
                        marker->Find<RndAnimatable>("active.anim", false);
                    if (anim) {
                        anim->SetFrame(i == GetActiveSkeletonIndex() ? 1.0f : 0.0f, 1);
                    }
                    RndAnimatable *tracked =
                        marker->Find<RndAnimatable>("tracked.anim", false);
                    if (tracked) {
                        tracked->SetFrame((float)(tracking == kSkeletonTracked), 1);
                    }
                    RndAnimatable *color =
                        marker->Find<RndAnimatable>("player_color.anim", false);
                    if (color) {
                        int idx = -1;
                        for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
                            if (skeleton.TrackingID()
                                == TheGameData->Player(i)->GetSkeletonTrackingID()) {
                                idx = i;
                            }
                        }
                        color->SetFrame(idx, 1);
                    }
                    float f12 = 0;
                    if (tracking == kSkeletonTracked) {
                        Vector3 left;
                        skeleton.JointPos(kCoordCamera, kJointShoulderLeft, left);
                        Vector3 right;
                        skeleton.JointPos(kCoordCamera, kJointShoulderRight, right);
                        Subtract(right, left, right);
                        f12 = atanf(right.z / right.x);
                        f12 *= RAD2DEG;
                    }
                    RndAnimatable *rot =
                        marker->Find<RndAnimatable>("rotation.anim", false);
                    if (rot) {
                        rot->SetFrame(f12, 1);
                    }
                }
            }
        }
    }
    if (mSkeletonDebugDisplay) {
        mSkeletonDebugDisplay->DrawShowing();
    }
}

DataNode GestureMgr::OnMsg(const KinectHardwareStatusMsg &msg) {
    if (msg.HardwareStatus() == KinectHardwareStatusMsg::kKinectReady) {
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
