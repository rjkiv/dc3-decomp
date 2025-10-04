#include "hamobj/MoveDir.h"
#include "MoveDir.h"
#include "ScoreUtl.h"
#include "gesture/GestureMgr.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Overlay.h"
#include "utl/Loader.h"
#include "utl/Std.h"

std::vector<FilterVersion *> MoveDir::sFilterVersions;

MoveDir::MoveDir()
    : mShowMoveOverlay(0), mErrorNodeInfo(0), mPlayClip(this), mRecordClip(this),
      unk2bc(this), unk2d0(this), unk2e4(0), mReportMove(this), mFiltersEnabled(0),
      unk308(0), unk30c(0), mFilterQueue(0), mAsyncDetector(0), unk394(0), unk3f8(10000),
      mMoveOverlay(RndOverlay::Find("ham_move", true)), mDancerSeq(this), unk414(0),
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

BEGIN_SAVES(MoveDir)
    SAVE_REVS(35, 0)
    SAVE_SUPERCLASS(SkeletonDir)
    if (IsProxy()) {
        bs << mFiltersEnabled;
    }
    bs << mShowMoveOverlay;
    bs << mErrorNodeInfo;
    if (!bs.Cached()) {
        bs << mImportClipPath;
    } else {
        bs << 0;
    }
    MILO_ASSERT(mFilterVer, 0x922);
    bs << mFilterVer->mVersionSym;
END_SAVES

BEGIN_COPYS(MoveDir)
    COPY_SUPERCLASS(SkeletonDir)
    CREATE_COPY(MoveDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mShowMoveOverlay)
        COPY_MEMBER(mErrorNodeInfo)
        COPY_MEMBER(mImportClipPath)
        COPY_MEMBER(mFiltersEnabled)
        COPY_MEMBER(mPlayClip)
        COPY_MEMBER(mRecordClip)
        COPY_MEMBER(unk2bc)
        COPY_MEMBER(unk2d0)
        COPY_MEMBER(mReportMove)
    END_COPYING_MEMBERS
END_COPYS

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

void MoveDir::SetFilterVersion(Symbol version) {
    for (int i = 0; i < sFilterVersions.size(); i++) {
        if (sFilterVersions[i]->mVersionSym == version) {
            mFilterVer = sFilterVersions[i];
            return;
        }
    }
    MILO_FAIL("Could not find filter version %s", version);
}

const FilterVersion *MoveDir::FindFilterVersion(FilterVersionType t) {
    for (std::vector<FilterVersion *>::iterator it = sFilterVersions.begin();
         it != sFilterVersions.end();
         ++it) {
        if ((*it)->mType == t)
            return *it;
    }
    return nullptr;
}

HamMove *MoveDir::CurrentMove(int player) const {
    MILO_ASSERT((0) <= (player) && (player) < (2), 0x164);
    return mMovePlayerData[player].mCurMove;
}

int MoveDir::MoveIdx() const { return TheTaskMgr.CurrentMeasure(); }
int MoveDir::MoveBeat() const { return TheTaskMgr.CurrentBeat(); }

void MoveDir::SetMoveOverlay(bool overlay) {
    if (!mFiltersEnabled && TheLoadMgr.EditMode()) {
        mFiltersEnabled = true;
        if (TheLoadMgr.EditMode()) {
            MiloInit();
        }
    }
    mShowMoveOverlay = overlay;
    mMoveOverlay->SetShowing(overlay);
}

SkeletonClip *MoveDir::ImportClip(bool b1) {
    if (mImportClipPath.empty()) {
        MILO_NOTIFY("Set import_clip_path first");
        return nullptr;
    } else {
        const char *filename = FileGetName(mImportClipPath.c_str());
        SkeletonClip *clip = Find<SkeletonClip>(filename, false);
        if (clip) {
            MILO_LOG("%s already exists, not importing\n", filename);
        } else {
            clip = Hmx::Object::New<SkeletonClip>();
            clip->SetName(filename, this);
            clip->SetPath(mImportClipPath.c_str());
        }
        return clip;
    }
}

void MoveDir::StopSongRecord() {
    if (mRecordClip && mRecordClip->IsRecording()) {
        mRecordClip->StopRecording();
        if (unk2bc)
            unk2bc->StopRecording();
    } else {
        MILO_NOTIFY("Start recording first");
    }
}

String RecordClipName(const char *, int);

void MoveDir::FlushMoveRecord() {
    SkeletonClip *clip = unk2d0;
    if (clip) {
        clip->FlushMoveRecord(RecordClipName("ktb", -1).c_str());
    } else {
        MILO_NOTIFY("skeleton recording not yet active");
    }
}

void MoveDir::SwapMoveRecord() {
    if (unk2d0) {
        unk2d0->SwapMoveRecord();
    } else {
        MILO_NOTIFY("skeleton recording not yet active");
    }
}

HamMove *MoveDir::GetMoveAtMeasure(int player, int i2) {
    static Symbol move("move");
    HamPlayerData *hpd = TheGameData->Player(player);
    Keys<Symbol, Symbol> *keys =
        TheHamDirector->GetPropKeys(hpd->GetDifficulty(), move)->AsSymbolKeys();
    return Find<HamMove>((*keys)[i2].value.Str(), false);
    return nullptr;
}

