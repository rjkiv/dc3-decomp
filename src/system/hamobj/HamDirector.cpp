#include "hamobj/HamDirector.h"
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/PropAnim.h"
#include "rndobj/TexRenderer.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "world/Dir.h"

HamDirector *TheHamDirector;

HamDirector::HamDirector()
    : unk8c(this), unka0(this), unkb4(this), unkc8(0), unkcc(""), mBackupDrift(1),
      mMerger(this), mMoveMerger(this), mGameModeMerger(this), mCurWorld(this),
      unk124(this), unk140(0), unk14c(0), unk150(this), mCamPostProc(this),
      mForcePostProc(this), unk18c(this), mForcePostProcBlend(0),
      mForcePostProcBlendRate(1), unk1a8(this), unk1bc(this), unk1d0(0), unk1d4(0),
      unk1d8(this), unk1ec(this), mFreestyleEnabled(1), unk204(this), unk218(this),
      unk22c(this), unk240(this), unk254(0), mDisabled(0), unk25a(0), mCurShot(this),
      unk270(this), unk284(this), unk29c(-kHugeFloat), mDisablePicking(0), unk2a1(0),
      unk2a4(0), unk2a8(-kHugeFloat), unk2ac(1), mPlayerFreestyle(0),
      mPlayerFreestylePaused(0), unk2c0(this), mPracticeStart(0), mPracticeEnd(0),
      mStartLoopMargin(1), mEndLoopMargin(1), mBlendDebug(0), unk304(0), unk308(this),
      unk31c(this), mNoTransitions(0), mCollisionChecks(1), mLoadedNewSong(1),
      mPoseFatalities(0), unk33c(RandomInt(0, 2)), unk33d(0), unk340(this), unk354(this),
      unk369(0), mOfflineSong(0) {
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
    HANDLE_EXPR(camera_source, mCurWorld)
    HANDLE_ACTION(force_scene, ForceScene(_msg->Sym(2)))
    HANDLE_ACTION(force_minivenue, ForceMiniVenue(_msg->Sym(2)))
    HANDLE(cur_postprocs, OnPostProcs)
    HANDLE_ACTION(reselect_world_postproc, ReselectWorldPostProc())
    HANDLE_EXPR(get_venue_world, mCurWorld)
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
        dancer_face_anim_by_difficulty, unk74[(Difficulty)(_msg->Int(2) != 3)].Ptr()
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
    HANDLE_ACTION(set_grooviness, unk2c0->SetGrooviness(_msg->Float(2)))
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
    SYNC_PROP_SET(cur_world, mCurWorld.Ptr(), )
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
            unk270 = unk284;
            unk284 = nullptr;
        } else
            FindNextShot();
        PlayNextShot();
    }
}

void HamDirector::SetupAnims() {
    unk5c.clear();
    unk74.clear();
    for (int i = 0; i < 3; i++) {
        Difficulty d = (Difficulty)i;
        unk5c[d] = GetPropAnim(d, "song.anim", true);
        unk74[d] = GetPropAnim(d, "dancer_face.anim", false);
    }
    SetupRoutineBuilderAnims();
    unk308 = mMerger->Dir()->Find<ObjectDir>("clips", false);
    unk31c = mMerger->Dir()->Find<ObjectDir>("moves", false);
    ObjDirItr<SongCollision> it(unk31c, true);
    if (it)
        unk124 = &*it;
}

void HamDirector::Initialize() {
    SetupAnims();
    WorldDir *dir = mMerger ? dynamic_cast<WorldDir *>(mMerger->Dir()) : nullptr;
    ObjectDir *iconManDir = dir->Find<ObjectDir>("iconmandir", false);
    if (iconManDir) {
        unk340 = iconManDir->Find<Character>("iconman", false);
        if (unk340) {
            RndAnimatable *anim = unk340->Find<RndAnimatable>("outline.anim", false);
            if (anim)
                anim->SetFrame(1, 1);
        }
        unk354 = iconManDir->Find<RndTexRenderer>("iconman.rndtex", false);
    }
    delete mPoseFatalities;
    mPoseFatalities = Hmx::Object::New<PoseFatalities>();
}

RndPropAnim *HamDirector::SongAnim(int playerIndex) {
    if (unk5c[kDifficultyEasy]) {
        MILO_ASSERT((0) <= (playerIndex) && (playerIndex) < (2), 0x620);
        if (TheHamProvider->Property("merge_moves", true)->Int()) {
            return playerIndex == 0 ? unka0 : unkb4;
        } else {
            HamPlayerData *hpd = TheGameData->Player(playerIndex);
            return SongAnimByDifficulty(hpd->GetDifficulty());
        }
    } else
        return nullptr;
}
