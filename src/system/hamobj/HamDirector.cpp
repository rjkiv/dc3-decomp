#include "hamobj/HamDirector.h"
#include "Difficulty.h"
#include "MoveMgr.h"
#include "PoseFatalities.h"
#include "SongCollision.h"
#include "SongUtl.h"
#include "char/CharBone.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharForeTwist.h"
#include "char/CharLipSync.h"
#include "char/CharNeckTwist.h"
#include "char/CharPollable.h"
#include "char/CharUpperTwist.h"
#include "char/CharUtl.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "flow/Flow.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/ClipPlayer.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/HamRibbon.h"
#include "hamobj/HamSkeletonConverter.h"
#include "hamobj/HamVisDir.h"
#include "hamobj/HamWardrobe.h"
#include "hamobj/TransConstraint.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Overlay.h"
#include "rndobj/Poll.h"
#include "rndobj/PostProc.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "rndobj/Trans.h"
#include "utl/FakeSongMgr.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "utl/TextStream.h"
#include "utl/TimeConversion.h"
#include "world/CameraManager.h"
#include "world/Dir.h"
#include <cctype>

HamDirector *TheHamDirector;
OfflineCallback gOfflineCallback;

float FrameToBeat(float frame) { return SecondsToBeat(frame * 0.033333335f); }
float BeatToFrame(float beat) { return BeatToSeconds(beat) * 30.0f; }

ObjectDir *OfflineCallback::SongMainDir() {
    MILO_ASSERT(TheHamDirector, 0x1137);
    return TheHamDirector->GetWorld();
}

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
      mBlendDebug(0), mBackupDancers((HamBackupDancers)0), mClipDir(this), mMoveDir(this),
      mNoTransitions(0), mCollisionChecks(1), mLoadedNewSong(1), mPoseFatalities(0),
      unk33c(RandomInt(0, 2)), unk33d(0), mIconManChar(this), mIconManTex(this),
      unk369(0), mOfflineSong(0) {
    static DataNode &n = DataVariable("hamdirector");
    n = this;
    TheHamDirector = this;
    mDirCutKeys.reserve(100);
}

HamDirector::~HamDirector() {
    delete mOfflineSong;
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

BEGIN_COPYS(HamDirector)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(HamDirector)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPracticeStart)
        COPY_MEMBER(mPracticeEnd)
        COPY_MEMBER(mBlendDebug)
        COPY_MEMBER(mNoTransitions)
        COPY_MEMBER(mCollisionChecks)
        COPY_MEMBER(mStartLoopMargin)
        COPY_MEMBER(mEndLoopMargin)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(HamDirector)
    LOAD_REVS(bs)
    ASSERT_REVS(9, 0)
    Hmx::Object::Load(bs);
    RndPollable::Load(bs);
    RndDrawable::Load(bs);
    if (d.rev < 7) {
        Symbol s;
        d >> s;
    }
    if (d.rev < 8) {
        int x;
        d >> x;
    }
    if (d.rev > 2) {
        d >> mPracticeStart;
        d >> mPracticeEnd;
    }
    if (d.rev > 3) {
        d >> mBlendDebug;
    }
    if (d.rev > 4) {
        d >> mNoTransitions;
    }
    if (d.rev > 8) {
        d >> mCollisionChecks;
    }
    if (d.rev > 5) {
        d >> mStartLoopMargin;
        d >> mEndLoopMargin;
    }
END_LOADS

void HamDirector::Enter() {
    RndPollable::Enter();
    if (mMerger) {
        mExcitement = 3;
        mNumPlayersFailed = 0;
        unk29c = -kHugeFloat;
        unk2a8 = -kHugeFloat;
        mShot = "";
        mCurShot = nullptr;
        unk14c = true;
        unk33d = false;
        mWorldPostProc = GetWorld()->Find<RndPostProc>("world.pp", true);
        RndPostProc *start = GetWorld()->Find<RndPostProc>("world_start.pp", true);
        if (start) {
            mWorldPostProc->Copy(start, kCopyDeep);
        }
        mWorldPostProc->Select();
        unk1a8 = mWorldPostProc;
        unk1bc = mWorldPostProc;
        unk1d0 = 0;
        mCamPostProc = nullptr;
        mForcePostProc = nullptr;
        mForcePostProcBlend = 0;
        mForcePostProcBlendRate = 1;
        unk1d8 = nullptr;
        mVisualizerPostProc =
            mVisualizer ? mVisualizer->Find<RndPostProc>("viz_start.pp", false) : nullptr;
        unk2e4 = -kHugeFloat;
        mDisabled = false;
        unk368 = false;
        static Message msg("set_force_postproc_no_blend", "performance_high");
        HandleType(msg);
        if (TheHamWardrobe) {
            TheHamWardrobe->ClearCrowd();
        }
        if (mVenue) {
            VenueEnter(mVenue);
        }
        Initialize();
        RndPropAnim *anim = SongAnim(0);
        if (anim) {
            anim->StartAnim();
            anim->SetFrame(-kHugeFloat, 1);
        }
        mDisablePicking = false;
        mNextShot = nullptr;
        mPlayerFreestyle = false;
        SyncScene();
        PlayIntroShot();
        unk1d4 = 0;
        mPlayerFreestylePaused = false;
        if (mVisualizer) {
            mVisualizer->SetShowing(false);
        }
        if (TheHamWardrobe) {
            TheHamWardrobe->PlayCrowdAnimation("realtime_idle", 2, true);
        }
    }
}

void HamDirector::Exit() {
    RndPollable::Exit();
    RndPropAnim *anim = SongAnim(0);
    if (anim) {
        anim->EndAnim();
    }
    if (mVenue) {
        mVenue->Exit();
    }
}

void HamDirector::ListPollChildren(std::list<RndPollable *> &polls) const {
    if (mVenue) {
        polls.push_back(mVenue);
    }
}

void HamDirector::DrawShowing() {
    static Symbol hide_venue("hide_venue");
    bool hide = TheHamProvider->Property(hide_venue, true)->Int();
    if (mVenue && !hide) {
        mVenue->DrawShowing();
    }
}

void HamDirector::ListDrawChildren(std::list<RndDrawable *> &draws) {
    if (mVenue) {
        draws.push_back(mVenue);
    }
}

void HamDirector::CollideList(const Segment &s, std::list<Collision> &colls) {
    if (mVenue) {
        mVenue->CollideList(s, colls);
    }
    RndDrawable::CollideList(s, colls);
}

DataNode HamDirector::OnSaveSong(DataArray *) { return 0; }
DataNode HamDirector::OnSaveFaceAnims(DataArray *) { return 0; }
DataNode HamDirector::OnFileMerged(DataArray *) { return 0; }

void HamDirector::ForceScene(Symbol s) {
    unk13c = s;
    unk138 = gNullStr;
}

