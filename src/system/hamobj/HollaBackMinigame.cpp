#include "hamobj/HollaBackMinigame.h"
#include "MoveMgr.h"
#include "flow/Flow.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveDir.h"
#include "math/Easing.h"
#include "midi/MidiParser.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Dir.h"
#include "rndobj/Poll.h"
#include "rndobj/PropAnim.h"
#include "synth/SynthSample.h"
#include "ui/PanelDir.h"
#include "ui/UIPanel.h"
#include "utl/Loader.h"
#include "utl/SongPos.h"
#include "utl/Std.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"

void JumpToMeasure(float beat) {
    float ms = BeatToMs(beat * 4.0f);
    Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
    if (game) {
        game->Handle(Message("jump", ms), true);
    }
}

HollaBackMinigame::HollaBackMinigame()
    : mActive(false), mSpecifyFirstMoveMeasure(-1), mInitialMoveCount(2),
      mTotalMeasures(0), mInitialJumpPending(false), m_ShoutoutSound(nullptr) {}

HollaBackMinigame::~HollaBackMinigame() { EndMinigame(true); }

BEGIN_HANDLERS(HollaBackMinigame)
    HANDLE_ACTION(begin_minigame, BeginMinigame(_msg->Array(2)))
    HANDLE_ACTION(end_minigame, EndMinigame(false))
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_EXPR(get_move_state, GetMoveState(_msg->Int(2)))
    HANDLE_ACTION(set_move_state, SetMoveState(_msg->Int(2), _msg->Sym(3)))
    HANDLE_ACTION(set_default_shot, SetDefaultShot())
    HANDLE_EXPR(get_first_move_idx, mStartLoopMeasure + 5)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HollaBackMinigame)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HollaBackMinigame)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(RndPollable)
END_SAVES

BEGIN_COPYS(HollaBackMinigame)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(HollaBackMinigame)
END_COPYS

INIT_REVS(5, 0)

BEGIN_LOADS(HollaBackMinigame)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(RndPollable)
END_LOADS

void HollaBackMinigame::Poll() {
    if (!TheLoadMgr.EditMode() && mActive) {
        RndPropAnim *anim =
            TheHamDirector->GetVenueWorld()->Find<RndPropAnim>("set_bid.anim");
        if (anim) {
            anim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
        }

        if (m_ShoutoutSound && !m_ShoutoutSound->IsPlaying()) {
            EndShoutOut();
        }
        if (mInitialJumpPending) {
            Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
            bool b20 = !game ? false : !game->Handle(Message("is_waiting"), true).Int();
            if (b20 && mInitialJumpPending) {
                JumpToMeasure(mInitialMusicJump);
                TheMaster->GetAudio()->SetPaused(true);
                mInitialJumpPending = false;
            }
        }
        if (mState == Title) {
            static Symbol holla_back_stage("holla_back_stage");
            static Symbol exit_title("exit_title");
            static Symbol start_score_move_index("start_score_move_index");
            static Symbol hide_hud("hide_hud");
            for (int i = 0; i < 2; i++) {
                HamPlayerData *hpd = TheGameData->Player(i);
                if (!hpd->IsPlaying()) {
                    hpd->Provider()->SetProperty(start_score_move_index, 1000);
                    hpd->Provider()->Export(Message(hide_hud, 0), true);
                    mHUDPanel
                        ->Find<RndPropAnim>(
                            hpd->Side() == kSkeletonRight ? "player_1_hud.anim"
                                                          : "player_2_hud.anim"
                        )
                        ->SetFrame(0, 1);
                }
            }
            if (!m_HaveBegunIntroVO && TheMaster->GetAudio()->IsReady()
                && m_PlayIntroVODelay-- <= 0) {
                m_HaveBegunIntroVO = true;
                TheHamProvider->SetProperty(holla_back_stage, Symbol("title"));
                Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
                if (game) {
                    game->Handle(Message("set_realtime", 1), true);
                }
                m_ShoutoutSound = mHollabackHUD->Find<Sound>(
                    MakeString("%s.snd", mIntroShoutout.Str()), false
                );
                if (m_ShoutoutSound) {
                    m_ShoutoutSound->Play(0, 0, 0, nullptr, 0);
                }
                TheHamProvider->SetProperty("game_stage", Symbol("title"));
                TheHamDirector->GetVenueWorld()
                    ->Find<Flow>("animate_timeywimey.flow")
                    ->Activate();
                SetDefaultShot();
            } else if (m_HaveBegunIntroVO && TheMaster->GetAudio()->IsReady()) {
                if (m_ShoutoutSound) {
                    SynthSample *sample = m_ShoutoutSound->Sample();
                    float f23 = m_ShoutoutSound->ElapsedTime() - 1.0f;
                    if (f23 >= sample->LengthMs() / 1000.0f)
                        goto next;
                }
                TheMaster->GetAudio()->SetPaused(false);
                Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
                if (game) {
                    game->Handle(Message("set_realtime", 0), true);
                }
                SetNumMoves(mInitialMoveCount);
                TheHamProvider->SetProperty(holla_back_stage, exit_title);
                m_FirstInstruction = true;
                TheHamProvider->SetProperty("game_stage", Symbol("playing"));
                TheHamProvider->SetProperty("hide_venue", 0);
                SetState((State)1);
                SetDefaultShot();
            }
        }
    next:
        float curBeat = TheTaskMgr.Beat();
        HamMove *move =
            TheMoveMgr->FindHamMoveFromName(TheHamDirector->MoveNameFromBeat(curBeat, 0));
        if (mState == Title) {
            mHUDPanel->Find<UILabel>("song_name.lbl", true)
                ->SetPrelocalizedString(String("???"));
            mHUDPanel->Find<UILabel>("song_artist.lbl", true)
                ->SetPrelocalizedString(String("???"));
        }
    }
}

