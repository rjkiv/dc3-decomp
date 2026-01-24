#include "hamobj/RhythmBattle.h"
#include "RhythmBattlePlayer.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/ArchiveSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/RhythmDetector.h"
#include "macros.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/Option.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"

RhythmBattle::RhythmBattle()
    : mCommandLabel(this), unk1c(this), mPlayerOne(this), mPlayerTwo(this), unk58(this),
      unk6c(this), unk80(this), unk94(this), unka8(this), unkbc(this), unkd0(this),
      unke4(this), unkf8(false), unkf9(true), unkfa(false), mActive(false), unk101(false),
      unk102(false), unk10c(0), unk110(0), unk114(0), unk118(0), unk11c(0), unk120(0),
      unk124(-1), unk128(0), unk130(0), unk148(0), unk14c(0) {}

RhythmBattle::~RhythmBattle() { End(); }

BEGIN_PROPSYNCS(RhythmBattle)
    SYNC_PROP(command_label, mCommandLabel)
    SYNC_PROP(player0, mPlayerOne)
    SYNC_PROP(player1, mPlayerTwo)
    SYNC_PROP(full_ktb, unkf9)
    SYNC_PROP(finale, unkfa)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RhythmBattle)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mCommandLabel;
    bs << mPlayerOne;
    bs << mPlayerTwo;
    bs << unkf9;
END_SAVES

BEGIN_COPYS(RhythmBattle)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RhythmBattle, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mPlayerOne)
        COPY_MEMBER(mPlayerTwo)
        COPY_MEMBER(mCommandLabel)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RhythmBattle)
END_LOADS

bool RhythmBattle::GetGoofy() const {
    MILO_ASSERT(TheGameData != NULL, 0x271);
    MILO_ASSERT(TheGameData->Player(0), 0x272);
    HamPlayerData *player = TheGameData->Player(0);
    return player->Side() == kSkeletonLeft;
}

Symbol RhythmBattle::GetLeader() const {
    static Symbol both("both");
    static Symbol left("left");
    static Symbol right("right");
    bool goofy = GetGoofy();

    return both;
}

void RhythmBattle::ResetCombo() {
    if (mPlayerOne)
        mPlayerOne->ResetCombo();
    if (mPlayerTwo)
        mPlayerTwo->ResetCombo();
}

void RhythmBattle::Enter() {
    RndPollable::Enter();
    if (mPlayerOne)
        mPlayerOne->OnReset(this);
    if (mPlayerTwo)
        mPlayerTwo->OnReset(this);
    unk10c = 0.0f;
    CheckIsFinale();
    if (unkf9)
        Begin();
}

void RhythmBattle::Exit() {
    static Symbol mind_control("mind_control");
    static Symbol gameplay_mode("gameplay_mode");
    const DataNode *node = TheHamProvider->Property(gameplay_mode, true);
    Symbol mindControlCheck = node->Sym();
    if (!unkf9 && mindControlCheck == mind_control)
        End();

    RndPollable::Exit();
}

void RhythmBattle::End() {
    if (mActive) {
        mActive = false;
        if (mPlayerOne)
            mPlayerOne->SetActive(false);
        if (mPlayerTwo)
            mPlayerTwo->SetActive(false);
        if (unk130) {
            unk130->Free();
            RELEASE(unk130);
        }
        if (unk102) {
            static UIPanel *panel =
                ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
            if (panel && panel->LoadedDir()) {
                for (int i = 0; i < 6; i++) {
                    String s = MakeString("RhythmDetectorX%d.rhy", i);
                    RhythmDetector *rh =
                        panel->LoadedDir()->Find<RhythmDetector>(s.c_str(), true);
                    rh->StopRecording();
                }
            }
        }
        if (unkfa) {
            ObjectDir *dir =
                dynamic_cast<ObjectDir *>(DataVariable("hud_panel").GetObj());
            RndDir *rightDir = dir->Find<RndDir>("score_right", true);
            rightDir->SetShowing(true);
            RndDir *leftDir = dir->Find<RndDir>("score_left", true);
            leftDir->SetShowing(true);
        }
        unk102 = false;
    }
}