__forceinline void HamDirector::ForceMiniVenue(Symbol s) {
    Symbol idk(gNullStr);
    unk138 = idk;
    unk138 = s;
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
        mCharsShowing[i] = false;
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

DataNode HamDirector::OnToggleCamshotFlag() { return unk33c = !unk33c; }

DataNode HamDirector::OnLoadSong(DataArray *a) {
    FilePathTracker tracker(FileRoot());
    MILO_ASSERT(TheGameData, 0xC1D);
    for (int i = 0; i < 2; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        MILO_ASSERT(hpd, 0xC21);
        unk2fc[i] = hpd->Crew();
        unk2f4[i] = hpd->CharacterOutfit(unk2fc[i]);
    }
    int i3 = a->Int(3);
    bool i4 = a->Int(4);
    bool b5 = a->Int(5);
    String str(a->Str(2));
    int dancers = a->Int(6);
    MILO_ASSERT(dancers >= 0 && dancers < kBackupDancersNumTypes, 0xC2E);
    mBackupDancers = (HamBackupDancers)dancers;
    mLoadedNewSong = true;
    if (mMerger && !str.empty()) {
        const char *speed;
        if (i3 < 113)
            speed = "slow";
        else if (i3 < 136)
            speed = "medium";
        else
            speed = "fast";
        unk330 = speed;
        TheGameData->SetSong(FileGetBase(str.c_str()));
        mMerger->Select("song", str.c_str(), true);
        if (i4) {
            mMerger->StartLoad(b5);
            if (mVenue) {
                FileMerger *extras = mVenue->Find<FileMerger>("extras.fm", false);
                if (extras) {
                    extras->StartLoad(b5);
                }
            }
        }
    }
    return 0;
}

DataNode HamDirector::OnPostProcs(DataArray *a) {
    DataNode *var1 = a->Var(2);
    DataNode *var2 = a->Var(3);
    DataNode *var3 = a->Var(4);
    DataNode *var4 = a->Var(5);
    *var1 = mWorldPostProc.Ptr();
    *var2 = mCamPostProc.Ptr();
    *var3 = mForcePostProc.Ptr();
    *var4 = mVisualizerPostProc.Ptr();
    return 0;
}

DataNode HamDirector::OnShotOver(DataArray *a) {
    if (strneq(a->Obj<HamCamShot>(2)->Category().Str(), "dc_", 3)) {
        unk140 = true;
    }
    unk29c = -kHugeFloat;
    return 0;
}

DataNode HamDirector::OnListPossibleMoves() {
    if (!TheMoveMgr) {
        MoveMgr::Init("../meta/move_data.dta");
    }
    DataArray *moveArr = new DataArray(0);
    for (std::map<Symbol, MoveParent *>::const_iterator it =
             TheMoveMgr->MoveParents().begin();
         it != TheMoveMgr->MoveParents().end();
         ++it) {
        moveArr->Insert(moveArr->Size(), it->first);
    }
    moveArr->SortNodes(0);
    DataNode ret(moveArr);
    moveArr->Release();
    return ret;
}

DataNode HamDirector::OnListPossibleVariants() {
    if (!TheMoveMgr) {
        MoveMgr::Init("../meta/move_data.dta");
    }
    DataArray *moveArr = new DataArray(0);
    // FIXME: should get MoveMgr's unk104 member
    for (std::map<Symbol, MoveVariant *>::const_iterator it =
             TheMoveMgr->MoveVariants().begin();
         it != TheMoveMgr->MoveVariants().end();
         ++it) {
        moveArr->Insert(moveArr->Size(), it->first);
    }
    moveArr->SortNodes(0);
    DataNode ret(moveArr);
    moveArr->Release();
    return ret;
}

namespace {
    const char *gGrooveName = "groove";
}

DataNode HamDirector::PracticeList(Difficulty d) {
    DataArray *arr = new DataArray(0);
    arr->Insert(0, Symbol());
    PropKeys *keys = GetPropKeys(d, "practice");
    if (keys) {
        Keys<Symbol, Symbol> *symKeys = keys->AsSymbolKeys();
        for (int i = 0; i < symKeys->size(); i++) {
            arr->Insert(arr->Size(), (*symKeys)[i].value);
        }
    }
    arr->Insert(arr->Size(), Symbol(gGrooveName));
    DataNode ret(arr);
    arr->Release();
    return ret;
}

DataNode HamDirector::OnCycleShot(DataArray *a) {
    if (mVenue) {
        mNextShot =
            dynamic_cast<HamCamShot *>(mVenue->GetCameraManager()->ShotAfter(mCurShot));
        mDisablePicking = mNextShot;
    }
    return 0;
}

DataNode HamDirector::OnForceShot(DataArray *a) {
    ForceShot(a->Str(2));
    return 0;
}

void GetVenuePath(FilePath &path, const char *cc) {
    FilePathTracker tracker(FileRoot());
    path.Set(FilePath::Root().c_str(), "");
    if (*cc == '\0')
        return;
    else {
        const char *milo = MakeString("world/%s/%s.milo", cc, cc);
        path.Set(FilePath::Root().c_str(), milo);
    }
}

DataNode HamDirector::OnFileLoaded(DataArray *a) {
    static Symbol song("song");
    static Symbol venue("venue");
    static Symbol viz("viz");
    static Symbol game_hud("game_hud");
    Symbol sym = a->Sym(2);
    if (sym != game_hud || mMerger) {
        unk25a = mMerger->AsyncLoad();
        if (sym == song) {
            if (!TheGameData->Venue().Null()) {
                if (TheHamWardrobe) {
                    TheHamWardrobe->LoadCharacters(
                        unk2f4[0],
                        unk2f4[1],
                        unk2fc[0],
                        unk2fc[1],
                        mBackupDancers,
                        unk330,
                        TheGameData->Venue().Str(),
                        unk25a
                    );
                }
                FilePath path;
                {
                    FilePathTracker tracker(FileRoot());
                    path.Set(FilePath::Root().c_str(), "ui/visualizer/visualizer.milo");
                }
                mMerger->Select("viz", path, false);
                GetVenuePath(path, TheGameData->Venue().Str());
                mMerger->Select("venue", path, false);
                if (mGameModeMerger) {
                    static Message load_game_hud("load_game_hud", 0, 0, 0, 0);
                    mGameModeMerger->HandleType(load_game_hud);
                    mGameModeMerger->StartLoad(unk25a);
                }
            }
            mMerger->StartLoad(unk25a);
        } else {
            ObjectDir *dir = a->Obj<ObjectDir>(3);
            if (sym == venue && dir) {
                mVenue = dynamic_cast<WorldDir *>(dir);
            } else if (sym == viz && dir) {
                mVisualizer = dynamic_cast<HamVisDir *>(dir);
            }
        }
    }
    return 0;
}

DataNode HamDirector::OnPostProcInterp(DataArray *a) {
    unk1a8 = a->Obj<RndPostProc>(2);
    unk1bc = a->Obj<RndPostProc>(3);
    unk1d0 = a->Float(4);
    return 0;
}

DataNode HamDirector::OnPracticeBeats(DataArray *a) {
    Key<Symbol> *key1;
    Key<Symbol> *key2;
    if (!GetPracticeFrames(key1, key2)) {
        return 0;
    } else {
        *a->Var(2) = FrameToBeat(key1->frame);
        *a->Var(3) = FrameToBeat(key2->frame);
        return 1;
    }
}

DataNode HamDirector::OnClipList(DataArray *a) {
    HamPlayerData *data = TheGameData->Player(0);
    if (data->GetDifficulty() == kDifficultyExpert) {
        return ObjectList(
            mMerger->Dir()->Find<ObjectDir>("clips", true), "CharClip", false
        );
    } else {
        DataNode list = PracticeList(kDifficultyExpert);
        DataArray *arr = list.Array();
        arr->SortNodes(0);
        return list;
    }
}

DataNode HamDirector::OnSetDircut(DataArray *a) {
    if (mVenue && !ShotsDisabled()) {
        Symbol sym = a->Sym(2);
        std::vector<CameraManager::PropertyFilter> filters;
        if (a->Size() > 3) {
            const DataNode &node = a->Evaluate(3);
            DataArray *arr;
            if (node.Type() == kDataInt && node.Int() != 0) {
                arr = nullptr;
            } else {
                MILO_ASSERT(node.Type() == kDataArray, 0xE74);
                arr = node.Array();
            }
            AddNumPlayers(filters, arr);
        }
        SetDircut(sym, filters);
    }
    return mNextShot;
}

HamCharacter *HamDirector::GetCharacter(int i) const {
    if (TheHamWardrobe) {
        return TheHamWardrobe->GetCharacter(i);
    } else
        return nullptr;
}

HamCharacter *HamDirector::GetBackup(int i) {
    if (TheHamWardrobe) {
        return TheHamWardrobe->GetBackup(i);
    } else
        return nullptr;
}

void HamDirector::ChangePlayerCharacter(int i1, Symbol s1, Symbol s2, Symbol s3) {
    HamPlayerData *hpd = TheGameData->Player(i1);
    hpd->SetCharacter(s1);
    hpd->SetCharacterOutfit(s3);
    unk2f4[i1] = s1;
    unk2fc[i1] = s2;
    TheHamWardrobe->LoadCharacters(
        unk2f4[0],
        unk2f4[1],
        unk2fc[0],
        unk2fc[1],
        mBackupDancers,
        unk330,
        TheGameData->Venue().Str(),
        true
    );
}

void HamDirector::SetMainFaceOverrideClip(Symbol s) {
    HamCharacter *hChar = GetCharacter(0);
    if (hChar) {
        String str(s);
        hChar->SetFaceOverrideClip(str.c_str(), true);
    }
}

Symbol HamDirector::GetMainFaceOverrideClip() const {
    HamCharacter *hChar = GetCharacter(0);
    if (hChar) {
        return hChar->GetFaceOverrideClip();
    } else
        return Symbol();
}

void HamDirector::SetMainFaceOverrideWeight(float wt) {
    HamCharacter *hChar = GetCharacter(0);
    if (hChar)
        hChar->SetFaceOverrideWeight(wt);
}

float HamDirector::GetMainFaceOverrideWeight() {
    HamCharacter *hChar = GetCharacter(0);
    if (hChar) {
        return hChar->GetFaceOverrideWeight();
    } else
        return 0;
}

void HamDirector::TeleportChars() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar)
            hChar->SetTeleport(true);
    }
}