void HollaBackMinigame::BeginMinigame(DataArray *a) {
    if (!mActive) {
        m_iRoundOn = 0;
        m_WinLingerBeats = 0;
        m_HaveBegunIntroVO = false;
        static Symbol captured("captured");
        for (int i = 0; i < 64; i++) {
            mMoveStates[i] = captured;
        }
        TheHamDirector->StartStopVisualizer(false, 0);
        TheHamProvider->SetProperty("use_char_projection", 1);
        mStateBeats = 0;
        mMeasure = 0;
        m_PlayIntroVODelay = 10;
        mHaveStartedInstruction = false;
        TheGameData->Player(0);
        TheGameData->Player(1);
        mHaveDoneInitialShoutout = false;
        mInitialMoveCount = 2;
        mMaxRoutineSize = 4;
        mSpecifyFirstMoveMeasure = -1;
        mInitialMusicJump = -1;
        mIntroShoutout = Symbol("hb_intro_70s");
        mWinShoutouts.clear();
        mWinCamCuts.clear();
        mWinCamCuts.push_back("practice_intro_skills");
        static Symbol initial_move_count("initial_move_count");
        static Symbol max_routine_size("max_routine_size");
        static Symbol win_shoutouts("win_shoutouts");
        static Symbol win_camcuts("win_camcuts");
        static Symbol initial_music_jump("initial_music_jump");
        if (a) {
            a->FindData(initial_move_count, mInitialMoveCount, false);
            a->FindData(max_routine_size, mMaxRoutineSize, false);
            a->FindData("intro_shoutout", mIntroShoutout, false);
            DataArray *shoutoutArr = a->FindArray(win_shoutouts, false);
            if (shoutoutArr) {
                mWinShoutouts.clear();
                int numShoutouts = shoutoutArr->Size();
                mWinShoutouts.reserve(numShoutouts);
                for (int i = 1; i < numShoutouts; i++) {
                    mWinShoutouts.push_back(shoutoutArr->Sym(i));
                }
            }
            DataArray *camCutArr = a->FindArray(win_camcuts, false);
            if (camCutArr) {
                mWinCamCuts.clear();
                int numCamCuts = camCutArr->Size();
                mWinCamCuts.reserve(numCamCuts);
                for (int i = 1; i < numCamCuts; i++) {
                    mWinCamCuts.push_back(camCutArr->Sym(i));
                }
            }
            if (a->FindData(initial_music_jump, mInitialMusicJump, false)) {
                mInitialJumpPending = true;
            }
            a->FindData("specify_first_move_measure", mSpecifyFirstMoveMeasure, false);
            TheHamProvider->SetProperty("merge_moves", 0);
        }
        SetNumMoves(mInitialMoveCount);
        mActive = true;
        TheMaster->AddSink(this, "beat");
        mHUDPanel = DataVariable("hud_panel").Obj<PanelDir>();
        mHollabackHUD = mHUDPanel->Find<RndDir>("holla_back_hud");
        if (m_ShoutoutSound) {
            Flow *flow = mHollabackHUD->Find<Flow>("hide_shoutout.flow", false);
            if (flow) {
                flow->Activate();
            }
            m_ShoutoutSound->Stop(nullptr, false);
            m_ShoutoutSound = nullptr;
        }
        HamLabel *lbl = mHollabackHUD->Find<HamLabel>("shoutout.lbl");
        if (lbl) {
            lbl->SetPrelocalizedString(String(""));
        }
        static Symbol set_num_display("set_num_display");
        static Symbol set_card_move("set_card_move");
        static Symbol set_card_campaign_status_2("set_card_campaign_status_2");
        mFlashcardDockPanel = ObjectDir::Main()->Find<UIPanel>("flashcard_dock_panel");
        mFlashcardDock = mFlashcardDockPanel->DataDir();
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        mUniqueMoves.clear();
        for (int i = 0; i < mMaxRoutineSize; i++) {
            HamMove *move = theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + i);
            bool found = false;
            FOREACH (it, mUniqueMoves) {
                if (*it == move) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                mUniqueMoves.push_back(move);
            }
        }
        mFlashcardDockPanel->Handle(
            Message(set_num_display, (int)mUniqueMoves.size()), true
        );
        for (int i = 0; i < mUniqueMoves.size(); i++) {
            mFlashcardDockPanel->Handle(Message(set_card_move, i, mUniqueMoves[i]), true);
            mFlashcardDockPanel->Handle(
                Message(set_card_campaign_status_2, i, captured), true
            );
        }
        static Symbol horz_layout("horz_layout");
        int numMoves = mUniqueMoves.size();
        if (numMoves <= 4) {
            mFlashcardDock->Find<Flow>("horz_layout4.flow")->Activate();
        } else {
            mFlashcardDock->Find<Flow>("horz_layout6.flow")->Activate();
        }
        mDirScoreLeft = mHUDPanel->Find<RndDir>("score_left");
        mDirScoreRight = mHUDPanel->Find<RndDir>("score_right");
        mDirScoreLeft->SetShowing(false);
        mDirScoreRight->SetShowing(false);
        TheHamDirector->SetPlayerSpotlightsEnabled(false);
        mStartLoopMeasure = mSpecifyFirstMoveMeasure;
        mEndLoopMeasure = mSpecifyFirstMoveMeasure + mNumMoves;
        TheHamProvider->SetProperty("visible_flashcard_btm", mSpecifyFirstMoveMeasure);
        TheHamProvider->SetProperty("visible_flashcard_top", mEndLoopMeasure);
        TheHamProvider->SetProperty("hide_venue", 1);
        TheHamDirector->UnselectVisualizerPostProc();
        TheHamDirector->GetVenueWorld()->Find<Flow>("animate_timeywimey.flow")->Activate();
        static Symbol start_score_move_index("start_score_move_index");
        static Symbol hide_hud("hide_hud");
        for (int i = 0; i < 2; i++) {
            HamPlayerData *hpd = TheGameData->Player(i);
            if (!hpd->IsPlaying()) {
                hpd->Provider()->SetProperty(start_score_move_index, 1000);
                hpd->Provider()->Export(Message(hide_hud, 0), true);
            }
        }
        SetDefaultShot();
        mState = Unk; // I have no idea why it has -1...
        SetState((State)0);
    }
}