void RhythmBattle::OnReset() {
    if (unk102)
        OnUnpause();

    CheckIsFinale();
    if (!mActive)
        Begin();

    unkfc = false;
    unkfd = false;
    unk128 = 0;
    unk124 = -1;
    unkfe = false;
    unk120 = 0.0f;
    unkff = false;
    unk144 = TheTaskMgr.Beat();
    unk100 = false;
    unk101 = false;
    unk148 = 0;
    unk14c = 0;
    unk140 = 0;

    if (mPlayerOne)
        mPlayerOne->OnReset(this);

    if (mPlayerTwo)
        mPlayerTwo->OnReset(this);

    if (unk80) {
        unk80->Animate(
            unk80->EndFrame(), unk80->EndFrame(), unk80->Units(), 0, 0, 0, kEaseLinear, 0, 0
        );
    }

    static Symbol gameplay_mode("gameplay_mode");
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol mind_control("mind_control");

    const DataNode *gameplay_node = TheHamProvider->Property(gameplay_mode, true);
    Symbol gameplay_sym = gameplay_node->Sym();
    if (TheHamDirector && unk11c == 0
        && TheHamDirector->SongAnimByDifficulty(kDifficultyEasy)) {
    }

    static Symbol finale_intro_01("finale_intro_01");
    static Symbol finale_intro_02("finale_intro_02");
    QueueFinaleVO(finale_intro_01);
    QueueFinaleVO(finale_intro_02);
}

void RhythmBattle::Poll() {
    if (!TheLoadMgr.EditMode()) {
        if (mActive && unk102) {
            if (GetGoofy() != unkf8) {
                mPlayerOne->SwapObjs(mPlayerTwo);
                unkf8 = GetGoofy();
            }
            Symbol leader = GetLeader();
            static Symbol left("left");
            static Symbol right("right");
            static UIPanel *panel =
                ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
            if (unk130 != 0) {
                static Symbol playing("playing");
                if (TheUI->FocusPanel() && unk100 && unk101) {
                    const Skeleton *skeleton = TheGameData->Player(0)->GetSkeleton();
                    if (!skeleton) {
                        unk130->SetVal44(-1);
                    } else {
                        unk130->SetVal44(skeleton->SkeletonIndex());
                    }
                    unk130->Poll();
                    Skeleton *skeleton2 = TheGestureMgr->GetSkeletonByTrackingID(
                        TheGameData->Player(1)->GetSkeletonTrackingID()
                    );
                    if (!skeleton) {
                        if (!unk134.empty()) {
                            unk134.clear();
                        }
                    } else {
                        ArchiveSkeleton ac = ArchiveSkeleton();
                        unk134.push_back(ac);
                        // if (200 < something) {
                        //     MILO_WARN("bustajack recordings are getting big %d\n",
                        //     something);
                        //     // unk134.back().Set(skeleton);
                        // }
                    }
                }
            }
            float beat = TheTaskMgr.Beat();
            if (beat != unk144 && beat != 0) {
                OnBeat();
                unk144 = beat;
            }
            UpdateMindControl();
        }
        if (unkfa) {
            WorldDir *dir = TheHamDirector->GetVenueWorld();
            RndDir *scoreStarDisplay = dir->Find<RndDir>("score_star_display", true);
            scoreStarDisplay->SetShowing(false);
        }
    }
}

void RhythmBattle::PlayMindControlVO(Symbol s) {
    static Message mind_control_vo("mind_control_vo", DataNode(0));
    mind_control_vo[0] = DataNode(5);
    TheHamProvider->HandleType(mind_control_vo);
    unk110 = 0;
}

void RhythmBattle::UpdateFinaleVO(int &i) {
    if (!unk150.empty() && 500 < i) {
        UIPanel *game_panel = TheUI->FocusPanel();
        MILO_ASSERT(game_panel != NULL, 0x708);
        unk150.erase(&unk150.front());
        static Message play_finale_vo("play_finale_vo", DataNode(0));
        play_finale_vo[0] = DataNode(5);
        DataNode handled = game_panel->HandleType(play_finale_vo);
        i = -1;
    }
}

void RhythmBattle::OnPause() {
    if (mActive) {
        unk102 = true;
        static UIPanel *panel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
        if (panel && panel->LoadedDir()) {
            for (int i = 0; i < 6; i++) {
                String s = MakeString("RhythmDetectorX%d.rhy", i);
                RhythmDetector *detector =
                    panel->LoadedDir()->Find<RhythmDetector>(s.c_str(), true);
                detector->StopRecording();
            }
        }
    }
}

void RhythmBattle::OnUnpause() {
    if (unk102) {
        MILO_ASSERT(mActive, 0x7cc);
        unk102 = false;
        static UIPanel *panel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
        if (panel && panel->LoadedDir()) {
            for (int i = 0; i < 6; i++) {
                String s = MakeString("RhythmDetectorX%d.rhy", i);
                RhythmDetector *detector =
                    panel->LoadedDir()->Find<RhythmDetector>(s.c_str(), true);
                detector->StartRecording();
            }
        }
    }
}

