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
#include "utl/Std.h"
#include "utl/TimeConversion.h"

void JumpToMeasure(float beat) {
    float ms = BeatToMs(beat * 4.0f);
    Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game", true);
    if (game) {
        game->Handle(Message("jump", ms), true);
    }
}

HollaBackMinigame::HollaBackMinigame()
    : unk410(0), mSpecifyFirstMoveMeasure(-1), mInitialMoveCount(2), unk450(0), unk474(0),
      mSound(0) {}

HollaBackMinigame::~HollaBackMinigame() { EndMinigame(true); }

BEGIN_HANDLERS(HollaBackMinigame)
    HANDLE_ACTION(begin_minigame, BeginMinigame(_msg->Array(2)))
    HANDLE_ACTION(end_minigame, EndMinigame(false))
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_EXPR(get_move_state, GetMoveState(_msg->Int(2)))
    HANDLE_ACTION(set_move_state, SetMoveState(_msg->Int(2), _msg->Sym(3)))
    HANDLE_ACTION(set_default_shot, SetDefaultShot())
    HANDLE_EXPR(get_first_move_idx, unk8 + 5)
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

BEGIN_LOADS(HollaBackMinigame)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(RndPollable)
END_LOADS

void HollaBackMinigame::Poll() {
    if (!TheLoadMgr.EditMode() && unk410) {
        RndPropAnim *anim =
            TheHamDirector->GetVenueWorld()->Find<RndPropAnim>("set_bid.anim", true);
        if (anim) {
            anim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
        }

        if (mSound && !mSound->IsPlaying()) {
            EndShoutOut();
        }
        if (unk474) {
            Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game", true);
            bool b20 = !game ? false : !game->Handle(Message("is_waiting"), true).Int();
            if (b20 && unk474) {
                JumpToMeasure(mInitialMusicJump);
                TheMaster->GetAudio()->SetPaused(true);
                unk474 = false;
            }
        }
        if (mState == 0) {
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
                                                          : "player_2_hud.anim",
                            true
                        )
                        ->SetFrame(0, 1);
                }
            }
            if (!unk481 && TheMaster->GetAudio()->IsReady() && unk494-- <= 0) {
                unk481 = true;
                TheHamProvider->SetProperty(holla_back_stage, Symbol("title"));
                Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game", true);
                if (game) {
                    game->Handle(Message("set_realtime", 1), true);
                }
                mSound = mHollabackHUD->Find<Sound>(
                    MakeString("%s.snd", mIntroShoutout.Str()), false
                );
                if (mSound) {
                    mSound->Play(0, 0, 0, nullptr, 0);
                }
                TheHamProvider->SetProperty("game_stage", Symbol("title"));
                TheHamDirector->GetVenueWorld()
                    ->Find<Flow>("animate_timeywimey.flow", true)
                    ->Activate();
                SetDefaultShot();
            } else if (unk481 && TheMaster->GetAudio()->IsReady()) {
                if (mSound) {
                    SynthSample *sample = mSound->Sample();
                    float f23 = mSound->ElapsedTime() - 1.0f;
                    if (f23 >= sample->LengthMs() / 1000.0f)
                        goto next;
                }
                TheMaster->GetAudio()->SetPaused(false);
                Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game", true);
                if (game) {
                    game->Handle(Message("set_realtime", 0), true);
                }
                SetNumMoves(mInitialMoveCount);
                TheHamProvider->SetProperty(holla_back_stage, exit_title);
                unk480 = true;
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
        if (mState == 0) {
            mHUDPanel->Find<UILabel>("song_name.lbl", true)
                ->SetPrelocalizedString(String("???"));
            mHUDPanel->Find<UILabel>("song_artist.lbl", true)
                ->SetPrelocalizedString(String("???"));
        }
    }
}