bool HamDirector::SongAnimation() {
    bool ret = false;
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar && hChar->SongAnimation() > -1) {
            ret = true;
            break;
        }
    }
    return ret;
}

void HamDirector::ResyncFaceDrivers() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar)
            hChar->ResyncLipSync(nullptr);
    }
    int i = 0;
    while (true) {
        HamCharacter *hChar = GetBackup(i++);
        if (!hChar)
            break;
        else {
            hChar->ResyncLipSync(nullptr);
        }
    }
}

void HamDirector::PlayCharBaseVisemes() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar)
            hChar->PlayBaseViseme();
    }
    int i = 0;
    while (true) {
        HamCharacter *hChar = GetBackup(i++);
        if (!hChar)
            break;
        else {
            hChar->PlayBaseViseme();
        }
    }
}

void HamDirector::DisableFacialAnimation() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar)
            hChar->DisableFacialAnimation();
    }
    int i = 0;
    while (true) {
        HamCharacter *hChar = GetBackup(i++);
        if (!hChar)
            break;
        else {
            hChar->DisableFacialAnimation();
        }
    }
}

void HamDirector::ResetFacialAnimation() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar)
            hChar->ResetFacialAnimation();
    }
    int i = 0;
    while (true) {
        HamCharacter *hChar = GetBackup(i++);
        if (!hChar)
            break;
        else {
            hChar->ResetFacialAnimation();
        }
    }
}

void HamDirector::SetLipsyncOffsets(float f1) {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar) {
            hChar->ResetFaceOverrideBlending();
            hChar->SetLipsyncOffset(f1);
        }
    }
    int i = 0;
    while (true) {
        HamCharacter *hChar = GetBackup(i++);
        if (!hChar)
            break;
        else {
            hChar->SetLipsyncOffset(f1);
        }
    }
}

void HamDirector::BlendInFaceOverrides(float f1) {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar) {
            hChar->BlendInFaceOverrides(f1);
        }
    }
}

void HamDirector::BlendOutFaceOverrides(float f1) {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hChar = GetCharacter(i);
        if (hChar) {
            hChar->BlendOutFaceOverrides(f1);
        }
    }
}

bool HamDirector::ShotsDisabled() {
    if (!mDisablePicking) {
        if (GetWorld() && GetWorld()->GetCameraManager()->HasFreeCam()) {
            return true;
        }
        if (!mPlayerFreestyle || mFreestyleEnabled) {
            return false;
        }
    }
    return true;
}

void HamDirector::SyncScene() {
    unk14c = false;
    if (!ShotsDisabled() && mVenue) {
        SetNewWorld();
    }
}

void HamDirector::RestoreBackups() {
    if (unk254) {
        if (mPlayer0Char) {
            mPlayer0Char->SetShowing(mCharsShowing[0]);
        }
        if (mPlayer1Char) {
            mPlayer1Char->SetShowing(mCharsShowing[1]);
        }
        mPlayer1Char->SetShowing(mCharsShowing[1]);
        if (mBackup0Char) {
            mBackup0Char->SetShowing(mCharsShowing[2]);
        }
        if (mBackup1Char) {
            mBackup1Char->SetShowing(mCharsShowing[3]);
        }
        for (int i = 0; i < 4; i++)
            mCharsShowing[i] = false;
        unk254 = false;
    }
}