Symbol HollaBackMinigame::GetMoveState(int measure) const {
    static Symbol captured("captured");
    if (measure >= 0 && measure < 64) {
        return mMoveStates[measure];
    } else {
        MILO_NOTIFY(
            "HollaBackMinigame::GetMoveState(int measure = %d), measure not between 0 and 64",
            measure
        );
        return captured;
    }
}

void HollaBackMinigame::SetNumMoves(int num) {
    mNumMoves = num;
    TheHamProvider->SetProperty("visible_flashcard_btm", mSpecifyFirstMoveMeasure);
    TheHamProvider->SetProperty(
        "visible_flashcard_top", mSpecifyFirstMoveMeasure + num - 1
    );
    mStartLoopMeasure = mSpecifyFirstMoveMeasure - 4;
    mEndLoopMeasure = mSpecifyFirstMoveMeasure + mNumMoves + 1;
}

void HollaBackMinigame::StartShoutOut(const char *text) {
    HamLabel *label = mHollabackHUD->Find<HamLabel>("shoutout.lbl");
    if (label) {
        label->SetTextToken(Symbol(text));
    }
    m_ShoutoutSound = mHollabackHUD->Find<Sound>(MakeString("%s.snd", text), false);
    if (m_ShoutoutSound) {
        m_ShoutoutSound->Play(0, 0, 0, nullptr, 0);
    }
    Flow *flow = mHollabackHUD->Find<Flow>("show_shoutout.flow", false);
    if (flow) {
        flow->Activate();
    }
}