DancerSequence *MoveDir::PerformanceSequence(Difficulty diff) {
    MILO_ASSERT((0) <= (diff) && (diff) < (kNumDifficulties), 0x207);
    Symbol diffSym = DifficultyToSym(diff);
    const char *seqName = MakeString("performance_%s.seq", diffSym);
    return Find<DancerSequence>(seqName, false);
}

void SetupRecordClip(
    ObjPtr<SkeletonClip> &clip, int i1, int i2, const char *cc, ObjectDir *dir
) {
    clip = Hmx::Object::New<SkeletonClip>();
    clip->EnableAlternateRecord(i1);
    String clipName = RecordClipName(cc, i1);
    clipName += ".clp";
    clip->SetName(clipName.c_str(), dir);
    const char *path = MakeString("devkit:\\%s", clip->Name());
    MILO_LOG("Starting song recording: %s\n", path);
    clip->StartXboxRecording(path);
}

void MoveDir::FinishGameRecord() {
    MILO_ASSERT(!TheLoadMgr.EditMode(), 0x604);
    if (mRecordClip) {
        MILO_LOG("Finishing song recording: %s\n", mRecordClip->Path());
        mRecordClip->StopRecording();
        RELEASE(mRecordClip);
    }
    if (unk2bc) {
        MILO_LOG("Finishing song recording: %s\n", unk2bc->Path());
        unk2bc->StopRecording();
        RELEASE(unk2bc);
    }
    RELEASE(unk2d0);
}

void MoveDir::SetupSongRecordClip() {
    static Symbol rhythm_battle("rhythm_battle");
    bool b1 = unk308 && unk308->Type() == rhythm_battle;
    bool b7 = false;
    if (unk308) {
        static Message msg("is_game_over");
        b7 = unk308->Handle(msg, true).Int();
    }
    if (!b7) {
        const char *modeStr;
        if (b1) {
            modeStr = "ktb";
        } else if (TheHamDirector->InPracticeMode()) {
            modeStr = "bid";
        } else
            modeStr = "pi";
        if (sGameRecord && !mRecordClip) {
            unsigned int x = sGameRecord2Player;
            if (x) {
                SetupRecordClip(mRecordClip, 0, 0, modeStr, this);
                SetupRecordClip(unk2bc, 1, 1, modeStr, this);
            } else {
                SetupRecordClip(mRecordClip, x, -1, modeStr, this);
            }
        }
        if (b1 && !unk2d0) {
            SetupRecordClip(unk2d0, 2, 0, modeStr, this);
        }
    }
}

void MoveDir::SetDancerSequence(DancerSequence *seq) { mDancerSeq = seq; }

void MoveDir::LoadScoring(const DataArray *cfg) {
    static Symbol min_frame_dist_beats("min_frame_dist_beats");
    cfg->FindData(min_frame_dist_beats, HamMove::sMinFrameDistBeats);
    static Symbol latency_offset("latency_offset");
    cfg->FindData(latency_offset, sLatencySeconds);
    sLatencySeconds /= 1000;
    static Symbol plf_min_time_error("plf_min_time_error");
    sPLFMinTimeError = cfg->FindFloat(plf_min_time_error);
    ScoreUtlInit(cfg);
    DeleteAll(sFilterVersions);
    DataArray *versionsArr = cfg->FindArray("versions");
    for (int i = 1; i < versionsArr->Size(); i++) {
        sFilterVersions.push_back(FilterVersion::Create(versionsArr->Array(i)));
    }
    MILO_ASSERT(!sFilterVersions.empty(), 0x2E2);
}

void MoveDir::ReloadScoring() {
    MILO_ASSERT(TheLoadMgr.EditMode(), 0x1268);
    DataArray *cfg = SystemConfig("scoring");
    DataArray *file = DataReadFile(cfg->Array(1)->File(), true);
    LoadScoring(file);
    ScoreUtlInit(file);
    Enter();
    file->Release();
}

void MoveDir::ResetDetection() {
    if (TheHamDirector) {
        if (SkeletonUpdate::HasInstance()) {
            SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
            if (!handle.HasCallback(this)) {
                handle.AddCallback(this);
            }
        }
        MILO_ASSERT(TheGameData, 0x642);
        for (int i = 0; i < 2; i++) {
            HamPlayerData *player_data = TheGameData->Player(i);
            MILO_ASSERT(player_data, 0x646);
            if (player_data->IsPlaying()) {
                ResetDetectFrames(i, player_data->GetDifficulty());
            }
        }
        SetupSongRecordClip();
    }
}

void MoveDir::SetSongPlayClip(SkeletonClip *clip) {
    if (!mFiltersEnabled && clip) {
        mFiltersEnabled = true;
        if (TheLoadMgr.EditMode()) {
            MiloInit();
        }
    }
    if (mRecordClip && mRecordClip->IsRecording()) {
        MILO_NOTIFY("Can't set play clip while recording");
    } else {
        mPlayClip = clip;
        SetSkeletonClip(clip);
        ResetDetection();
        TheGameData->UnassignSkeletons();
    }
}

void MoveDir::MiloUpdate() {
    SkeletonDir::MiloUpdate();
    MILO_ASSERT(TheGestureMgr, 0xB14);
    SetCurrentMove(0, mMovePlayerData[0].mCurMove);
    SetMoveOverlay(mShowMoveOverlay);
    SetSongPlayClip(mPlayClip);
}