ObjectDir *HamDirector::GetDifficultyProxy(Difficulty d) {
    Difficulty d_legacy = LegacyDifficulty(d);
    WorldDir *dir = GetWorld();
    if (dir) {
        Symbol sym = DifficultyToSym(d_legacy);
        return dir->Find<ObjectDir>(sym.Str(), false);
    } else
        return nullptr;
}

RndPropAnim *HamDirector::GetPropAnim(Difficulty d, const char *name, bool warn) {
    RndPropAnim *anim = nullptr;
    ObjectDir *proxy = GetDifficultyProxy(d);
    if (proxy) {
        anim = proxy->Find<RndPropAnim>(name, false);
        if (warn && !anim) {
            MILO_NOTIFY("%s has no PropAnim \"%s\"", PathName(proxy), name);
        }
    }
    return anim;
}

void HamDirector::EnableFacialAnimation() {
    for (int i = 0; i < 2; i++) {
        CharLipSync *lipsync = nullptr;
        ObjectDir *proxy = GetDifficultyProxy(TheGameData->Player(i)->GetDifficulty());
        if (proxy) {
            lipsync = proxy->Find<CharLipSync>("dancer_face.lipsync", false);
        }
        HamCharacter *hChar = GetCharacter(i);
        if (hChar) {
            hChar->EnableFacialAnimation(lipsync, 0);
        }
        if (i == 0) {
            int j = 0;
            while (true) {
                HamCharacter *hChar = GetBackup(j++);
                if (!hChar)
                    break;
                else {
                    hChar->EnableFacialAnimation(lipsync, 0);
                }
            }
        }
    }
}

Symbol HamDirector::ClosestMove() {
    Symbol out = unk2f0;
    char buf[256];
    Difficulty playerDiff = TheGameData->Player(0)->GetDifficulty();
    if (playerDiff != kDifficultyExpert) {
        if (mSongAnims[playerDiff]) {
            SymbolKeys *keys =
                dynamic_cast<SymbolKeys *>(GetPropKeys(playerDiff, "move"));
            if (keys) {
                Key<Symbol> *nearest =
                    keys->KeyNearest(mSongAnims[playerDiff]->GetFrame());
                if (nearest) {
                    strcpy(buf, nearest->value.Str());
                    char *chr = strchr(buf, 0x2E);
                    if (chr)
                        *chr = '\0';
                    DataNode list = PracticeList(kDifficultyExpert);
                    DataArray *listArr = list.Array();
                    int i17 = -1;
                    for (int i = 0; i < listArr->Size(); i++) {
                        const char *str = listArr->Str(i);
                        int numlower = 0;
                        if (*str) {
                            char *p = (char *)str;
                            while (p[buf - str]) {
                                if (tolower(p[buf - str]) != tolower(*p))
                                    break;
                                numlower++;
                            }
                        }
                    }
                }
            }
        }
    }
    return out;
}

bool HamDirector::IsWorldLoaded() const {
    return mVenue && mMerger && !mMerger->HasPendingFiles() && mMoveMerger
        && !mMoveMerger->HasPendingFiles();
}

void HamDirector::CheckBeginFatal(int i1, HamMove *move, int i3) {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol dance_battle("dance_battle");
    if (i3 <= 1) {
        if (move->IsFinalPose() && !mPoseFatalities->InFatality(i1)) {
            if (TheHamProvider->Property(gameplay_mode, true)->Sym() == dance_battle) {
                mPoseFatalities->ActivateFatal(i1);
            }
        }
    }
}

void HamDirector::UpdatePostProcOverlay(
    const char *cc, const RndPostProc *p1, const RndPostProc *p2, float f4
) {
    RndOverlay *ppOverlay = RndOverlay::Find("postproc", true);
    static const RndPostProc *sPostProcA;
    static const RndPostProc *sPostProcB;
    float sPostProcBlend = -99;
    if (!ppOverlay->Showing())
        return;
    if (p1 == sPostProcA && p2 == sPostProcB && f4 == sPostProcBlend)
        return;
    static int sHamDirID = 0;
    sHamDirID = (sHamDirID + 1) % 100;
    TextStream *reflect = TheDebug.SetReflect(ppOverlay);
    if (p1 && p2) {
        MILO_LOG("%03d:HAMDIR Post Proc A %s\n", sHamDirID, p1->Name());
        MILO_LOG("%03d:HAMDIR Post Proc B %s\n", sHamDirID, p2->Name());
    } else if (p1 && !p2) {
        MILO_LOG("%03d:HAMDIR Post Proc %s is not blended\n", sHamDirID, p1->Name());
    } else if (!p1 && p2) {
        MILO_LOG("%03d:HAMDIR Post Proc B %s\n", sHamDirID, p2->Name());
    }
    MILO_LOG(
        "           PostProc set by %s, blend is %.2f%%\n", cc ? cc : "", f4 * 100.0f
    );
    sPostProcA = p1;
    sPostProcB = p2;
    sPostProcBlend = f4;
    TheDebug.SetReflect(reflect);
}

bool HamDirector::IsMoveMergerFinished() const {
    return mMoveMerger && !mMoveMerger->HasPendingFiles();
}

void HamDirector::SetNewWorld() {
    MILO_ASSERT(mVenue, 0x7D5);
    if (TheHamWardrobe) {
        TheHamWardrobe->SetDir(mVenue);
    }
    unk140 = true;
    GetWorld()->SetSphere(mVenue->GetSphere());
}

void HamDirector::HideBackups(bool b1, bool b2) {
    mCharsShowing[0] = mPlayer0Char && mPlayer0Char->Showing();
    mCharsShowing[1] = mPlayer1Char && mPlayer1Char->Showing();
    if (b1 ^ b2) {
        if (b1) {
            mPlayer1Char->SetShowing(false);
        } else {
            mPlayer0Char->SetShowing(false);
        }
    }
    mCharsShowing[2] = mBackup0Char && mBackup0Char->Showing();
    if (mBackup0Char) {
        mBackup0Char->SetShowing(false);
    }
    mCharsShowing[3] = mBackup1Char && mBackup1Char->Showing();
    if (mBackup1Char) {
        mBackup1Char->SetShowing(false);
    }
    unk254 = true;
}

