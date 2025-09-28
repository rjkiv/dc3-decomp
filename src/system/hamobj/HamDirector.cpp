#include "hamobj/HamDirector.h"
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "SongUtl.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "flow/Flow.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/TexRenderer.h"
#include "rndobj/Trans.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "world/CameraManager.h"
#include "world/Dir.h"

HamDirector *TheHamDirector;

HamDirector::HamDirector()
    : mMasterClipAnim(this), mPlayer1RoutineBuilderAnim(this),
      mPlayer2RoutineBuilderAnim(this), unkc8(0), unkcc(""), mBackupDrift(1),
      mMerger(this), mMoveMerger(this), mGameModeMerger(this), mVenue(this), unk124(this),
      unk140(0), unk14c(0), mWorldPostProc(this), mCamPostProc(this),
      mForcePostProc(this), unk18c(this), mForcePostProcBlend(0),
      mForcePostProcBlendRate(1), unk1a8(this), unk1bc(this), unk1d0(0), unk1d4(0),
      unk1d8(this), mVisualizerPostProc(this), mFreestyleEnabled(1), mPlayer0Char(this),
      mPlayer1Char(this), mBackup0Char(this), mBackup1Char(this), unk254(0), mDisabled(0),
      unk25a(0), mCurShot(this), mNextShot(this), unk284(this), unk29c(-kHugeFloat),
      mDisablePicking(0), unk2a1(0), unk2a4(0), unk2a8(-kHugeFloat), unk2ac(1),
      mPlayerFreestyle(0), mPlayerFreestylePaused(0), mVisualizer(this),
      mPracticeStart(0), mPracticeEnd(0), mStartLoopMargin(1), mEndLoopMargin(1),
      mBlendDebug(0), unk304(0), mClipDir(this), mMoveDir(this), mNoTransitions(0),
      mCollisionChecks(1), mLoadedNewSong(1), mPoseFatalities(0), unk33c(RandomInt(0, 2)),
      unk33d(0), mIconManChar(this), mIconManTex(this), unk369(0), mOfflineSong(0) {
    static DataNode &n = DataVariable("hamdirector");
    n = this;
    TheHamDirector = this;
    mDirCutKeys.reserve(100);
}

HamDirector::~HamDirector() {
    MILO_ASSERT(TheGameData, 0xC5);
    TheGameData->Clear();
    if (TheHamDirector == this) {
        static DataNode &n = DataVariable("hamdirector");
        n = NULL_OBJ;
        TheHamDirector = nullptr;
    }
    delete mPoseFatalities;
}

