#include "hamobj/MoveDir.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "utl/Loader.h"

std::vector<FilterVersion *> MoveDir::sFilterVersions;

MoveDir::MoveDir()
    : mShowMoveOverlay(0), mErrorNodeInfo(0), mPlayClip(this), mRecordClip(this),
      unk2bc(this), unk2d0(this), unk2e4(0), mReportMove(this), mFiltersEnabled(0),
      unk308(0), unk30c(0), mFilterQueue(0), unk390(0), unk394(0), unk3f8(10000),
      mMoveOverlay(RndOverlay::Find("ham_move", true)), unk400(this), unk414(0),
      mSkeletonViz(Hmx::Object::New<SkeletonViz>()), unk41c(0), mDebugLatencyOffset(0),
      unkef8(0), mLastPollMs(0), mDebugCollision(0), unkf84(-1) {
    for (int i = 0; i < 2; i++) {
        mMovePlayerData[i].Reset();
        unkf04[i].Reset();
    }
    SetFilterVersion("ham2");
}

MoveDir::~MoveDir() {}

BEGIN_PROPSYNCS(MoveDir)
    SYNC_PROP_SET(current_move, mMovePlayerData[0].mCurMove.Ptr(), )
    SYNC_PROP_SET(filters_enabled, mFiltersEnabled, SetFiltersEnabled(_val.Int()))
    SYNC_PROP_SET(move_overlay, mShowMoveOverlay, SetMoveOverlay(_val.Int()))
    SYNC_PROP(debug_latency_offset, mDebugLatencyOffset)
    SYNC_PROP_SET(
        debug_skeleton_rotation,
        mSkeletonViz->PhysicalCamRotation(),
        mSkeletonViz->SetPhysicalCamRotation(_val.Float())
    )
    SYNC_PROP(debug_collision, mDebugCollision)
    SYNC_PROP(debug_node_types, mErrorNodeInfo)
    SYNC_PROP(debug_node_joints, mErrorNodeInfo)
    SYNC_PROP_SET(play_clip, mPlayClip.Ptr(), SetSongPlayClip(_val.Obj<SkeletonClip>()))
    SYNC_PROP(report_move, mReportMove)
    SYNC_PROP(record_clip, mRecordClip)
    SYNC_PROP(import_clip_path, mImportClipPath)
    SYNC_SUPERCLASS(SkeletonDir)
END_PROPSYNCS

void MoveDir::ClearLimbFeedback(int player) {
    MILO_LOG("MoveDir::ClearLimbFeedback(int player = %d)\n", player);
    CharFeedback *feedback = mMovePlayerData[player].mFeedback;
    HamPlayerData *hpd = TheGameData->Player(player);
    if (feedback && hpd) {
        feedback->ResetErrors();
        for (int i = 0; i < 4; i++) {
            feedback->UpdateLimb(i, false);
        }
    }
}

void MoveDir::SetFiltersEnabled(bool enabled) {
    mFiltersEnabled = enabled;
    if (mFiltersEnabled && TheLoadMgr.EditMode()) {
        MiloInit();
    }
}