void HamDirector::LoadCrew(Symbol s1, Symbol s2) {
    char buffer[128];
    Symbol symbols[2] = { s1, s2 };
    Symbol mind_control("mind_control");
    Symbol gameplaySym = TheHamProvider->Property("gameplay_mode", true)->Sym();
    for (int i = 0; i < 2; i++) {
        HamPlayerData *hpd = TheGameData->Player(i);
        MILO_ASSERT(hpd, 0x98B);
        unk2fc[i] = symbols[i];
        strcpy(buffer, hpd->CharacterOutfit(unk2fc[i]).Str());
        if (strstr(buffer, "lima") || strstr(buffer, "rasa")) {
            buffer[strlen(buffer)] = '0';
            buffer[strlen(buffer)] = gameplaySym == mind_control ? '6' : '5';
        } else {
            buffer[strlen(buffer)] = '0';
            buffer[strlen(buffer)] = '4';
        }
        unk2f4[i] = buffer;
    }
    TheHamWardrobe->LoadCharacters(
        unk2f4[0],
        unk2f4[1],
        unk2fc[0],
        unk2fc[1],
        mBackupDancers,
        unk330,
        TheGameData->Venue().Str(),
        true
    );
}

void HamDirector::OfflineLoadSong(Symbol song) {
    MILO_ASSERT(TheFakeSongMgr, 0x1154);
    MILO_ASSERT(mOfflineSong, 0x1155);
    mOfflineSong->SetSong(song);
}

DataNode HamDirector::OnClipAnnotate(DataArray *a) {
    ClipPlayer player;
    if (player.Init(TheGameData->Player(0)->GetDifficulty())) {
        float frame;
        RndPropAnim *anim = a->Obj<RndPropAnim>(2);
        anim->GetKeys(this, a->Array(3))->FrameFromIndex(a->Int(4), frame);
        return player.AnnotateClip(frame);
    } else {
        return 0;
    }
}

DataNode HamDirector::OnPracticeAnnotate(DataArray *a) {
    ClipPlayer player;
    if (player.Init(TheGameData->Player(0)->GetDifficulty())) {
        return player.AnnotatePractice();
    } else {
        return 0;
    }
}

DataNode HamDirector::OnToggleDebugInterests(DataArray *a) {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hc = GetCharacter(i);
        if (hc) {
            hc->SetDebugDrawInterestObjects(!hc->DebugDrawInterestObjects());
        }
    }
    return 0;
}

DataNode HamDirector::OnToggleCamCharacterSkeleton(DataArray *a) {
    for (int i = 0; i < 2; i++) {
        HamCharacter *hc = GetCharacter(i);
        if (hc) {
            hc->SetUseCameraSkeleton(!hc->UseCameraSkeleton());
        }
    }
    return 0;
}

DataNode HamDirector::OnBlendInFaceClip(DataArray *a) {
    Symbol s1 = a->Sym(2);
    float f1 = a->Float(3);
    float f2 = a->Float(4);
    HamCharacter *hc = GetCharacter(0);
    if (hc) {
        hc->BlendInFaceOverrideClip(s1, f1, f2);
    }
    return 0;
}

void HamDirector::InitOffline() {
    if (!mOfflineSong) {
        // HamSong::mPreferStreaming = true;
        mOfflineSong = dynamic_cast<Song *>(Hmx::Object::NewObject("Song"));
        MILO_ASSERT(mOfflineSong, 0x114B);
        DataVariable("tool_song") = mOfflineSong;
        Song::sCallback = &gOfflineCallback;
    }
}

bool HamDirector::GetPracticeFrames(Key<Symbol> *&k1, Key<Symbol> *&k2) {
    if (!mPracticeStart.Null() && !mPracticeEnd.Null()) {
        PropKeys *propKeys =
            GetPropKeys(TheGameData->Player(0)->GetDifficulty(), "practice");
        if (propKeys) {
            Keys<Symbol, Symbol> *keys = propKeys->AsSymbolKeys();
            int numKeys = keys->size();
            int startIdx = 0;
            for (; startIdx < numKeys; startIdx++) {
                if (mPracticeStart == (*keys)[startIdx].value)
                    goto next;
            }
            startIdx = -1;
        next:
            int endIdx = 0;
            for (; endIdx < numKeys; endIdx++) {
                if (mPracticeEnd == (*keys)[endIdx].value)
                    goto end;
            }
            endIdx = -1;
        end:
            if (startIdx < endIdx && startIdx != -1 && endIdx != -1) {
                k1 = &(*keys)[startIdx];
                k2 = &(*keys)[endIdx];
                return true;
            }
        }
    }
    return false;
}

void HamDirector::SetPhraseMetersFlipped(bool b1) {
    if (unk369 != b1) {
        unk369 = b1;
        static Symbol spotlight_constraint("spotlight_constraint");
        WorldDir *venue = TheHamDirector->GetVenueWorld();
        if (venue) {
            RndTransformable *p0 = venue->Find<RndTransformable>("player0", true);
            RndTransformable *p1 = venue->Find<RndTransformable>("player1", true);
            TransConstraint *c0 =
                venue->Find<TransConstraint>("TransConstraint.tc", true);
            TransConstraint *c1 =
                venue->Find<TransConstraint>("TransConstraint1.tc", true);
            if (b1) {
                p0->SetProperty(spotlight_constraint, c1);
                p1->SetProperty(spotlight_constraint, c0);
                c0->SetParent(p1);
                c0->SnapToParent();
                c1->SetParent(p0);
                c1->SnapToParent();
            } else {
                p0->SetProperty(spotlight_constraint, c0);
                p1->SetProperty(spotlight_constraint, c1);
                c0->SetParent(p0);
                c0->SnapToParent();
                c1->SetParent(p1);
                c1->SnapToParent();
            }
        }
    }
}

void HamDirector::SetPlayerSpotlightsEnabled(bool b1) {
    WorldDir *venue = TheHamDirector->GetVenueWorld();
    if (venue) {
        RndTransformable *players[2] = { venue->Find<RndTransformable>("player0", true),
                                         venue->Find<RndTransformable>("player1", true) };
        TransConstraint *constraints[2] = {
            venue->Find<TransConstraint>("TransConstraint.tc", true),
            venue->Find<TransConstraint>("TransConstraint1.tc", true)
        };
        HamPhraseMeter *phraseMeters[2] = {
            venue->Find<HamPhraseMeter>("phrase_meter0", true),
            venue->Find<HamPhraseMeter>("phrase_meter1", true)
        };
        RndDrawable *moveFeedbacks[2] = {
            venue->Find<RndDrawable>("move_feedback0", true),
            venue->Find<RndDrawable>("move_feedback1", true)
        };
        for (int i = 0; i < 2; i++) {
            if (b1) {
                constraints[i]->SetParent(players[i]);
                constraints[i]->SnapToParent();
                // constraints[i]->unk52 = true;
                constraints[i]->SnapToParent();
                phraseMeters[i]->SetShowing(true);
                moveFeedbacks[i]->SetShowing(true);
            } else {
                constraints[i]->SetParent(nullptr);
                constraints[i]->SnapToParent();
                // constraints[i]->unk52 = false;
                Vector3 v(-1000000.0f, -1000000.0f, 0);
                phraseMeters[i]->SetLocalPos(v);
                phraseMeters[i]->SetShowing(false);
                moveFeedbacks[i]->SetShowing(false);
            }
        }
    }
}