void HollaBackMinigame::BeginMinigame(DataArray *a) {
    if (!unk410) {
        unk484 = 0;
        unk47c = 0;
        unk481 = false;
        static Symbol captured("captured");
        for (int i = 0; i < 0x40; i++) {
            unk10[i] = captured;
        }
        TheHamDirector->StartStopVisualizer(false, 0);
        TheHamProvider->SetProperty("use_char_projection", 1);
        unk418 = 0;
        unk420 = 0;
        unk494 = 10;
        unk46c = false;
        TheGameData->Player(0);
        TheGameData->Player(1);
        unk444 = false;
        mInitialMoveCount = 2;
        mSpecifyFirstMoveMeasure = -1;
        mMaxRoutineSize = 4;
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
                unk474 = true;
            }
            a->FindData("specify_first_move_measure", mSpecifyFirstMoveMeasure, false);
            TheHamProvider->SetProperty("merge_moves", 0);
        }
        SetNumMoves(mInitialMoveCount);
        unk410 = true;
        TheMaster->AddSink(this, "beat");
        mHUDPanel = DataVariable("hud_panel").Obj<PanelDir>();
        mHollabackHUD = mHUDPanel->Find<RndDir>("holla_back_hud", true);
        if (mSound) {
            Flow *flow = mHollabackHUD->Find<Flow>("hide_shoutout.flow", false);
            if (flow) {
                flow->Activate();
            }
            mSound->Stop(nullptr, false);
            mSound = nullptr;
        }
        HamLabel *lbl = mHollabackHUD->Find<HamLabel>("shoutout.lbl", true);
        if (lbl) {
            lbl->SetPrelocalizedString(String(""));
        }
        static Symbol set_num_display("set_num_display");
        static Symbol set_card_move("set_card_move");
        static Symbol set_card_campaign_status_2("set_card_campaign_status_2");
        mFlashcardDockPanel =
            ObjectDir::Main()->Find<UIPanel>("flashcard_dock_panel", true);
        unk460 = mFlashcardDockPanel->DataDir();
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        unk488.clear();
        for (int i = 0; i < mMaxRoutineSize; i++) {
            HamMove *move = theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + i);
            bool found = false;
            FOREACH (it, unk488) {
                if (*it == move) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                unk488.push_back(move);
            }
        }
        mFlashcardDockPanel->Handle(Message(set_num_display, (int)unk488.size()), true);
        for (int i = 0; i < unk488.size(); i++) {
            mFlashcardDockPanel->Handle(Message(set_card_move, i, unk488[i]), true);
            mFlashcardDockPanel->Handle(
                Message(set_card_campaign_status_2, i, captured), true
            );
        }
        static Symbol horz_layout("horz_layout");
        int numMoves = unk488.size();
        if (numMoves <= 4) {
            unk460->Find<Flow>("horz_layout4.flow", true)->Activate();
        } else {
            unk460->Find<Flow>("horz_layout6.flow", true)->Activate();
        }
        mScoreLeft = mHUDPanel->Find<RndDir>("score_left", true);
        mScoreRight = mHUDPanel->Find<RndDir>("score_right", true);
        mScoreLeft->SetShowing(false);
        mScoreRight->SetShowing(false);
        TheHamDirector->SetPlayerSpotlightsEnabled(false);
        unk8 = mSpecifyFirstMoveMeasure;
        unkc = mSpecifyFirstMoveMeasure + mNumMoves;
        TheHamProvider->SetProperty("visible_flashcard_btm", mSpecifyFirstMoveMeasure);
        TheHamProvider->SetProperty("visible_flashcard_top", unkc);
        TheHamProvider->SetProperty("hide_venue", 1);
        TheHamDirector->UnselectVisualizerPostProc();
        TheHamDirector->GetVenueWorld()
            ->Find<Flow>("animate_timeywimey.flow", true)
            ->Activate();
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
        mState = -1;
        SetState((State)0);
    }
}