void RhythmBattle::QueueFinaleVO(Symbol s) {
    if (unkfa) {
        bool b = false;
        for (int i = 0; i < unk150.size(); i++) {
            if (unk150[i] == s)
                b = true;
        }

        if (!b)
            unk150.push_back(s);
    }
}

void RhythmBattle::UpdateMindControl() {
    static Symbol mind_control("mind_control");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol game_stage("game_stage");
    static Symbol playing("playing");
}

void RhythmBattle::OnBeat() {
    // This is a ~16KB function with extensive game logic for rhythm battles
    // Full implementation requires matching ~4700 lines of assembly
    MILO_ASSERT(mActive, 0x290);

    // Static symbol initialization (must be in exact order for bit flags to match)
    static Symbol playing("playing");
    UIPanel *game_panel = TheUI->FocusPanel();
    bool goofy = GetGoofy();

    if (!game_panel || !mPlayerOne || !mPlayerTwo)
        return;

    static Symbol gameplay_mode("gameplay_mode");
    static Symbol mind_control("mind_control");

    const DataNode *gameplay_node = TheHamProvider->Property(gameplay_mode, true);
    Symbol gameplay_sym = gameplay_node->Sym(0);
    bool isMindControl = gameplay_sym == mind_control;

    // Set player flags based on game mode
    if (unkf9 && !unkfa && !isMindControl) {
        mPlayerOne->unk2a5 = false;
        mPlayerTwo->unk2a5 = false;
    } else {
        mPlayerOne->unk2a5 = true;
        mPlayerTwo->unk2a5 = true;
    }

    // Get remaining VO time from panel
    static Message remaining_vo_ms("remaining_vo_ms");
    DataNode result = game_panel->HandleType(remaining_vo_ms);
    int remaining_ms = result.Type() == 0 ? result.Int(0) : 0;

    // Initialize play_vo message
    static Message play_vo("play_vo", DataNode(0), DataNode(0));
    static Message play_finale_vo("play_finale_vo", DataNode(0));
    static Symbol empty("");
    play_vo[2] = DataNode(empty);
    static Symbol both("both");
    static Symbol left("left");
    static Symbol right("right");
    play_vo[3] = DataNode(both);

    // Calculate beat information
    float beat = TheTaskMgr.Beat();
    int iBeat = beat >= 0.0f ? (int)(beat + 0.5f) : (int)(beat - 0.5f);

    // Count-in message at beat 4
    if (iBeat == 4) {
        static Message countInMsg("count_in", DataNode(0), DataNode(0));
        countInMsg[2] = DataNode(unk104 - 4.0f);
        countInMsg[3] = DataNode(unk104 - 4.0f);
        game_panel->Handle(countInMsg, true);
    }

    UpdateFinaleVO(remaining_ms);
    Symbol leader = GetLeader();

    // intro trigger
    const char *intro = "intro";
    if (remaining_ms > 0 && !unkfd) {
        static Symbol finished_intro("finished_intro");
        play_vo[2] = DataNode(finished_intro);
        if (!unkfa) {
            game_panel->HandleType(play_vo);
            remaining_ms = -1;
        }
        unkfd = true;
    }

    // halftime and almost_over triggers
    const char *halftime = "halftime";
    const char *almost_over = "almost_over";

    if (remaining_ms > 2000 && !unkfe && unk114 < beat) {
        static Symbol halftime_sym("halftime");
        play_vo[2] = DataNode(halftime_sym);
        play_vo[3] = DataNode(leader);
        if (!unkfa) {
            game_panel->HandleType(play_vo);
            remaining_ms = -1;
        }
        unkfe = true;
    }

    if (remaining_ms > 2000 && !unkff && unk118 < beat) {
        static Symbol almost_over_sym("almost_over");
        play_vo[2] = DataNode(almost_over_sym);
        play_vo[3] = DataNode(leader);
        if (!unkfa) {
            game_panel->HandleType(play_vo);
            remaining_ms = -1;
        }
        unkff = true;
    }

    // unk104 check - intro animation
    if (unk104 < beat) {
        if (!unkfc) {
            float inAnimLength = mPlayerOne->InAnimBeatLength();
            if (unk104 >= beat + inAnimLength) {
                mPlayerOne->AnimateIn();
                mPlayerTwo->AnimateIn();
                if (unk80) {
                    float startFrame = unk80->StartFrame();
                    unk80->Animate(
                        startFrame,
                        unk80->EndFrame(),
                        unk80->Units(),
                        0.0f,
                        0.0f,
                        0,
                        kEaseLinear,
                        0.0f,
                        false
                    );
                }
                unkfc = true;
            } else {
                mPlayerOne->SetActive(false);
                mPlayerTwo->SetActive(false);
            }
        }
    } else {
        if (!unk100) {
            ObjectDir *obj = mPlayerOne->unk1e8;
            if (obj) {
                obj->Find<Hmx::Object>(gNullStr, false);
            }
            obj = mPlayerTwo->unk1e8;
            if (obj) {
                obj->Find<Hmx::Object>(gNullStr, false);
            }

            mPlayerOne->SetActive(true);
            mPlayerTwo->SetActive(true);
            mPlayerOne->SetInTheZone(0, false, false);
            mPlayerTwo->SetInTheZone(0, false, false);

            static Message finished_intro2("finished_intro");
            game_panel->HandleType(finished_intro2);
        }
    }

    // Winner check
    const char *winner = "winner";
    if (unkf9) {
        if (unk108 < beat && !unkfa) {
            unk101 = true;
            mPlayerOne->SetActive(false);
            mPlayerTwo->SetActive(false);
            if (unkfc) {
                static Symbol winner_sym("winner");
                play_vo[2] = DataNode(winner_sym);
                play_vo[3] = DataNode(leader);
                if (!unkfa) {
                    game_panel->HandleType(play_vo);
                    remaining_ms = -1;
                }
                mPlayerOne->ResetCombo();
                mPlayerTwo->ResetCombo();
                if (unk94) {
                    unk94->Animate(
                        unk94->StartFrame(),
                        unk94->EndFrame(),
                        unk94->Units(),
                        0.0f,
                        0.0f,
                        0,
                        kEaseLinear,
                        0.0f,
                        false
                    );
                }
                unkfc = false;
            }
        } else {
            if (!unkfa) {
                mPlayerOne->SetActive(true);
                mPlayerTwo->SetActive(true);
            } else {
                bool active = unk148 != 0 && (int)beat <= 16;
                mPlayerOne->SetActive(active);
                mPlayerTwo->SetActive(active);
            }
        }
    }

    mPlayerOne->HackPlayerQuit();
    mPlayerTwo->HackPlayerQuit();

    // Game state check and swag jack scoring
    float f27_one = 1.0f;
    bool r19_fullKtb = !isMindControl; // Based on !isMindControl
    bool r21_swagJacked = false;

    // This block runs if we're in full KTB mode (not mind control)
    if (r19_fullKtb) {
        // Inner conditional for swag jack recording
        if (unkf9 && !unkfa && !isMindControl) {
            unk130->StopRecording();

            bool shouldScore = false;
            if (unk140 == 0 && (mPlayerOne->InTheZone() || mPlayerTwo->InTheZone())) {
                shouldScore = true;
            }

            float totalScore = 0.0f;
            if (shouldScore) {
                // DancerSkeleton scoring loop iterates through recorded skeletons
                // and computes a score based on movement similarity
                size_t vecSize = unk134.size();
                if (vecSize > 0) {
                    float scoreAccum = 0.0f;
                    const float scoreScale = 0.001f;

                    for (size_t i = 0; i < vecSize; i++) {
                        DancerSkeleton ds;

                        if (i > 0) {
                            scoreAccum += (float)unk134[i].ElapsedMs() * scoreScale;
                        }

                        // Process each joint
                        for (int j = 0; j < 20; j++) {
                            Vector3 pos1, pos2;
                            unk134[i].JointPos(kCoordCamera, (SkeletonJoint)j, pos1);

                            size_t prevIdx = (i > 0) ? i - 1 : 0;
                            unk134[prevIdx].JointPos(kCoordCamera, (SkeletonJoint)j, pos2);

                            ds.SetCamJointPos((SkeletonJoint)j, pos1);
                            ds.SetCamJointDisplacement(
                                (SkeletonJoint)j,
                                Vector3(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z)
                            );
                        }

                        ds.SetDisplacementElapsedMs(unk134[i].ElapsedMs());
                        ds.Set(unk134[i]);
                        totalScore = unk130->GetScore(&ds, true, scoreAccum, false);
                    }
                }
            }

            // autojack option check
            static bool autojack = OptionBool("autojack", false);
            if (shouldScore && autojack) {
                totalScore = f27_one;
            }

            // Score threshold check (0.7f)
            if (totalScore > 0.7f) {
                RhythmBattlePlayer *jacker = mPlayerOne;
                RhythmBattlePlayer *jackee = mPlayerTwo;

                // Determine who gets jacked based on InTheZone and score
                if (jacker->InTheZone()) {
                    if (jackee->InTheZone()) {
                        if (jacker->unk280 > jackee->unk280) {
                            jackee = jacker;
                        }
                    }
                } else if (jacker->InTheZone()) {
                    jackee = jacker;
                }

                if (!jackee->InTheZone()) {
                    jackee = jackee->InTheZone() ? mPlayerOne : mPlayerTwo;
                }

                MILO_ASSERT(jackee->InTheZone(), 0x398);
                r21_swagJacked = true;
                unk140 = 4;
            }

            // Clear the vector
            if (unk134.begin() != unk134.end()) {
                unk134.erase(unk134.begin(), unk134.end());
            }

            unk130->ClearRecording();
            unk130->ClearFrameScores();

            unk140--;
            if (unk140 < 0)
                unk140 = 0;
        }

        if (!unk101) {
            unk130->StartRecording();
        }

        // UpdateScore for both players
        mPlayerOne->UpdateScore(game_panel);
        mPlayerTwo->UpdateScore(game_panel);
    }

    // bars_between_vo_suggestion section
    static Message bars_between_vo_suggestion("bars_between_vo_suggestion");
    DataNode barsResult = game_panel->HandleType(bars_between_vo_suggestion);
    int barsBetween = barsResult.Type() == 0 ? barsResult.Int(0) : 0;

    // UpdateState for both players
    bool p1StateChanged = false;
    bool p2StateChanged = false;
    if (r19_fullKtb) {
        p1StateChanged = mPlayerOne->UpdateState();
        p2StateChanged = mPlayerTwo->UpdateState();
    }

    // Determine leading player state for VO decisions
    int p1State = p1StateChanged ? mPlayerOne->unk260 : -1;
    int p2State = p2StateChanged ? mPlayerTwo->unk260 : -1;

    int leadingState = -1;
    if (p1State > p2State) {
        if (p1StateChanged)
            leadingState = mPlayerOne->unk260;
    } else {
        if (p2StateChanged)
            leadingState = mPlayerTwo->unk260;
    }

    // VO symbol string pointers (stored on stack for switch statement)
    const char *inzone_warning_str = "inzone_warning";
    const char *inzone_str = "inzone";
    const char *new_groove_working_str = "new_groove_working";
    const char *max_multiplier_str = "max_multiplier";
    const char *jack_swag_str = "jack_swag";

    // Static VO symbols (must be in specific order for MSVC bit flags)
    static Symbol msg_offbeat_p1p2("rhythmbattle_off_beat_p1p2");
    static Symbol msg_offbeat_p1("rhythmbattle_off_beat_p1");
    static Symbol msg_offbeat_p2("rhythmbattle_off_beat_p2");
    static Symbol msg_nice_moves_p1p2("rhythmbattle_nice_moves_p1p2");
    static Symbol msg_nice_moves_p1("rhythmbattle_nice_moves_p1");
    static Symbol msg_nice_moves_p2("rhythmbattle_nice_moves_p2");
    static Symbol msg_lemme_see_p1p2("rhythmbattle_lemme_see_something_p1p2");
    static Symbol msg_lemme_see_p1("rhythmbattle_lemme_see_something_p1");
    static Symbol msg_lemme_see_p2("rhythmbattle_lemme_see_something_p2");
    static Symbol msg_stale_warn_p1p2("rhythmbattle_stale_moves_warning_p1p2");
    static Symbol msg_stale_warn_p1("rhythmbattle_stale_moves_warning_p1");
    static Symbol msg_stale_warn_p2("rhythmbattle_stale_moves_warning_p2");
    static Symbol msg_stale_penalty_p1p2("rhythmbattle_stale_moves_penalty_p1p2");
    static Symbol msg_stale_penalty_p1("rhythmbattle_stale_moves_penalty_p1");
    static Symbol msg_stale_penalty_p2("rhythmbattle_stale_moves_penalty_p2");
    static Symbol msg_synchronized1("rhythmbattle_synchronized1");
    static Symbol msg_swagjacked1("rhythmbattle_swagjackeddd1");
    static Symbol msg_synchronized2("rhythmbattle_synchronized2");
    static Symbol msg_swagjacked2("rhythmbattle_swagjackeddd2");
    static Symbol msg_groove_expired("groove_expired");
    static Symbol msg_swag_jacked("swag_jacked");
    static Symbol msg_no_groove_yet("no_groove_yet");
    static Symbol msg_change_groove("change_groove");
    static Symbol msg_trick_performed("trick_performed");
    static Symbol msg_new_groove("new_groove");
    static Symbol msg_getting_idea("getting_idea");
    static Symbol msg_stole_congrats("stole_congrats");
    static Symbol msg_finale_no_groove("finale_no_groove_yet");

    // Check player InTheZone states for combo updates
    int p1InZone = mPlayerOne->InTheZone();
    int p2InZone = mPlayerTwo->InTheZone();

    // If we're in full KTB mode, update combo progress and animations
    if (r19_fullKtb) {
        mPlayerOne->UpdateComboProgress();
        mPlayerTwo->UpdateComboProgress();
        mPlayerOne->UpdateAnimations(game_panel);
        mPlayerTwo->UpdateAnimations(game_panel);
    }

    // VO suggestion switch based on unk124 state
    int voState = unk124;
    switch (voState) {
    case 6: {
        // Swagjacked VO for player 2
        unk120 = 0;
        unk124 = 5;
        RhythmBattlePlayer *target = mPlayerTwo;
        if (target) {
            static Message msg_swagjack_p2("rhythm_battle_swagjackeddd");
            target->Handle(msg_swagjack_p2, false);
        }
        break;
    }
    case 5: {
        // Swagjacked VO for player 1
        unk120 = 0;
        unk124 = 5;
        RhythmBattlePlayer *target = mPlayerOne;
        if (target) {
            static Message msg_swagjack_p1("rhythm_battle_swagjackeddd");
            target->Handle(msg_swagjack_p1, false);
        }
        break;
    }
    case 4: {
        // Synchronized VO for player 2
        RhythmBattlePlayer *target = mPlayerTwo;
        if (target) {
            static Message msg_sync_p2("rhythm_battle_synchronized");
            target->Handle(msg_sync_p2, false);
        }
        break;
    }
    case 3: {
        // Synchronized VO for player 1
        RhythmBattlePlayer *target = mPlayerOne;
        if (target) {
            static Message msg_sync_p1("rhythm_battle_synchronized");
            target->Handle(msg_sync_p1, false);
        }
        break;
    }
    case 2: {
        // Switch it up VO with sub-cases
        if (r21_swagJacked) {
            bool goofyMode = GetGoofy();
            if (goofyMode) {
                RhythmBattlePlayer *target = mPlayerTwo;
                if (target) {
                    static Message msg_switch_p1p2("rhythm_battle_switch_it_up_p1p2");
                    target->Handle(msg_switch_p1p2, false);
                }
            } else {
                if (r21_swagJacked) {
                    RhythmBattlePlayer *target = mPlayerTwo;
                    if (target) {
                        static Message msg_switch_p1("rhythm_battle_switch_it_up_p1");
                        target->Handle(msg_switch_p1, false);
                    }
                }
            }
        }
        break;
    }
    case 1: {
        // Fresh moves VO with sub-cases
        unk120 = 6000;
        unk124 = 5;
        if (r21_swagJacked) {
            bool goofyMode = GetGoofy();
            if (goofyMode) {
                unk120 = 5000;
                RhythmBattlePlayer *target = mPlayerTwo;
                if (target) {
                    static Message msg_fresh_p1p2("rhythm_battle_fresh_moves_p1p2");
                    target->Handle(msg_fresh_p1p2, false);
                }
            } else {
                if (r21_swagJacked) {
                    RhythmBattlePlayer *target = mPlayerTwo;
                    if (target) {
                        static Message msg_fresh_p1("rhythm_battle_fresh_moves_p1");
                        target->Handle(msg_fresh_p1, false);
                    }
                } else {
                    RhythmBattlePlayer *target = mPlayerTwo;
                    if (target) {
                        static Message msg_fresh_p2("rhythm_battle_fresh_moves_p2");
                        target->Handle(msg_fresh_p2, false);
                    }
                }
            }
        }
        break;
    }
    case 0:
    default: {
        // No groove VO with sub-cases
        if (r21_swagJacked) {
            bool goofyMode = GetGoofy();
            if (goofyMode) {
                unk120 = 6000;
                RhythmBattlePlayer *target = mPlayerTwo;
                if (target) {
                    static Message msg_nogroove_p1p2("rhythm_battle_no_groove_p1p2");
                    target->Handle(msg_nogroove_p1p2, false);
                }
            } else {
                if (r21_swagJacked) {
                    RhythmBattlePlayer *target = mPlayerTwo;
                    if (target) {
                        static Message msg_nogroove_p1("rhythm_battle_no_groove_p1");
                        target->Handle(msg_nogroove_p1, false);
                    }
                } else {
                    RhythmBattlePlayer *target = mPlayerTwo;
                    if (target) {
                        static Message msg_nogroove_p2("rhythm_battle_no_groove_p2");
                        target->Handle(msg_nogroove_p2, false);
                    }
                }
            }
        }
        break;
    }
    }

    // Post-switch projection logic
    // Check bars_between_vo_suggestion result for projection triggers
    static Symbol proj_inzone_p1p2_sym("inzone_p1p2");
    static Symbol proj_inzone_p1_sym("inzone_p1");
    static Symbol proj_inzone_p2_sym("inzone_p2");

    // Get remaining VO ms comparison
    Symbol symResult = barsResult.Sym(0);
    if (symResult != msg_finale_no_groove && unk120 > 0) {
        // Check player zone state for projection
        RhythmBattlePlayer *p1 = mPlayerOne;
        RhythmBattlePlayer *p2 = mPlayerTwo;

        bool p1HasZone = (p1->mInTheZone != 0 || p1->unk26c != 0);
        bool p2HasZone = (p2->mInTheZone != 0 || p2->unk26c != 0);

        if (!p1HasZone || !p2HasZone) {
            // At least one player not in full zone
            bool r30_p1InZone = false;
            bool r29_p2InZone = false;

            if (p1->InTheZone() || p2->InTheZone()) {
                // Check both players zone status
                if (p1->InTheZone() && p2->InTheZone()) {
                    // Both in zone - use p1p2 symbol
                    static Message proj_both("projection_inzone");
                    game_panel->HandleType(proj_both);
                } else {
                    // Only one in zone
                    static Message proj_single("projection_single");
                    game_panel->HandleType(proj_single);
                }

                r30_p1InZone = p1->InTheZone() != 0;
                r29_p2InZone = p2->InTheZone() != 0;
            }

            // Swap if goofy mode
            if (GetGoofy()) {
                bool temp = r30_p1InZone;
                r30_p1InZone = r29_p2InZone;
                r29_p2InZone = temp;
            }

            // Send appropriate projection message
            if (r30_p1InZone && r29_p2InZone) {
                static Message proj_msg_p1p2("projection_inzone_p1p2");
                game_panel->HandleType(proj_msg_p1p2);
            } else if (r30_p1InZone) {
                static Message proj_msg_p1("projection_inzone_p1");
                game_panel->HandleType(proj_msg_p1);
            } else {
                static Message proj_msg_p2("projection_inzone_p2");
                game_panel->HandleType(proj_msg_p2);
            }
        }
    }

    // Line 326: worked_it_progress - after checking loading_panel
    {
        static UIPanel *loading_panel =
            ObjectDir::Main()->Find<UIPanel>("loading_panel", false);
        if (loading_panel && loading_panel->LoadedDir()) {
            static Message worked_it_progress("worked_it_progress", DataNode(0));
            worked_it_progress[0] = DataNode(1);
            worked_it_progress[1] = DataNode(std::max(mPlayerOne->unk284, mPlayerTwo->unk284));
            loading_panel->HandleType(worked_it_progress);
        }
    }

    // reset_finale check
    static Symbol reset_finale("reset_finale");
    if (DataVariable(reset_finale).Int(0) != 0) {
        DataVariable(reset_finale) = DataNode(0);
        unk14c = 0;
        unk148 = 0;
    }

    // Finale-specific logic
    if (unkfa) {
        WorldDir *venueWorld = TheHamDirector->GetVenueWorld();
        RndAnimatable *flow = venueWorld->Find<RndAnimatable>("show_timeywimey.flow", true);
        flow->Animate(flow->StartFrame(), flow->EndFrame(), flow->Units(), 0, 0, 0, kEaseLinear, 0, 0);
    }

    // Disable player spotlights when appropriate
    if (isMindControl) {
        TheHamDirector->SetPlayerSpotlightsEnabled(false);
    }

    // Finale unk148 countdown logic with phase transitions
    int finaleFlags = (unkfa ? 0x10 : 0) | 0x8;
    if (isMindControl || unkfa) {
        if (unk148 > 0) {
            unk148--;

            // unk148 == 22 (0x16): hide boxyman
            if (unkfa && unk148 == 22) {
                RndDir *boxyman = TheHamDirector->GetVenueWorld()->Find<RndDir>("boxyman", true);
                boxyman->SetShowing(false);
            }

            // unk148 == 17 (0x11): play tan clip if unk14c < 3
            if (unk148 == 17 && unk14c < 3) {
                PlayTanClip(unk14c, false);
            }

            // Line 355: charProjectionActive - unk148 == 16 (0x10)
            if (unk148 == 16) {
                static Message charProjectionActive("char_projection_move_active");
                TheHamProvider->HandleType(charProjectionActive);
            }

            // unk148 == 8: enable player spotlights
            if (unk148 == 8) {
                TheHamDirector->SetPlayerSpotlightsEnabled(true);
            }

            // Line 378: destroyCharProjection - unk14c == 3 && unk148 == 4
            if (unk14c == 3 && unk148 == 4) {
                PlayTanClip(4, true);
                static Message destroyCharProjection("destroy_char_projection");
                TheHamProvider->HandleType(destroyCharProjection);
            }
        } else {
            // unk148 == 0: increment phase
            unk14c++;

            // Phase transitions
            if (unk14c == 1) {
                if (unkfa) {
                    // Line 387: first set of projection handlers
                    static Symbol finale_phaseout_01("finale_phaseout_01");
                    static Symbol finale_phaseout_01b("finale_phaseout_01b");
                    QueueFinaleVO(finale_phaseout_01);
                    QueueFinaleVO(finale_phaseout_01b);

                    static Message charProjectionInactive("char_projection_move_inactive");
                    TheHamProvider->HandleType(charProjectionInactive);

                    static Message hideCharProjection("hide_char_projection_keepdrawing");
                    TheHamProvider->HandleType(hideCharProjection);

                    static Message tanPhaseOut("tan_finale_phaseout02");
                    TheHamProvider->HandleType(tanPhaseOut);
                } else {
                    // Line 394: second set (non-finale path)
                    static Message charProjectionInactive("char_projection_move_inactive");
                    TheHamProvider->HandleType(charProjectionInactive);

                    static Message hideCharProjection("hide_char_projection_keepdrawing");
                    TheHamProvider->HandleType(hideCharProjection);

                    static Message tanPhaseOut("tan_finale_phaseout02");
                    TheHamProvider->HandleType(tanPhaseOut);
                }
            } else if (unk14c == 2) {
                // Line 403: game_outro_mind_control
                if (!unkfa) {
                    static Message game_outro_mind_control("game_outro_mind_control");
                    game_panel->HandleType(game_outro_mind_control);
                }
            } else if (unk14c == 3) {
                // Line 410: game_outro_finale
                if (unkfa) {
                    static Message game_outro_finale("game_outro_finale");
                    game_panel->HandleType(game_outro_finale);
                }
            } else if (unk14c == 4) {
                // Line 439: mindControlCompleteMsg
                if (unkfa) {
                    static Symbol finale_phaseout_03("finale_phaseout_03");
                    static Symbol finale_phaseout_03b("finale_phaseout_03b");
                    QueueFinaleVO(finale_phaseout_03);
                    QueueFinaleVO(finale_phaseout_03b);

                    static Symbol tan_destroyed("tan_destroyed");
                    game_panel->SetProperty(tan_destroyed, DataNode(1));
                } else {
                    static Message mindControlCompleteMsg("mind_control_complete");
                    TheHamProvider->HandleType(mindControlCompleteMsg);
                }
            } else if (unk14c == 5) {
                MILO_ASSERT(unkfa, 0x66c);
            }

            unk148 = unk148 + unk120;
        }
    }

    // Update finale VO at the end
    int voMs = unk120;
    UpdateFinaleVO(voMs);
}

void ClearJump() {
    if (TheMaster && TheMaster->GetAudio()) {
        TheMaster->GetAudio()->ClearLoop();
    }
}

BEGIN_HANDLERS(RhythmBattle)
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_ACTION(reset, OnReset())
    HANDLE_ACTION(begin, Begin())
    HANDLE_ACTION(pause, OnPause())
    HANDLE_ACTION(unpause, OnUnpause())
    HANDLE_ACTION(end, End())
    HANDLE_ACTION(reset_combo, ResetCombo())
    HANDLE_ACTION(set_jump, SetJump(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(clear_jump, ClearJump())
    // HANDLE_ACTION(both_players_dancing_bad,)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