bool HamDirector::InPracticeMode() {
    Key<Symbol> *start;
    Key<Symbol> *end;
    return GetPracticeFrames(start, end);
}

void HamDirector::PoseIconMan(
    CharClip *clip1, float f2, RndTex *tex, bool b4, CharClip *clip2, float f5, float f6
) {
    if (clip1) {
        CharBonesMeshes meshes;
        meshes.SetName("preview_anim", mIconManChar);
        clip1->StuffBones(meshes);
        meshes.Zero();
        if (clip2) {
            clip1->ScaleAdd(meshes, 1 - f6, f2, 0);
            clip2->ScaleAdd(meshes, f6, f5, 0);
        } else {
            clip1->ScaleAdd(meshes, 1, f2, 0);
        }
        meshes.PoseMeshes();
        if (b4) {
            RndTransformable *pelvis = CharUtlFindBoneTrans("bone_pelvis", meshes.Dir());
            void *rotzPtr = meshes.FindPtr("bone_facing.rotz");
            void *posPtr = meshes.FindPtr("bone_facing.pos");
            if (pelvis && posPtr && rotzPtr) {
                float *rot = (float *)rotzPtr;
                Vector3 *pos = (Vector3 *)posPtr;
                Transform &pelvisXfm = pelvis->DirtyLocalXfm();
                RotateAboutZ(pelvisXfm.m, *rot, pelvisXfm.m);
                RotateAboutZ(pelvisXfm.v, *rot, pelvisXfm.v);
                Normalize(pelvisXfm.m, pelvisXfm.m);
                pelvisXfm.v += *pos;
            }
            CharBoneDir *resourceDir = clip1->GetResource();
            for (ObjDirItr<CharBone> it(resourceDir, false); it != nullptr; ++it) {
                if (it->BakeOutAsTopLevel()) {
                    String name(it->Name());
                    if (name.find(".cb") != FixedString::npos) {
                        name = name.substr(0, name.length() - 3);
                    }
                    RndTransformable *curTrans =
                        CharUtlFindBoneTrans(name.c_str(), meshes.Dir());
                    if (curTrans && posPtr && rotzPtr) {
                        float *rot = (float *)rotzPtr;
                        Vector3 *pos = (Vector3 *)posPtr;
                        Transform &curXfm = curTrans->DirtyLocalXfm();
                        RotateAboutZ(curXfm.m, *rot, curXfm.m);
                        RotateAboutZ(curXfm.v, *rot, curXfm.v);
                        Normalize(curXfm.m, curXfm.m);
                        curXfm.v += *pos;
                    }
                }
            }
        }
        for (ObjDirItr<CharPollable> it(mIconManChar, true); it != nullptr; ++it) {
            if (dynamic_cast<CharForeTwist *>(&*it)
                || dynamic_cast<CharUpperTwist *>(&*it)
                || dynamic_cast<CharNeckTwist *>(&*it)) {
                it->Poll();
            }
        }
    }
    mIconManTex->SetShowing(true);
    mIconManTex->SetOutputTexture(tex);
    mIconManTex->DrawShowing();
    mIconManTex->SetShowing(false);
}

void HamDirector::PoseIconMan(const BaseSkeleton *skeleton, RndTex *tex) {
    HamCharacter *iconMan = dynamic_cast<HamCharacter *>(mIconManChar.Ptr());
    MILO_ASSERT(iconMan, 0x52D);
    for (ObjDirItr<HamRibbon> it(iconMan, true); it != nullptr; ++it) {
        it->Reset();
    }
    iconMan->SetUseCameraSkeleton(true);
    HamSkeletonConverter *skelCvt =
        iconMan->Find<HamSkeletonConverter>("HamSkeletonConverter.cvt", true);
    MILO_ASSERT(skelCvt, 0x535);
    skelCvt->Set(skeleton);
    iconMan->SetPollWhenHidden(true);
    iconMan->Poll();
    iconMan->SetPollWhenHidden(false);
    mIconManTex->SetShowing(true);
    mIconManTex->SetOutputTexture(tex);
    mIconManTex->DrawShowing();
    mIconManTex->SetShowing(false);
    iconMan->SetUseCameraSkeleton(false);
}

DataNode HamDirector::OnGetDancerVisemes(DataArray *a) {
    std::list<Symbol> visemes;
    HamCharacter *hc = GetCharacter(0);
    if (hc) {
        ObjectDir *visemeDir = hc->Find<ObjectDir>("viseme", false);
        if (visemeDir) {
            for (ObjDirItr<CharClip> it(visemeDir, false); it != nullptr; ++it) {
                visemes.push_back(it->Name());
            }
        }
    }
    DataArray *visemeArr = new DataArray(visemes.size() + 1);
    visemeArr->Node(0) = Symbol();
    int idx = 1;
    FOREACH (it, visemes) {
        visemeArr->Node(idx) = *it;
        ++idx;
    }
    DataNode ret(visemeArr);
    visemeArr->Release();
    return ret;
}

void HamDirector::CleanOriginalMoveData() {
    for (ObjDirItr<CharClip> it(mClipDir, true); it != nullptr; ++it) {
        MILO_LOG("[HamDirector::CleanOriginalMoveData] Removing %s ...\n", it->Name());
        delete it;
    }
    for (ObjDirItr<HamMove> it(mMoveDir, true); it != nullptr; ++it) {
        RndTex *smallTex = it->SmallTex();
        RndTex *regTex = it->Tex();
        if (smallTex && smallTex != regTex) {
            MILO_LOG(
                "[HamDirector::CleanOriginalMoveData] Removing %s ...\n", smallTex->Name()
            );
            delete smallTex;
        }
        if (regTex) {
            MILO_LOG(
                "[HamDirector::CleanOriginalMoveData] Removing %s ...\n", regTex->Name()
            );
            delete regTex;
        }
        MILO_LOG("[HamDirector::CleanOriginalMoveData] Removing %s ...\n", it->Name());
        delete it;
    }
}