void HollaBackMinigame::SetDefaultShot() {
    if (TheGameData->Player(1)->IsPlaying()) {
        if (TheGameData->Player(0)->IsPlaying()) {
            if (TheGameData->Player(0)->Side() == kSkeletonRight) {
                TheHamDirector->Handle(
                    Message("force_shot", "practice_center_p21.shot"), true
                );
            } else {
                TheHamDirector->Handle(
                    Message("force_shot", "practice_center_p12.shot"), true
                );
            }
        } else {
            TheHamDirector->Handle(Message("force_shot", "practice_center_p2.shot"), true);
        }
    } else {
        TheHamDirector->Handle(Message("force_shot", "practice_center_p1.shot"), true);
    }
    RndPropAnim *anim = TheHamDirector->GetVenueWorld()->Find<RndPropAnim>(
        "bid_start_character_faded_out.anim"
    );
    anim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
}

void HollaBackMinigame::DecipherShoutOut(float pct) {
    if (pct < 0.25f) {
        StartShoutOut("hb_crazedeciphered0pct");
    } else if (pct < 0.33f) {
        StartShoutOut("hb_crazedeciphered25pct");
    } else if (pct < 0.5f) {
        StartShoutOut("hb_crazedeciphered33pct");
    } else if (pct < 0.66f) {
        StartShoutOut("hb_crazedeciphered50pct");
    } else if (pct < 0.75f) {
        StartShoutOut("hb_crazedeciphered66pct");
    } else {
        StartShoutOut("hb_crazedeciphered75pct");
    }
}

void HollaBackMinigame::WinShoutOut() {
    if (mWinShoutouts.size() > 0) {
        StartShoutOut(mWinShoutouts.front().Str());
    }
    if (mWinCamCuts.size() > 0) {
        TheHamDirector->Handle(Message("force_shot", mWinCamCuts.front().Str()), true);
    }
}

void HollaBackMinigame::EndShoutOut() {
    Flow *flow = mHollabackHUD->Find<Flow>("hide_shoutout.flow", false);
    if (flow) {
        flow->Activate();
    }
    m_ShoutoutSound = nullptr;
    if (mState == Instruction) {
        SetDefaultShot();
        if (!m_FirstInstruction) {
            TheHamProvider->Export(Message("show_char_projection"), true);
        }
    }
}

float HollaBackMinigame::NailedMovesInRoutinePct() {
    static Symbol powered_up("powered_up");
    int i5 = 0;
    int numMoves = mUniqueMoves.size();
    MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
    for (int i = 0; i < numMoves; i++) {
        HamMove *curMove = mUniqueMoves[i];
        for (int j = 0; j < mMaxRoutineSize; j++) {
            HamMove *moveAtMeasure =
                theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + j);
            if (moveAtMeasure == curMove
                && mMoveStates[mSpecifyFirstMoveMeasure + j] == powered_up) {
                i5++;
                for (int k = 0; k < mMaxRoutineSize; k++) {
                    HamMove *moveAtMeasure =
                        theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + k);
                    if (moveAtMeasure == curMove) {
                        mMoveStates[mSpecifyFirstMoveMeasure + j] = powered_up;
                    }
                }
            }
        }
    }
    return (float)i5 / (float)numMoves;
}

void HollaBackMinigame::EndMinigame(bool called_from_dtor) {
    if (mActive) {
        mActive = false;
        TheMaster->RemoveSink(this);
        TheHamProvider->RemoveSink(this);
        Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game_panel");
        game->RemoveSink(this);
        mHUDPanel->Find<Flow>("unset_flashcards_mystery.flow")->Activate();
        mHUDPanel->Find<UILabel>("song_name.lbl")->SetPrelocalizedString(String("???"));
        mHUDPanel->Find<UILabel>("song_artist.lbl")->SetPrelocalizedString(String("???"));
        TheHamProvider->SetProperty("visible_flashcard_btm", -1);
        TheHamProvider->SetProperty("visible_flashcard_top", -1);
        static Symbol clear_all_flashcard_campaign_status(
            "clear_all_flashcard_campaign_status"
        );
        mHUDPanel->Handle(Message(clear_all_flashcard_campaign_status), true);
        TheHamProvider->SetProperty("use_char_projection", 0);
        TheHamProvider->SetProperty("use_char_projectionp2", 0);
        TheHamDirector->Handle(Message("force_shot", Symbol("")), true);
        if (mDirScoreLeft) {
            mDirScoreLeft->SetShowing(true);
        }
        if (mDirScoreRight) {
            mDirScoreRight->SetShowing(true);
        }
    }
}