Symbol HollaBackMinigame::GetMoveState(int measure) const {
    static Symbol captured("captured");
    if (measure >= 0 && measure < 64) {
        return unk10[measure];
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
    unk8 = mSpecifyFirstMoveMeasure - 4;
    unkc = mSpecifyFirstMoveMeasure + mNumMoves + 1;
}

void HollaBackMinigame::StartShoutOut(const char *cc) {
    HamLabel *label = mHollabackHUD->Find<HamLabel>("shoutout.lbl", true);
    if (label) {
        label->SetTextToken(Symbol(cc));
    }
    mSound = mHollabackHUD->Find<Sound>(MakeString("%s.snd", cc), false);
    if (mSound) {
        mSound->Play(0, 0, 0, nullptr, 0);
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
        "bid_start_character_faded_out.anim", true
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
    if (mWinShoutouts.size()) {
        StartShoutOut(mWinShoutouts.front().Str());
    }
    if (mWinCamCuts.size()) {
        TheHamDirector->Handle(Message("force_shot", mWinCamCuts.front().Str()), true);
    }
}

void HollaBackMinigame::EndShoutOut() {
    Flow *flow = mHollabackHUD->Find<Flow>("hide_shoutout.flow", false);
    if (flow) {
        flow->Activate();
    }
    mSound = nullptr;
    if (mState == 1) {
        SetDefaultShot();
        if (!unk480) {
            TheHamProvider->Export(Message("show_char_projection"), true);
        }
    }
}

float HollaBackMinigame::NailedMovesInRoutinePct() {
    static Symbol powered_up("powered_up");
    int i5 = 0;
    int numMoves = unk488.size();
    MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
    for (int i = 0; i < numMoves; i++) {
        HamMove *curMove = unk488[i];
        for (int j = 0; j < mMaxRoutineSize; j++) {
            HamMove *moveAtMeasure =
                theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + j);
            if (moveAtMeasure == curMove
                && unk10[mSpecifyFirstMoveMeasure + j] == powered_up) {
                i5++;
                for (int k = 0; k < mMaxRoutineSize; k++) {
                    HamMove *moveAtMeasure =
                        theMoveDir->GetMoveAtMeasure(0, mSpecifyFirstMoveMeasure + k);
                    if (moveAtMeasure == curMove) {
                        unk10[mSpecifyFirstMoveMeasure + j] = powered_up;
                    }
                }
            }
        }
    }
    return (float)i5 / (float)numMoves;
}

void HollaBackMinigame::EndMinigame(bool b1) {
    if (unk410) {
        unk410 = false;
        TheMaster->RemoveSink(this);
        TheHamProvider->RemoveSink(this);
        Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game_panel", true);
        game->RemoveSink(this);
        mHUDPanel->Find<Flow>("unset_flashcards_mystery.flow", true)->Activate();
        mHUDPanel->Find<UILabel>("song_name.lbl", true)
            ->SetPrelocalizedString(String("???"));
        mHUDPanel->Find<UILabel>("song_artist.lbl", true)
            ->SetPrelocalizedString(String("???"));
        TheHamProvider->SetProperty("visible_flashcard_btm", -1);
        TheHamProvider->SetProperty("visible_flashcard_top", -1);
        static Symbol clear_all_flashcard_campaign_status(
            "clear_all_flashcard_campaign_status"
        );
        mHUDPanel->Handle(Message(clear_all_flashcard_campaign_status), true);
        TheHamProvider->SetProperty("use_char_projection", 0);
        TheHamProvider->SetProperty("use_char_projectionp2", 0);
        TheHamDirector->Handle(Message("force_shot", Symbol("")), true);
        if (mScoreLeft) {
            mScoreLeft->SetShowing(true);
        }
        if (mScoreRight) {
            mScoreRight->SetShowing(true);
        }
    }
}

void HollaBackMinigame::SetMoveState(int measure, Symbol state) {
    if (measure >= 0 && measure < 0x40) {
        static Symbol powered_up("powered_up");
        static Symbol set_card_campaign_status_2("set_card_campaign_status_2");
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        HamMove *move = theMoveDir->GetMoveAtMeasure(0, measure);
        if (unk10[measure] != state) {
            if (state == powered_up) {
                bool b2 = false;
                for (int i = mSpecifyFirstMoveMeasure;
                     i < mSpecifyFirstMoveMeasure + mMaxRoutineSize;
                     i++) {
                    HamMove *curMove = theMoveDir->GetMoveAtMeasure(0, i);
                    if (curMove == move && unk10[i] == powered_up) {
                        b2 = true;
                    }
                }
                if (!b2) {
                    mFlashcardDockPanel->SetShowing(true);
                    int numMoves = unk488.size();
                    for (int i = 0; i < numMoves; i++) {
                        if (move == unk488[i]) {
                            mFlashcardDockPanel->Handle(
                                Message(set_card_campaign_status_2, i, powered_up), true
                            );
                            break;
                        }
                    }
                    unk460->Find<Flow>("activate_popup.flow", true)->Activate();
                }
            }
            unk10[measure] = state;
        }
    } else {
        MILO_NOTIFY(
            "HollaBackMinigame::SetMoveState(int measure = %d, Symbol state = '%s'), measure not between 0 and 64",
            measure,
            state.Str()
        );
    }
}

void HollaBackMinigame::SetState(State s) {
    if (mState != s) {
        static Symbol holla_back_stage("holla_back_stage");
        static Symbol enter_title("enter_title");
        static Symbol enter_instruction("enter_instruction");
        static Symbol enter_win("enter_win");
        static Symbol game_stage("game_stage");
        static Symbol intro("intro");
        static Symbol title("title");
        static Symbol playing("playing");
        MoveDir *theMoveDir = TheHamDirector->GetMoveDir();
        mState = s;
        unk418 = -1;
        switch (mState) {
        case -1:
        case 0:
            theMoveDir->ResetDetection();
            TheHamProvider->SetProperty(game_stage, title);
            mHUDPanel->Find<RndPropAnim>("song_overlay.anim", true)
                ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
            TheHamProvider->SetProperty(holla_back_stage, enter_title);
            OnBeat();
            break;
        case 1:
            TheHamProvider->SetProperty(game_stage, playing);
            mHUDPanel->Find<Flow>("unset_flashcards_mystery.flow", true)->Activate();
            TheHamProvider->SetProperty(holla_back_stage, enter_instruction);
            OnBeat();
            break;
        case 2: {
            TheHamProvider->SetProperty("game_stage", Symbol("outro"));
            TheHamProvider->SetProperty(holla_back_stage, enter_win);
            unk450 = 420;
            MidiParser *p = TheMidiParserMgr->GetParser("count_in_player");
            p->SetProperty("active", 0);
            TheMaster->GetAudio()->SetLoop(0, unk450 * 4.0f);
            OnBeat();
            break;
        }
        default:
            OnBeat();
            break;
        }
    }
}