void HamDirector::LoadRoutineBuilderData(
    std::set<const MoveVariant *> &moveVariants, bool b2
) {
    if (moveVariants.empty()) {
        FOREACH (it, unk370) {
            delete *it;
        }
        unk370.clear();
    } else {
        ObjectDir *moveMgrDir = TheMoveMgr->MoveDataDir();
        if (!moveMgrDir) {
            MILO_NOTIFY("Move data missing from %s", TheGameData->GetSong());
        } else {
            ObjectDir *movesDir = GetWorld()->Find<ObjectDir>("moves", true);
            int movesDirHash = movesDir->HashTableSize() + moveMgrDir->HashTableSize();
            int movesDirStr = movesDir->StrTableSize() + moveMgrDir->StrTableSize();
            movesDir->Reserve(movesDirHash, movesDirStr);
            std::vector<Hmx::Object *> objects;
            objects.reserve(0x80);
            for (ObjDirItr<HamMove> it(moveMgrDir, false); it != nullptr; ++it) {
                objects.push_back(it);
            }
            FOREACH (it, objects) {
                Hmx::Object *cur = *it;
                const char *name = cur->Name();
                Hmx::Object *find = movesDir->FindObject(name, false, false);
                if (!find) {
                    cur->SetName(name, movesDir);
                    unk370.insert(cur);
                }
            }
            objects.clear();
            ObjectDir *clipsDir = GetWorld()->Find<ObjectDir>("clips", true);
            int clipsDirHash = clipsDir->HashTableSize() + moveMgrDir->HashTableSize();
            int clipsDirStr = clipsDir->StrTableSize() + moveMgrDir->StrTableSize();
            clipsDir->Reserve(clipsDirHash, clipsDirStr);
            for (ObjDirItr<CharClip> it(clipsDir, false); it != nullptr; ++it) {
                objects.push_back(it);
            }
            FOREACH (it, objects) {
                Hmx::Object *cur = *it;
                const char *name = cur->Name();
                Hmx::Object *find = clipsDir->FindObject(name, false, false);
                if (!find) {
                    cur->SetName(name, clipsDir);
                    unk370.insert(cur);
                }
            }
        }
    }
}

void HamDirector::OnPopulateMoveMgr() {
    TheMoveMgr->LoadSongData();
    TheMoveMgr->InitSong();
    TheMoveMgr->AutoFillParents();
    TheMoveMgr->FillRoutineFromParents(-1);
    TheMoveMgr->ComputeLoadedMoveSet();
    // LoadRoutineBuilderData()
    OnPopulateFromMoveMgr();
    DataArrayPtr variants;
    TheMoveMgr->SaveRoutineVariants(variants);
    DataWriteFile("routine_test_variants.dta", variants, 0);
    DataArrayPtr parents;
    TheMoveMgr->SaveRoutine(parents);
    DataWriteFile("routine_test_parents.dta", parents, 0);
}

void HamDirector::OnPopulateFromFile() {
    TheMoveMgr->LoadSongData();
    TheMoveMgr->InitSong();
    DataArray *variants = DataReadFile("routine_test_variants.dta", true);
    if (variants) {
        TheMoveMgr->LoadRoutineVariants(variants);
        variants->Release();
    }
    TheMoveMgr->ComputeLoadedMoveSet();
    // LoadRoutineBuilderData()
    OnPopulateFromMoveMgr();
}

void HamDirector::MoveKeys(
    Difficulty diff, MoveDir *moveDir, std::vector<HamMoveKey> &moveKeys
) {
    moveKeys.clear();
    PropKeys *propKeys = GetPropKeys(diff, "move");
    if (propKeys) {
        Keys<Symbol, Symbol> *symKeys = propKeys->AsSymbolKeys();
        for (int i = 0; i < symKeys->size(); i++) {
            Key<Symbol> curSymKey = (*symKeys)[i];
            HamMove *move = moveDir->Find<HamMove>(curSymKey.value.Str(), false);
            if (move) {
                HamMoveKey key;
                key.move = move;
                key.beat = FrameToBeat(curSymKey.frame);
                moveKeys.push_back(key);
            }
        }
    }
}

DataNode HamDirector::OnClipSafeToAdd(DataArray *a) {
    if (a->Int(2)) {
        for (int i = 0; i < kNumDifficultiesDC2; i++) {
            if (mSongAnims[(Difficulty)i]) {
                PropKeys *propKeys = mSongAnims[(Difficulty)i]->GetKeys(
                    this, DataArrayPtr(Symbol("clip"))
                );
                if (propKeys) {
                    Keys<Symbol, Symbol> *symKeys = propKeys->AsSymbolKeys();
                    for (int j = 0; j < symKeys->size(); j++) {
                        Key<Symbol> &curKey = (*symKeys)[j];
                        curKey.frame =
                            BeatToFrame(floor(FrameToBeat(curKey.frame) + 0.5f));
                    }
                }
            }
        }
    }
    return 0;
}

DataNode HamDirector::OnPracticeSafeToAdd(DataArray *a) {
    if (a->Int(2)) {
        for (int i = 0; i < kNumDifficultiesDC2; i++) {
            if (mSongAnims[(Difficulty)i]) {
                PropKeys *propKeys = mSongAnims[(Difficulty)i]->GetKeys(
                    this, DataArrayPtr(Symbol("practice"))
                );
                if (propKeys) {
                    Keys<Symbol, Symbol> *symKeys = propKeys->AsSymbolKeys();
                    for (int j = 0; j < symKeys->size(); j++) {
                        Key<Symbol> &curKey = (*symKeys)[j];
                        curKey.frame =
                            BeatToFrame(floor(FrameToBeat(curKey.frame) + 0.5f));
                    }
                }
            }
        }
    }
    return 0;
}

void HamDirector::HandleDifficultyChange() {
    if (TheLoadMgr.EditMode()) {
        if (TheHamProvider->Property("merge_moves", true)->Int()) {
            MoveDir *dir = TheHamDirector->GetWorld()->Find<MoveDir>("moves", false);
            if (dir) {
                SetupRoutineBuilderAnims();
                TheMoveMgr->ResetRemixer();
            }
        }
    }
}

void HamDirector::FindNextShot() {
    mNextShot = nullptr;
    Key<Symbol> *start;
    Key<Symbol> *end;
    if (GetPracticeFrames(start, end)) {
        static Symbol skills_mode("skills_mode");
        static Symbol review("review");
        bool inReview = TheHamProvider->Property(skills_mode, true)->Sym() == review;
        static Symbol PRACTICE("PRACTICE");
        static Symbol PRACTICE_RECAP("PRACTICE_RECAP");
        static Symbol SKILLS_RESULTS("SKILLS_RESULTS");
        if (mShot != SKILLS_RESULTS) {
            if (inReview) {
                mShot = PRACTICE_RECAP;
            } else {
                mShot = PRACTICE;
            }
        }
    }
    if (!mShot.Null() && mVenue) {
        std::vector<CameraManager::PropertyFilter> propFilters;
        AddNumPlayers(propFilters, nullptr);
        String shotStr(mShot.Str());
        shotStr.ToLower();
        if (shotStr.compare(0, 5, "area2") == 0 || shotStr.compare(0, 5, "area3") == 0) {
            shotStr = mShot;
            shotStr[4] = '1';
            mShot = shotStr.c_str();
        }
        mNextShot = dynamic_cast<HamCamShot *>(
            mVenue->GetCameraManager()->FindCameraShot(mShot, propFilters)
        );
        if (!mNextShot) {
            MILO_NOTIFY(
                "could not find HamCamShot %s in %s at %s, ignoring",
                mShot,
                mVenue->GetPathName(),
                TheTaskMgr.GetMBT()
            );
        }
    }
}