void HollaBackMinigame::SetMoveState(int measure, Symbol state) {
    if (measure >= 0 && measure < 64) {
        static Symbol powered_up("powered_up");
        static Symbol set_card_campaign_status_2("set_card_campaign_status_2");
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        HamMove *move = theMoveDir->GetMoveAtMeasure(0, measure);
        if (mMoveStates[measure] != state) {
            if (state == powered_up) {
                bool b2 = false;
                for (unsigned int i = mSpecifyFirstMoveMeasure;
                     i < mSpecifyFirstMoveMeasure + mMaxRoutineSize;
                     i++) {
                    HamMove *curMove = theMoveDir->GetMoveAtMeasure(0, i);
                    if (curMove == move && mMoveStates[i] == powered_up) {
                        b2 = true;
                    }
                }
                if (!b2) {
                    mFlashcardDockPanel->SetShowing(true);
                    unsigned int numMoves = mUniqueMoves.size();
                    for (int i = 0; i < numMoves; i++) {
                        if (move == mUniqueMoves[i]) {
                            mFlashcardDockPanel->Handle(
                                Message(set_card_campaign_status_2, i, powered_up), true
                            );
                            break;
                        }
                    }
                    mFlashcardDock->Find<Flow>("activate_popup.flow")->Activate();
                }
            }
            mMoveStates[measure] = state;
        }
    } else {
        MILO_NOTIFY(
            "HollaBackMinigame::SetMoveState(int measure = %d, Symbol state = '%s'), measure not between 0 and 64",
            measure,
            state.Str()
        );
    }
}

void HollaBackMinigame::SetState(State new_state) {
    if (mState != new_state) {
        static Symbol holla_back_stage("holla_back_stage");
        static Symbol enter_title("enter_title");
        static Symbol enter_instruction("enter_instruction");
        static Symbol enter_win("enter_win");
        static Symbol game_stage("game_stage");
        static Symbol intro("intro");
        static Symbol title("title");
        static Symbol playing("playing");
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        mState = new_state;
        mStateBeats = -1;
        switch (mState) {
        case 0:
            theMoveDir->ResetDetection();
            TheHamProvider->SetProperty(game_stage, title);
            mHUDPanel->Find<RndPropAnim>("song_overlay.anim", true)
                ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
            TheHamProvider->SetProperty(holla_back_stage, enter_title);
            break;
        case 1:
            TheHamProvider->SetProperty(game_stage, playing);
            mHUDPanel->Find<Flow>("unset_flashcards_mystery.flow", true)->Activate();
            TheHamProvider->SetProperty(holla_back_stage, enter_instruction);
            break;
        case 2: {
            TheHamProvider->SetProperty("game_stage", Symbol("outro"));
            TheHamProvider->SetProperty(holla_back_stage, enter_win);
            mTotalMeasures = 420;
            MidiParser *p = TheMidiParserMgr->GetParser("count_in_player");
            p->SetProperty("active", 0);
            TheMaster->GetAudio()->SetLoop(0, mTotalMeasures * 4.0f);
            break;
        }
        default:
            break;
        }
        OnBeat();
    }
}