BEGIN_HANDLERS(HamDirector)
    HANDLE(shot_over, OnShotOver)
    HANDLE(postproc_interp, OnPostProcInterp)
    HANDLE(save_song, OnSaveSong)
    HANDLE(save_face_anims, OnSaveFaceAnims)
    HANDLE(on_file_loaded, OnFileLoaded)
    HANDLE(on_file_merged, OnFileMerged)
    HANDLE(load_song, OnLoadSong)
    HANDLE_EXPR(is_world_loaded, IsWorldLoaded())
    HANDLE_ACTION(unload_all, UnloadAll())
    HANDLE_ACTION(pick_new_shot, unk140 = true)
    HANDLE(select_camera, OnSelectCamera)
    HANDLE(cycle_shot, OnCycleShot)
    HANDLE(force_shot, OnForceShot)
    HANDLE_EXPR(camera_source, mVenue)
    HANDLE_ACTION(force_scene, ForceScene(_msg->Sym(2)))
    HANDLE_ACTION(force_minivenue, ForceMiniVenue(_msg->Sym(2)))
    HANDLE(cur_postprocs, OnPostProcs)
    HANDLE_ACTION(reselect_world_postproc, ReselectWorldPostProc())
    HANDLE_EXPR(get_venue_world, GetVenueWorld())
    HANDLE_EXPR(get_world, mMerger ? mMerger->Dir() : (ObjectDir *)nullptr)
    HANDLE(set_dircut, OnSetDircut)
    HANDLE(get_dancer_visemes, OnGetDancerVisemes)
    HANDLE_ACTION(play_base_visemes, PlayCharBaseVisemes())
    HANDLE_ACTION(enable_facial_animation, EnableFacialAnimation())
    HANDLE_ACTION(disable_facial_animation, DisableFacialAnimation())
    HANDLE_ACTION(reset_facial_animation, ResetFacialAnimation())
    HANDLE_ACTION(set_lipsync_offsets, SetLipsyncOffsets(_msg->Float(2)))
    HANDLE_ACTION(resync_face_drivers, ResyncFaceDrivers())
    HANDLE(blend_face_clip, OnBlendInFaceClip)
    HANDLE_ACTION(blend_face_overrides_in, BlendInFaceOverrides(_msg->Float(2)))
    HANDLE_ACTION(blend_face_overrides_out, BlendOutFaceOverrides(_msg->Float(2)))
    HANDLE(practice_beats, OnPracticeBeats)
    HANDLE_EXPR(beat_to_movename, MoveNameFromBeat(_msg->Float(2), _msg->Int(3)))
    HANDLE_EXPR(is_intro, strneq(_msg->Sym(2).Str(), "INTRO_", 6))
    HANDLE_ACTION(initialize, Initialize())
    HANDLE_EXPR(player_song_anim, SongAnim(_msg->Int(2)))
    HANDLE_EXPR(difficulty_song_anim, SongAnimByDifficulty((Difficulty)_msg->Int(2)))
    HANDLE_EXPR(
        dancer_face_anim_by_difficulty,
        mDancerFaceAnims[LegacyDifficulty((Difficulty)_msg->Int(2))].Ptr()
    )
    HANDLE_EXPR(dancer_face_anim_by_player, DancerFaceAnimByPlayer(_msg->Int(2)))
    HANDLE_EXPR(toggle_camshot_flag, OnToggleCamshotFlag())
    HANDLE_EXPR(get_character_sym, unk2f4[_msg->Int(2)])
    HANDLE_ACTION(hide_backups, HideBackups(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(restore_backups, RestoreBackups())
    HANDLE_ACTION(teleport_chars, TeleportChars())
    HANDLE_ACTION(reteleport, Reteleport())
    HANDLE_EXPR(list_possible_move, OnListPossibleMoves())
    HANDLE_EXPR(list_possible_variants, OnListPossibleVariants())
    HANDLE_ACTION(set_grooviness, mVisualizer->SetGrooviness(_msg->Float(2)))
    HANDLE_ACTION(start_stop_visualizer, StartStopVisualizer(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(set_player_spotlights_enabled, SetPlayerSpotlightsEnabled(_msg->Int(2)))
    HANDLE_ACTION(hud_entered, HudEntered())
    HANDLE_ACTION(
        change_player_character,
        ChangePlayerCharacter(_msg->Int(2), _msg->Sym(3), _msg->Sym(4), _msg->Sym(5))
    )
    HANDLE_ACTION(set_suppress_intro_shot, unk2a1 = _msg->Int(2))
    HANDLE_EXPR(get_suppress_intro_shot, 0)
    HANDLE_ACTION(set_suppress_next_shot, unk2a4 = _msg->Int(2))
    HANDLE_EXPR(get_suppress_next_shot, unk2a4)
    HANDLE_EXPR(is_game_start_hold, unk33d)
    HANDLE_ACTION(enable_poll, unk2ac = _msg->Int(2))
    HANDLE(clip_annotate, OnClipAnnotate)
    HANDLE(clip_safetoadd, OnClipSafeToAdd)
    HANDLE(clip_list, OnClipList)
    HANDLE(practice_safetoadd, OnPracticeSafeToAdd)
    HANDLE(practice_annotate, OnPracticeAnnotate)
    HANDLE_EXPR(practice_list, PracticeList((Difficulty)_msg->Int(2)))
    HANDLE(toggle_debug_interests, OnToggleDebugInterests)
    HANDLE_ACTION(init_offline, InitOffline())
    HANDLE_ACTION(offline_load_song, OfflineLoadSong(_msg->Sym(2)))
    HANDLE(toggle_cam_character_skeleton, OnToggleCamCharacterSkeleton)
    HANDLE_ACTION(populate_moves, OnPopulateMoves())
    HANDLE_ACTION(populate_movemgr, OnPopulateMoveMgr())
    HANDLE_ACTION(populate_from_file, OnPopulateFromFile())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamDirector)
    SYNC_PROP_SET(shot, mShot, SetShot(_val.Sym()))
    static Symbol none("none");
    SYNC_PROP_SET(postproc, NULL_OBJ, )
    SYNC_PROP_SET(world_event, none, SetWorldEvent(_val.Sym()))
    SYNC_PROP_SET(clip, ClosestMove(), )
    SYNC_PROP_SET(practice, Symbol(), )
    SYNC_PROP_SET(move, Symbol(), )
    SYNC_PROP_SET(move_instance, Symbol(), )
    SYNC_PROP_SET(move_parents, Symbol(), )
    SYNC_PROP_SET(clip_crossover, Symbol(), )
    SYNC_PROP(merger, mMerger)
    SYNC_PROP(game_mode_merger, mGameModeMerger)
    SYNC_PROP(move_merger, mMoveMerger)
    SYNC_PROP(disable_picking, mDisablePicking)
    SYNC_PROP_SET(player_freestyle, mPlayerFreestyle, UpdatePlayerFreestyle(_val.Int()))
    SYNC_PROP_SET(
        pause_player_freestyle, mPlayerFreestylePaused, PausePlayerFreestyle(_val.Int())
    )
    SYNC_PROP(force_postproc, mForcePostProc)
    SYNC_PROP(force_postproc_blend, mForcePostProcBlend)
    SYNC_PROP(force_postproc_blend_rate, mForcePostProcBlendRate)
    SYNC_PROP(disabled, mDisabled)
    SYNC_PROP(excitement, mExcitement)
    SYNC_PROP(num_players_failed, mNumPlayersFailed)
    SYNC_PROP(cam_postproc, mCamPostProc)
    SYNC_PROP_SET(cur_shot, mCurShot.Ptr(), )
    SYNC_PROP_SET(cur_world, mVenue.Ptr(), )
    SYNC_PROP_SET(backup_drift, mBackupDrift, )
    SYNC_PROP_SET(spot_instructor, Symbol("off"), SetCharSpot("instructor", _val.Sym()))
    SYNC_PROP(practice_start, mPracticeStart)
    SYNC_PROP(practice_end, mPracticeEnd)
    SYNC_PROP(start_loop_margin, mStartLoopMargin)
    SYNC_PROP(end_loop_margin, mEndLoopMargin)
    SYNC_PROP(blend_debug, mBlendDebug)
    SYNC_PROP(no_transitions, mNoTransitions)
    SYNC_PROP(collision_checks, mCollisionChecks)
    SYNC_PROP_SET(
        dancer_face_clip, GetMainFaceOverrideClip(), SetMainFaceOverrideClip(_val.Sym())
    )
    SYNC_PROP_SET(
        dancer_face_weight,
        GetMainFaceOverrideWeight(),
        SetMainFaceOverrideWeight(_val.Float())
    )
    SYNC_PROP(freestyle_enabled, mFreestyleEnabled)
    SYNC_PROP(loaded_new_song, mLoadedNewSong)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamDirector)
    SAVE_REVS(9, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndPollable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mPracticeStart;
    bs << mPracticeEnd;
    bs << mBlendDebug;
    bs << mNoTransitions;
    bs << mCollisionChecks;
    bs << mStartLoopMargin;
    bs << mEndLoopMargin;
END_SAVES

void HamDirector::ListPollChildren(std::list<RndPollable *> &polls) const {
    if (mVenue) {
        polls.push_back(mVenue);
    }
}

void HamDirector::ListDrawChildren(std::list<RndDrawable *> &draws) {
    if (mVenue) {
        draws.push_back(mVenue);
    }
}

DataNode HamDirector::OnSaveSong(DataArray *) { return 0; }
DataNode HamDirector::OnSaveFaceAnims(DataArray *) { return 0; }
DataNode HamDirector::OnFileMerged(DataArray *) { return 0; }

void HamDirector::ForceScene(Symbol s) {
    unk13c = s;
    unk138 = gNullStr;
}

void HamDirector::ForceMiniVenue(Symbol s) {
    unk138 = s;
    Symbol idk(gNullStr);
}

void HamDirector::DrawDebug() {
    if (mPoseFatalities)
        mPoseFatalities->DrawDebug();
}

void HamDirector::ArmMultiIntroMode() {
    unk33d = true;
    mDisablePicking = true;
}

void HamDirector::HudEntered() {
    if (mPoseFatalities)
        mPoseFatalities->Enter();
}

void HamDirector::PlayIntroShot() {
    if (!unk284)
        PickIntroShot();
    if (!unk2a1) {
        if (unk284) {
            static Message msg("set_intro_shot", 0);
            msg[0] = unk284.Ptr();
            DataNode handled = HandleType(msg);
            mNextShot = unk284;
            unk284 = nullptr;
        } else
            FindNextShot();
        PlayNextShot();
    }
}

void HamDirector::SetupAnims() {
    mSongAnims.clear();
    mDancerFaceAnims.clear();
    for (int i = 0; i < 3; i++) {
        Difficulty d = (Difficulty)i;
        mSongAnims[d] = GetPropAnim(d, "song.anim", true);
        mDancerFaceAnims[d] = GetPropAnim(d, "dancer_face.anim", false);
    }
    SetupRoutineBuilderAnims();
    mClipDir = mMerger->Dir()->Find<ObjectDir>("clips", false);
    mMoveDir = mMerger->Dir()->Find<ObjectDir>("moves", false);
    ObjDirItr<SongCollision> it(mMoveDir, true);
    if (it)
        unk124 = &*it;
}

WorldDir *HamDirector::GetWorld() {
    return mMerger ? dynamic_cast<WorldDir *>(mMerger->Dir()) : nullptr;
}

WorldDir *HamDirector::GetVenueWorld() { return mVenue; }

void HamDirector::Initialize() {
    SetupAnims();
    ObjectDir *iconManDir = GetWorld()->Find<ObjectDir>("iconmandir", false);
    if (iconManDir) {
        mIconManChar = iconManDir->Find<Character>("iconman", false);
        if (mIconManChar) {
            RndAnimatable *anim =
                mIconManChar->Find<RndAnimatable>("outline.anim", false);
            if (anim)
                anim->SetFrame(1, 1);
        }
        mIconManTex = iconManDir->Find<RndTexRenderer>("iconman.rndtex", false);
    }
    delete mPoseFatalities;
    mPoseFatalities = Hmx::Object::New<PoseFatalities>();
}

RndPropAnim *HamDirector::SongAnim(int playerIndex) {
    if (!mSongAnims[kDifficultyEasy]) {
        return nullptr;
    } else {
        MILO_ASSERT((0) <= (playerIndex) && (playerIndex) < (2), 0x620);
        if (TheHamProvider->Property("merge_moves", true)->Int()) {
            return playerIndex == 0 ? mPlayer1RoutineBuilderAnim
                                    : mPlayer2RoutineBuilderAnim;
        } else {
            HamPlayerData *hpd = TheGameData->Player(playerIndex);
            return SongAnimByDifficulty(LegacyDifficulty(hpd->GetDifficulty()));
        }
    }
}

PropKeys *HamDirector::GetPropKeys(Difficulty d, Symbol s) {
    RndPropAnim *anim = GetPropAnim(d, "song.anim", false);
    if (!anim) {
        return nullptr;
    } else {
        return anim->GetKeys(this, DataArrayPtr(s));
    }
}

void HamDirector::VenueEnter(WorldDir *dir) {
    if (dir)
        dir->Enter();
    mPlayer0Char = dir ? dir->Find<HamCharacter>("player0", true) : nullptr;
    mPlayer1Char = dir ? dir->Find<HamCharacter>("player1", true) : nullptr;
    mBackup0Char = dir ? dir->Find<HamCharacter>("backup0", true) : nullptr;
    mBackup1Char = dir ? dir->Find<HamCharacter>("backup1", true) : nullptr;

    RndTransformable *p0 =
        dir ? dir->Find<RndTransformable>("player0.trans", true) : nullptr;
    RndTransformable *p1 =
        dir ? dir->Find<RndTransformable>("player1.trans", true) : nullptr;
    RndTransformable *b0 =
        dir ? dir->Find<RndTransformable>("backup0.trans", true) : nullptr;
    RndTransformable *b1 =
        dir ? dir->Find<RndTransformable>("backup1.trans", true) : nullptr;

    if (b1) {
        MILO_LOG(
            "(%7.2f,%7.2f,%7.2f)\n",
            b1->LocalXfm().v.x,
            b1->LocalXfm().v.y,
            b1->LocalXfm().v.z
        );
    } else {
        MILO_LOG("NULL\n");
    }

    if (p0) {
        p0->SetLocalXfm(Transform::IDXfm());
    }
    if (p1) {
        p1->SetLocalXfm(Transform::IDXfm());
    }
    if (b0) {
        b0->SetLocalXfm(Transform::IDXfm());
    }
    if (b1) {
        b1->SetLocalXfm(Transform::IDXfm());
    }
    unk254 = false;
    for (int i = 0; i < 4; i++) {
        unk255[i] = false;
    }
}

void HamDirector::SetMasterClipAnim() {
    WorldDir *dir = GetWorld();
    if (dir) {
        ObjectDir *clipDir = dir->Find<ObjectDir>("master_clips", false);
        if (clipDir) {
            mMasterClipAnim = clipDir->Find<RndPropAnim>("song.anim", false);
        }
        if (!mMasterClipAnim) {
            mMasterClipAnim = GetPropAnim(kDifficultyExpert, "song.anim", false);
        }
    }
}

void HamDirector::PickIntroShot() {
    if (!DataVariable("skip_intro").Int()) {
        mNextShot = nullptr;
        static Message m("pick_intro_shot");
        DataNode n = HandleType(m);
        unk284 = mNextShot;
        mNextShot = nullptr;
    }
}

void HamDirector::ForceShot(const char *name) {
    if (mVenue) {
        mNextShot = mVenue->Find<HamCamShot>(name, false);
        mDisablePicking = mNextShot;
    }
}

PropKeys *HamDirector::GetMasterKeys(Symbol s) {
    if (!mMasterClipAnim) {
        SetMasterClipAnim();
    }
    if (!mMasterClipAnim) {
        MILO_NOTIFY(
            "HamDirector::GetMasterKeys: no master clip anim, can't return PropKeys."
        );
        return nullptr;
    } else {
        return mMasterClipAnim->GetKeys(this, DataArrayPtr(s));
    }
}

Key<Symbol> *HamDirector::GetMasterPracticeFrame(Symbol s) {
    if (!mMasterClipAnim) {
        SetMasterClipAnim();
    }
    MILO_ASSERT(mMasterClipAnim, 0x23E);
    static Symbol practice("practice");
    PropKeys *keys = mMasterClipAnim->GetKeys(this, DataArrayPtr(practice));
    if (keys) {
        Keys<Symbol, Symbol> *symKeys = keys->AsSymbolKeys();
        int i = 0;
        for (; i < symKeys->size(); i++) {
            if (s == (*symKeys)[i].value) {
                goto done;
            }
        }
        i = -1;
    done:
        if (i != -1) {
            return &(*symKeys)[i];
        }
    }
    return nullptr;
}

HamCamShot *HamDirector::FindNextDircut() {
    float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    const DircutEntry *entry = mDirCutKeys.Cross(secs, secs - TheTaskMgr.DeltaSeconds());
    if (entry) {
        HamCamShot *ret = nullptr;
        if (mNumPlayersFailed != 0 || (entry->unk4 && mExcitement < 3)) {
            ret = entry->unk0;
            unk140 = true;
        }
        return ret;
    }
    return nullptr;
}

void HamDirector::SetDircut(Symbol s, std::vector<CameraManager::PropertyFilter> filters) {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol holla_back("holla_back");
    if (TheHamProvider->Property(gameplay_mode, true)->Sym() == holla_back) {
        return;
    } else {
        MILO_LOG("HamDirector::SetDircut cat = '%s'\n", s.Str());
        mNextShot = dynamic_cast<HamCamShot *>(
            mVenue->GetCameraManager()->FindCameraShot(s, filters)
        );
        MILO_LOG("   mNextShot = '%s'\n", SafeName(mNextShot));
    }
}

void HamDirector::SetupRoutineBuilderAnims() {
    for (int i = 0; i < 2; i++) {
        RndPropAnim *routineBuilderAnim;
        if (i == 0) {
            mPlayer1RoutineBuilderAnim =
                GetWorld()->Find<RndPropAnim>("player_1_routine_builder.anim", true);
            routineBuilderAnim = mPlayer1RoutineBuilderAnim;
        } else {
            mPlayer2RoutineBuilderAnim =
                GetWorld()->Find<RndPropAnim>("player_2_routine_builder.anim", true);
            routineBuilderAnim = mPlayer2RoutineBuilderAnim;
        }
        HamPlayerData *hpd = TheGameData->Player(i);
        RndPropAnim *anim = mSongAnims[LegacyDifficulty(hpd->GetDifficulty())];
        if (anim) {
            routineBuilderAnim->Copy(anim, kCopyDeep);
            Symbol syms[3] = { "clip", "move", "practice" };
            for (int i = 0; i < 3; i++) {
                DataArrayPtr ptr(syms[i]);
                routineBuilderAnim->GetKeys(this, ptr)->AsSymbolKeys()->clear();
            }
        }
    }
}

RndPropAnim *HamDirector::SongAnimByDifficulty(Difficulty diff) {
    MILO_ASSERT((0) <= (diff) && (diff) < (kNumDifficultiesDC2), 0x633);
    return mSongAnims[diff];
}

RndPropAnim *HamDirector::DancerFaceAnimByPlayer(int player) {
    return mDancerFaceAnims[LegacyDifficulty(TheGameData->Player(player)->GetDifficulty()
    )];
}

void HamDirector::AddNumPlayers(
    std::vector<CameraManager::PropertyFilter> &filters, DataArray *arr
) {
    CameraManager::PropertyFilter filter;
    if (arr) {
        filter.prop = arr->Sym(0);
        filter.match = arr->Array(1);
    } else {
        static Symbol player_flag("player_flag");
        filter.prop = player_flag;
        static Symbol cam_player_config("cam_player_config");
        DataArrayPtr ptr(3, TheHamProvider->Property(cam_player_config, true)->Int());
        filter.match = (DataArray *)ptr;
    }
    filters.push_back(filter);
}

PropKeys *HamDirector::GetPropKeysByPlayer(int player, Symbol s) {
    RndPropAnim *anim = SongAnim(player);
    if (!anim) {
        return nullptr;
    } else {
        return anim->GetKeys(this, DataArrayPtr(s));
    }
}

Symbol HamDirector::MoveNameFromBeat(float f1, int player) {
    RndPropAnim *anim = SongAnim(player);
    if (!anim)
        return gNullStr;
    else {
        PropKeys *keys = anim->GetKeys(this, DataArrayPtr(Symbol("move")));
        if (!keys)
            return gNullStr;
        else {
            Symbol ret;
            float frame = BeatToFrame(f1);
            Keys<Symbol, Symbol> *symKeys = keys->AsSymbolKeys();
            symKeys->AtFrame(frame, ret);
            return ret;
        }
    }
}

void HamDirector::TriggerNextIntro() {
    mDisablePicking = false;
    std::vector<CameraManager::PropertyFilter> filters;
    static Symbol s("CAMP_SONG1_INTRO_CONTINUE");
    SetDircut(s, filters);
    unk284 = mNextShot;
    mNextShot = nullptr;
    PlayIntroShot();
    unk33d = false;
}

void HamDirector::ReactToCollision_InsertRealShot(Symbol s, float f2) {
    static Symbol shot("shot");
    PropKeys *keys = GetPropKeysByPlayer(0, shot);
    Keys<Symbol, Symbol> *shot_keys = keys->AsSymbolKeys();
    MILO_ASSERT(shot_keys, 0xE08);
    shot_keys->Add(s, BeatToFrame(TheTaskMgr.Beat()), false);
}

void HamDirector::ReactToCollision_MoveShot(int shotIdx, float beat) {
    static Symbol shot("shot");
    PropKeys *shot_keys = GetPropKeysByPlayer(0, shot);
    MILO_ASSERT(shot_keys, 0xE10);
    shot_keys->ChangeFrame(shotIdx, BeatToFrame(beat), true);
}

bool HamDirector::ShouldDoCollisionPrevention() const {
    if (TheLoadMgr.EditMode() && !mCollisionChecks) {
        return false;
    } else {
        static Symbol cam_player_config("cam_player_config");
        return TheHamProvider->Property(cam_player_config, true)->Int() == 2;
    }
}

void HamDirector::StartStopVisualizer(bool b1, int i2) {
    if (mVisualizer && unk368 != b1) {
        unk368 = b1;
        mVisualizer->SetShowing(b1);
        mVisualizer->Run(b1);
        if (b1) {
            mVisualizer->Find<Flow>("enter_timeywimey.flow", true)->Activate();
        } else {
            if (mVisualizerPostProc) {
                mVisualizerPostProc->Unselect();
            }
            switch (i2) {
            case 0:
                mVisualizer->Find<Flow>("exit_timeywimey.flow", true)->Activate();
                break;
            case 1:
                mVisualizer->Find<Flow>("exit_timeywimey_fast.flow", true)->Activate();
                break;
            case 2:
                mVisualizer->Find<Flow>("exit_timeywimey_totimeywimey.flow", true)
                    ->Activate();
                mVisualizer->SetShowing(false);
                break;
            default:
                break;
            }
        }
    }
}

void HamDirector::UnselectVisualizerPostProc() {
    if (mVisualizerPostProc)
        mVisualizerPostProc->Unselect();
}

void HamDirector::ReselectWorldPostProc() {
    MILO_LOG("HamDirector::ReselectWorldPostProc()\n");
    if (mWorldPostProc)
        mWorldPostProc->Select();
}

void HamDirector::StartStopVisualizer() {
    if (mVisualizer) {
        mVisualizer->SetShowing(mPlayerFreestyle);
    }
    if (mVisualizer) {
        StartStopVisualizer(mPlayerFreestyle, 1);
    }
}

void HamDirector::UpdatePlayerFreestyle(bool b1) {
    if (b1 != mPlayerFreestyle) {
        static Symbol in_freestyle("in_freestyle");
        static Symbol game_stage("game_stage");
        mPlayerFreestyle = b1;
        if (mPlayerFreestyle) {
            unk1d4 = 0;
            unk1d8 = mForcePostProc;
            mForcePostProc = mVisualizerPostProc;
            mForcePostProcBlend = 0;
            mForcePostProcBlendRate = 0.625;
            if (GetWorld()) {
                static Symbol freestyle("freestyle");
                TheHamProvider->SetProperty(game_stage, freestyle);
                HamPlayerData *pPlayer0 = TheGameData->Player(0);
                MILO_ASSERT(pPlayer0, 0xEDF);
                HamPlayerData *pPlayer1 = TheGameData->Player(1);
                MILO_ASSERT(pPlayer1, 0xEE1);
                PropertyEventProvider *pPlayer0Provider = pPlayer0->Provider();
                MILO_ASSERT(pPlayer0Provider, 0xEE4);
                PropertyEventProvider *pPlayer1Provider = pPlayer1->Provider();
                MILO_ASSERT(pPlayer1Provider, 0xEE6);
                bool p1InFreestyle = pPlayer1->InFreestyle();
                pPlayer0Provider->SetProperty(in_freestyle, pPlayer0->InFreestyle());
                pPlayer1Provider->SetProperty(in_freestyle, p1InFreestyle);
            }
        } else {
            StartStopVisualizer();
            mForcePostProc = unk1d8;
            mForcePostProcBlendRate = 0;
            mForcePostProcBlend = 1;
            if (GetWorld()) {
                static Symbol playing("playing");
                TheHamProvider->SetProperty(game_stage, playing);
                HamPlayerData *pPlayer0 = TheGameData->Player(0);
                MILO_ASSERT(pPlayer0, 0xEFA);
                HamPlayerData *pPlayer1 = TheGameData->Player(1);
                MILO_ASSERT(pPlayer1, 0xEFC);
                PropertyEventProvider *pPlayer0Provider = pPlayer0->Provider();
                MILO_ASSERT(pPlayer0Provider, 0xEFF);
                PropertyEventProvider *pPlayer1Provider = pPlayer1->Provider();
                MILO_ASSERT(pPlayer1Provider, 0xF01);
                pPlayer0Provider->SetProperty(in_freestyle, false);
                pPlayer1Provider->SetProperty(in_freestyle, false);
            }
        }
    }
}

void HamDirector::SetWorldEvent(Symbol event) {
    static Symbol none("none");
    if (event != none && mVenue) {
        static Message msg("");
        msg.SetType(event);
        mVenue->Handle(msg, false);
    }
}

void HamDirector::SendCurWorldMsg(Symbol s, bool b2) {
    static Message msg("");
    if (mVenue) {
        msg.SetType(s);
        if (b2) {
            mVenue->HandleType(msg);
        } else {
            mVenue->Handle(msg, false);
        }
    }
}

void HamDirector::SetCharSpot(Symbol s1, Symbol s2) {
    SendCurWorldMsg(MakeString("spotlight_%s_%s", s1.Str(), s2.Str()), false);
}