void HamDirector::SetShot(Symbol s) {
    if (TheTaskMgr.Seconds(TaskMgr::kRealTime) >= 0
        && !(SongAnim(0) && SongAnim(0)->GetFrame() < 0)) {
        static Symbol review("review");
        static Symbol skills_mode("skills_mode");
        bool inReview = TheHamProvider->Property(skills_mode, true)->Sym() == review;
        Key<Symbol> *start;
        Key<Symbol> *end;
        if (s == "SKILLS_RESULTS" || !GetPracticeFrames(start, end)
            || (inReview && s != "DC_PLAYER_FREESTYLE")) {
            static Symbol mind_control("mind_control");
            static Symbol gameplay_mode("gameplay_mode");
            static Symbol game_stage("game_stage");
            static Symbol playing("playing");
            static Symbol CAMP_MINDCONTROL_DANCE("CAMP_MINDCONTROL_DANCE");
            Symbol gameplayModeSym = TheHamProvider->Property(gameplay_mode, true)->Sym();
            Symbol stageSym = TheHamProvider->Property(game_stage, true)->Sym();
            if (gameplayModeSym == mind_control && stageSym == playing) {
                s = CAMP_MINDCONTROL_DANCE;
            }
            if (strncmp(s.Str(), "dc_", 3) != 0) {
                mShot = s;
                unk140 = true;
            }
        }
    }
}

float HamDirector::BeatFromTag(Symbol s) {
    static Symbol practice("practice");
    RndPropAnim *songAnim = SongAnim(0);
    PropKeys *propKeys = songAnim->GetKeys(this, DataArrayPtr(practice));
    if (propKeys) {
        for (int i = 0; i < propKeys->NumKeys(); i++) {
            float frame = 0;
            propKeys->FrameFromIndex(i, frame);
            if (s == (*propKeys->AsSymbolKeys())[i].value) {
                return FrameToBeat(frame);
            }
        }
    }
    return -1;
}

void HamDirector::Reteleport() {
    Symbol s;
    float beat = MsToBeat(TheMaster->StreamMs());
    static Symbol practice("practice");
    RndPropAnim *anim =
        GetPropAnim(TheGameData->Player(0)->GetDifficulty(), "song.anim", false);
    PropKeys *propKeys = anim->GetKeys(this, DataArrayPtr(practice));
    int frameIdx = 0;
    if (propKeys) {
        frameIdx = propKeys->AsSymbolKeys()->AtFrame(BeatToFrame(beat), s);
        // GetClipStartAndEndBeats
    }
    Vector3 v = Vector3::ZeroVec();
}

bool HamDirector::ReactToCollision(float frame) {
    if (TheLoadMgr.EditMode()) {
        return false;
    }
    float beat = FrameToBeat(frame);
    if (!mCurShot)
        return false;
    Symbol cat = mCurShot->Category();
    if (strncmp(cat.Str(), "Area", 4) != 0) {
        return false;
    }
    static Symbol shot("shot");
    PropKeys *propKeys = GetPropKeysByPlayer(0, shot);
    Symbol symAt;
    if (!propKeys)
        return false;
    int keyIdx = propKeys->SymbolAt(frame, symAt);
    if (keyIdx >= 0 && strncmp(symAt.Str(), "Area", 4) == 0) {
        cat = symAt;
    }
    float frame2;
    bool idxExists = propKeys->FrameFromIndex(keyIdx, frame2);
    Symbol symAt2;
    if (!idxExists)
        return false;
    int keyIdx2 = propKeys->SymbolAt(frame, symAt2);
    if (keyIdx2 == -1 || keyIdx2 == propKeys->NumKeys() - 1
        || strncmp(symAt2.Str(), "Area", 4) != 0) {
        mShot = cat;
        unk140 = true;
        return true;
    }
    float beat2 = FrameToBeat(frame2);
    static float sSongCollisionUseShotWithinXBeats =
        DataGetMacro("SONG_COLLISION_USE_SHOT_WITHIN_X_BEATS")->Float(0);
    if (beat2 < sSongCollisionUseShotWithinXBeats + beat) {
        ReactToCollision_MoveShot(keyIdx2, beat);
        return true;
    } else {
        static float sSongCollisionForXBeatsSuppressNextShot =
            DataGetMacro("SONG_COLLISION_FOR_X_BEATS_SUPPRESS_NEXT_SHOT")->Float(0);
        float beatSum = sSongCollisionForXBeatsSuppressNextShot + beat;
        if (beat2 < beatSum) {
            ReactToCollision_InsertRealShot(cat, beat);
        } else {
            static bool sSongCollisionRoundUpSuppressedShotToMeasure =
                DataGetMacro("SONG_COLLISION_ROUND_UP_SUPPRESSED_SHOT_TO_MEASURE")->Int(0);
            if (sSongCollisionRoundUpSuppressedShotToMeasure) {
                beatSum = ceil(beatSum / 4.0f);
            }
            float frame3;
            if (!propKeys->FrameFromIndex(keyIdx2 + 1, frame3)) {
                return false;
            }
            float beat3 = FrameToBeat(frame3);
            static float sSongCollisionAbortSuppressedShotIfAnotherWithinXBeats =
                DataGetMacro(
                    "SONG_COLLISION_ABORT_SUPPRESSED_SHOT_IF_ANOTHER_WITHIN_X_BEATS"
                )
                    ->Float(0);
            if (beatSum
                < beat3 - sSongCollisionAbortSuppressedShotIfAnotherWithinXBeats) {
                ReactToCollision_MoveShot(keyIdx2, beat3);
            } else {
                ReactToCollision_MoveShot(keyIdx2, beat2);
            }
        }
    }
    return true;
}

void HamDirector::UnloadMergers() {
    if (mMerger) {
        mMerger->Clear();
        mMoveMerger->Clear();
        if (TheHamWardrobe) {
            for (int i = 0; i < 2; i++) {
                HamCharacter *hc = TheHamWardrobe->GetCharacter(i);
                if (hc) {
                    hc->UnloadAll();
                }
            }
            int i = 0;
            while (true) {
                HamCharacter *hc = TheHamWardrobe->GetBackup(i);
                i++;
                if (!hc)
                    break;
                hc->UnloadAll();
            }
            TheHamWardrobe->ClearCrowdClips();
        }
        unk370.clear();
    }
}

void HamDirector::UnloadAll() {
    AutoGlitchReport report(50, "HamDirector::UnloadAll");
    TheMoveMgr->Clear();
    UnloadMergers();
}