void HollaBackMinigame::OnBeat() {
    static Symbol holla_back_stage("holla_back_stage");
    static Symbol exit_title("exit_title");
    static Symbol exit_instruction("exit_instruction");
    static Symbol exit_win("exit_win");
    MoveDir *moveDir = TheHamDirector->GetMoveDir();
    const SongPos &pos1 = TheMaster->Pos1();
    const SongPos &pos2 = TheMaster->Pos2();
    int i98, i94;
    TheMaster->GetAudio()->GetCurrLoopBeats(i98, i94);

    float beat1;
    if ((beat1 = pos1.GetTotalBeat()) < pos2.GetTotalBeat()) {
        float beat1_2 = pos1.GetTotalBeat();
        TheMidiParserMgr->GetParser("midi_player")
            ->Handle(Message("reset_to_beat", pos1.GetTotalBeat()), true);
        TheMidiParserMgr->GetParser("count_in_player")
            ->Handle(Message("reset_to_beat", pos1.GetTotalBeat()), true);
        moveDir->ResetDetection();
    }
    int iBeat1 = beat1;
    mMeasure = iBeat1 / 4;
    if (mSpecifyFirstMoveMeasure > 0 && mState == 1) {
        mStateBeats = (4 - mSpecifyFirstMoveMeasure) * 4 + iBeat1;
    } else {
        if (mState != 0) {
            mStateBeats++;
        }
    }
    int i18 = mStateBeats / 4;
    int i17 = mStateBeats % 4;
    int i19 = iBeat1 % 4;
    if (mStateBeats >= 0 && i17 != i19) {
        int sub = i19 - i17;
        mStateBeats += sub;
    }
    bool i19IsZero = i19 == 0;
    if (mState == 1) {
        if (m_FirstInstruction && i18 == 3 && i19 == 0) {
            m_FirstInstruction = false;
            WorldDir *world = TheHamDirector->GetVenueWorld();
            world->Find<RndPropAnim>("bid_start_character_faded_out.anim")
                ->Animate(0, false, 0);
            TheHamProvider->Export(Message("show_char_projection"), true);
        }
        if (i18 == 3 && i19 == 3) {
            MidiParser *parser = TheMidiParserMgr->GetParser("count_in_player");
            parser->SetProperty("active", false);
        }

        if (i19IsZero || i18 < 0) {
            if (i18 == 1) {
                MidiParser *parser = TheMidiParserMgr->GetParser("count_in_player");
                parser->SetProperty("active", true);
                parser->Handle(
                    Message(
                        "set_section",
                        mSpecifyFirstMoveMeasure * 4 - 4,
                        mSpecifyFirstMoveMeasure * 4
                    ),
                    true
                );
            }
            if (i18 == 2) {
                TheMaster->GetAudio()->SetLoop(
                    (float)(mStartLoopMeasure + 1) * 4, (float)mEndLoopMeasure * 4
                );
            }
            static int sInt8a4 = 4;
            if (i18 == sInt8a4) {
                mHaveStartedInstruction = true;
                TheHamDirector->SetPlayerSpotlightsEnabled(true);
                TheHamProvider->SetProperty(holla_back_stage, exit_instruction);
            }
            bool b5 =
                (mHaveStartedInstruction
                 && (mMeasure == mSpecifyFirstMoveMeasure - 3
                     || mMeasure == mEndLoopMeasure));
            bool b4 =
                (mHaveStartedInstruction
                 && (mMeasure == mSpecifyFirstMoveMeasure - 3
                     || mMeasure == mEndLoopMeasure - 1));
            if (b4) {
                TheHamProvider->Export(Message("hide_char_projection"), true);
            }
            if (b5) {
                TheHamDirector->SetPlayerSpotlightsEnabled(false);
                float pct = NailedMovesInRoutinePct();
                mNailedLast = pct >= 1;
                if (mNailedLast) {
                    mDirScoreLeft->SetShowing(true);
                    mDirScoreRight->SetShowing(true);
                    WinShoutOut();
                    ObjectDir::Main()->Find<Hmx::Object>("songseq")->Handle(
                        Message("load_next_song_audio"), true
                    );
                    for (int i = 0; i < 2; i++) {
                        Hmx::Object *prov = TheGameData->Player(i)->Provider();
                        prov->SetProperty("start_score_move_index", 1000);
                        prov->Export(Message("hide_hud", false), true);
                    }
                    if (m_iRoundOn == 0) {
                        Hmx::Object *gamePanel =
                            ObjectDir::Main()->Find<Hmx::Object>("game_panel");
                        gamePanel->Handle(
                            Message("earn_acc", Symbol("acc_hollaback")), true
                        );
                    }
                    SetState((State)2);
                } else {
                    DecipherShoutOut(pct);
                    m_iRoundOn++;
                    SetState((State)1);
                }
            }
        }

    } else if (mState == 2) {
        if (m_ShoutoutSound && m_ShoutoutSound->IsPlaying()) {
            return;
        }
        if (mWinShoutouts.size() > 1) {
            const char *name = MakeString("%s.snd", mWinShoutouts[1].Str());
            WorldDir *world = TheHamDirector->GetVenueWorld();
            m_ShoutoutSound = world->Find<Sound>(name, false);
            if (m_ShoutoutSound) {
                m_ShoutoutSound->Play(0, 0, 0, nullptr, 0);
            }
        }
        TheHamDirector->SetPlayerSpotlightsEnabled(true);
        ObjectDir::Main()
            ->Find<Hmx::Object>("game_panel")
            ->Handle(Message("win_hollaback"), true);
    }
}
