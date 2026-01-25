#include "BustAMovePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/GamePanel.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DepthBuffer3D.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/ScoreUtl.h"
#include "lazer/game/GameMode.h"
#include "lazer/meta_ham/HamPanel.h"
#include "math/Easing.h"
#include "math/Rand.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "ui/UIColor.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"
#include <float.h>

namespace {
    void GetShuffledInts(std::vector<int> &, int);

}

BustAMovePanel::BustAMovePanel()
    : unk40(0), unk44(0), unk58(-1), unk5c(0), mHUDPanel(0), unk64(0), unk68(4), unk6c(0),
      unk70(10), unk80(0), unka0(1), unk92c(0), unk930(0), unk934(0), unk958(-1),
      unk95c(-1), unk968(-1), unk970(0), unk988(0), unk98c(0), unk99c(0), unk9a0(FLT_MAX),
      unk9b9(0), unk9bc(-1) {
    unk40 = new FreestyleMoveRecorder();
    unk40->AssignStaticInstance();
}

BustAMovePanel::~BustAMovePanel() { delete unk40; }

BEGIN_HANDLERS(BustAMovePanel)
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_ACTION(cache_objects, CacheObjects())
    HANDLE_ACTION(set_up_song_structure, SetUpSongStructure(_msg->Sym(2)))
    HANDLE_ACTION(on_stream_jump, unk98c = true)
    HANDLE_ACTION(play_intro_vo, PlayIntroVO())
    HANDLE_SUPERCLASS(HamPanel)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BustAMovePanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void BustAMovePanel::Draw() {
    UIPanel::Draw();
    unk40->DrawDebug();
    if (unk934) {
        RndDir *renderer = DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
        String flashcard(MakeString("flashcard%i.tex", unk84));
        RndTexRenderer *texRenderer =
            renderer->Find<RndTexRenderer>("TexRenderer.rndtex");
        int numPoses = 0;
        for (int i = 0; i < 3; i++) {
            if (unka4[i].Tracked()) {
                numPoses++;
                MILO_ASSERT(numPoses <= 2, 0x1BC);
                String pose(MakeString("pose%i.tex", numPoses));
                RndDir *renderer =
                    DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
                RndTex *tex = renderer->Find<RndTex>(pose.c_str());
                TheHamDirector->PoseIconMan(&unka4[i], tex);
            }
        }
        if (numPoses == 0) {
            unk9b9 = true;
        }
        RndAnimatable *anim = renderer->Find<RndAnimatable>("num_poses.anim");
        anim->SetFrame(numPoses, 1);
        texRenderer->SetOutputTexture(DataDir()->Find<RndTex>(flashcard.c_str()));
        renderer->DrawShowing();
        unk934--;
    }
}

void BustAMovePanel::Enter() {
    HamPanel::Enter();
    mHUDPanel = 0;
    if (InBustAMove()) {
        CacheObjects();
        unk9b8 = true;
    }
}

void BustAMovePanel::Exit() {
    UIPanel::Exit();
    TheMaster->RemoveSink(this);
    if (unk40) {
        unk40->Free();
    }
    TheHamDirector->SetPlayerSpotlightsEnabled(true);
}

bool BustAMovePanel::InBustAMove() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol bustamove("bustamove");
    return TheGameMode->Property(gameplay_mode)->Sym() == bustamove;
}

Symbol BustAMovePanel::GetPlayerColor(int i1) {
    static Symbol is_in_party_mode("is_in_party_mode");
    if (TheHamProvider->Property(is_in_party_mode)->Int()) {
        if (TheGameData->Player(i1)->Side() == kSkeletonRight) {
            return "pink";
        } else {
            return "blue";
        }
    }
    if (i1 == 0) {
        return "pink";
    } else {
        return "blue";
    }
}

MoveRating BustAMovePanel::GetMoveRating(float f1) {
    if (f1 > 0.85f) {
        return kMoveRatingSuperPerfect;
    } else if (f1 > 0.70f) {
        return kMoveRatingPerfect;
    } else if (f1 > 0.4f) {
        return kMoveRatingAwesome;
    } else {
        return kMoveRatingOk;
    }
}

void BustAMovePanel::SetFlashcardText(int side, int index, Symbol s3) {
    HamLabel *label =
        mBAMColumns[side]->Find<HamLabel>(MakeString("flashcard_%d.lbl", index));
    label->SetTextToken(s3);
    label = mBAMColumns[side == 0]->Find<HamLabel>(MakeString("flashcard_%d.lbl", index));
    if (mState == kBAMState_ShowMoveSequence
        || mState == kBAMState_ShowMoveSequenceSetup) {
        label->SetTextToken(s3);
    } else {
        label->SetTextToken(gNullStr);
    }
}

DataArray *BustAMovePanel::GetMoveNameData(int index) {
    static Symbol bustamove_move_names("bustamove_move_names");
    MILO_ASSERT(index >= 0 && index < MAX_FREESTYLE_MOVES, 0x656);
    int nameIndex = unk93c[index];
    MILO_ASSERT(nameIndex >= 0 && nameIndex < mShuffledMoveNames.size(), 0x658);
    DataArray *arr = TheGamePanel->Property(bustamove_move_names)->Array();
    return arr->Array(nameIndex);
}

void BustAMovePanel::SetMovePrompt() {
    Symbol sym = GetMoveNameData(unk84)->Sym(0);
    mMovePromptLabel->SetTextToken(sym);
    UIColor *movePromptColor = DataDir()->Find<UIColor>("move_prompt.color");
    UIColor *playerColor =
        DataDir()->Find<UIColor>(MakeString("%s.color", GetPlayerColor(unk64)));
    Hmx::Color color = playerColor->GetColor();
    movePromptColor->SetColor(color);
}

void BustAMovePanel::IncreaseScore(int player, int scoreToAdd) {
    static Symbol score("score");
    int oldScore = TheGameData->Player(player)->Provider()->Property(score)->Int();
    TheGameData->Player(player)->Provider()->SetProperty(score, oldScore + scoreToAdd);
}

void BustAMovePanel::ResetScores() {
    static Symbol score("score");
    TheGameData->Player(0)->Provider()->SetProperty(score, 0);
    TheGameData->Player(1)->Provider()->SetProperty(score, 0);
}

void BustAMovePanel::SetFlashcardName(int side, int index, int i3) {
    Symbol s(gNullStr);
    if (i3 >= 0) {
        s = GetMoveNameData(i3)->Sym(1);
    }
    HamLabel *label =
        mBAMColumns[side]->Find<HamLabel>(MakeString("flashcard_name_%d.lbl", index));
    label->SetTextToken(s);
    label = mBAMColumns[side == 0]->Find<HamLabel>(
        MakeString("flashcard_name_%d.lbl", index)
    );
    if (mState == kBAMState_ShowMoveSequence
        || mState == kBAMState_ShowMoveSequenceSetup) {
        label->SetTextToken(s);
    } else {
        label->SetTextToken(gNullStr);
    }
}

void BustAMovePanel::CountIn(int i1) {
    int f4 = (int)(TheTaskMgr.Beat() + 0.5f) + i1;
    static Message countInMsg("count_in", 0, 0);
    countInMsg[0] = f4;
    countInMsg[1] = f4;
    Handle(countInMsg, true);
}

void BustAMovePanel::ShowMoveRating(MoveRating mr, int side) {
    const char *sideStr = side == 0 ? "left" : "right";
    RndDir *feedbackDir =
        DataDir()->Find<RndDir>(MakeString("bustamove_text_feedback_%s", sideStr));
    static Symbol move_perfect("move_perfect");
    static Symbol move_awesome("move_awesome");
    static Symbol move_ok("move_ok");
    static Symbol move_bad("move_bad");
    static Message moveFinishedMsg("move_finished", 0, 0);
    moveFinishedMsg[0] = side;
    if (mr == kMoveRatingSuperPerfect) {
        RndAnimatable *anim =
            feedbackDir->Find<RndAnimatable>("move_flawless_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_perfect;
    } else if (mr == kMoveRatingPerfect) {
        RndAnimatable *anim = feedbackDir->Find<RndAnimatable>("move_nice_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_awesome;
    } else if (mr == kMoveRatingAwesome) {
        RndAnimatable *anim = feedbackDir->Find<RndAnimatable>("move_okay_right.anim");
        anim->Animate(0, 0, 0, nullptr, kEaseLinear, 0, false);
        moveFinishedMsg[1] = move_ok;
    } else if (mr == kMoveRatingOk) {
        moveFinishedMsg[1] = move_bad;
    }
    TheHamProvider->Handle(moveFinishedMsg, false);
}

void BustAMovePanel::SetRoundFailure() {
    static Message resultMessage("set_bustamove_result", 0, 0, 0);
    resultMessage[0] = unk64 == 0;
    resultMessage[1] = 0;
    resultMessage[2] = 0;
    TheHamProvider->Handle(resultMessage, false);
}

void BustAMovePanel::PlayMovePromptVO() {
    PlayVO(GetMoveNameData(unk84)->Sym(unka0 + 2));
}

float BustAMovePanel::GetMovePromptVOLength() {
    float len = 0;
    Symbol sym = GetMoveNameData(unk84)->Sym(unka0 + 2);
    static Message voLengthMsg("get_seq_length", 0);
    voLengthMsg[0] = sym;
    DataNode handled = mHUDPanel->Handle(voLengthMsg, true);
    if (handled != DATA_UNHANDLED) {
        len = handled.Float();
    }
    return len;
}

void BustAMovePanel::ShowGetReadyCard(Symbol s, SkeletonSide side) {
    mBAMColumns[side]->Find<HamLabel>("get_ready.lbl")->SetTextToken(s);
    static Message getReadyMsg("bustamove_get_ready", 0);
    getReadyMsg[0] = side;
    TheHamProvider->Handle(getReadyMsg, false);
}

void BustAMovePanel::CacheObjects() {
    mBAMVisualizerPanel = ObjectDir::Main()->Find<HamPanel>("bustamove_visualizer_panel");
    mBAMVisualizerPanel->DataDir()
        ->Find<RndAnimatable>("num_players.anim")
        ->SetFrame(1, 1);
    for (ObjDirItr<DepthBuffer3D> it(mBAMVisualizerPanel->DataDir(), true); it != nullptr;
         ++it) {
        //   while (local_6c != (DepthBuffer3D *)0x0) {
        //     DepthBuffer3D::SetGrooviness(local_6c,(float)dVar23);
        //     if (*(int *)(local_6c + 0x198) != 0) {
        //       *(undefined4 *)(*(int *)(local_6c + 0x194) + 4) = *(undefined4
        //       *)(local_6c + 400);
        //       *(undefined4 *)(*(int *)(local_6c + 400) + 8) = *(undefined4 *)(local_6c
        //       + 0x194);
        //     }
        //     *(undefined4 *)(local_6c + 0x198) = 0;
        //     ObjDirItr<>::operator++(aOStack_70);
        //   }
    }
    TheMaster->AddSink(this, "beat");
    mStatusLabel = DataDir()->Find<HamLabel>("status.lbl");
    mMovePromptLabel = DataDir()->Find<HamLabel>("move_prompt.lbl");
    mStatusLabel->SetTextToken(gNullStr);
    mMovePromptLabel->SetTextToken(gNullStr);
    if (SystemLanguage() == "jpn" || SystemLanguage() == "kor"
        || SystemLanguage() == "cht") {
        DataDir()
            ->Find<RndAnimatable>("asian_prompt_adjust.anim")
            ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    }
    mState = kBAMState_CountIn;
    unk44 = 0;
    unk6c = 0;
    unk84 = 0;
    unk64 = RandomInt(0, 2);
    unk7c = false;
    mHUDPanel = DataVariable("hud_panel").Obj<ObjectDir>();
    for (int i = 0; i < 4; i++) {
        String flashcard = MakeString("flashcard_slot%i.mat", i);
        RndMat *flashcardMat = DataDir()->Find<RndMat>(flashcard.c_str());
        RndTex *blank = DataDir()->Find<RndTex>("blank.tex");
        flashcardMat->SetDiffuseTex(blank);
    }
    mBAMColumns[kSkeletonRight] = DataDir()->Find<RndDir>("bustamove_column_right");
    mBAMColumns[kSkeletonLeft] = DataDir()->Find<RndDir>("bustamove_column_left");
    unk48.clear();
    unk50.clear();
    ResetScores();
    unk954 = 1;
    unk950 = 0;
    unk94c = 0;
    mPhraseMeters[kSkeletonRight] = DataDir()->Find<HamPhraseMeter>("phrase_meter_right");
    mPhraseMeters[kSkeletonLeft] = DataDir()->Find<HamPhraseMeter>("phrase_meter_left");
    unk9a0 = FLT_MAX;
    DataDir()->Find<RndAnimatable>("num_players.anim")->SetFrame(1, 1);
    for (int i = 0; i < 4; i++) {
        String flashcardSlot = MakeString("flashcard_slot%i.lbl", i);
        HamLabel *label = DataDir()->Find<HamLabel>(flashcardSlot.c_str());
        label->SetTextToken(gNullStr);
        String flashcardSlotBG = MakeString("flashcard_slot_background%i.mat", i);
        RndMat *mat = DataDir()->Find<RndMat>(flashcardSlotBG.c_str());
        UIColor *gray = DataDir()->Find<UIColor>("gray.color");
        const Hmx::Color &color = gray->GetColor();
        mat->SetColor(color.red, color.green, color.blue);
    }
    Symbol song = MetaPerformer::Current()->GetSong();
    unk9bc = -1;
}

void BustAMovePanel::SetUpMoveNames() {
    mShuffledMoveNames.clear();
    static Symbol bustamove_move_names("bustamove_move_names");
    GetShuffledInts(
        mShuffledMoveNames, TheGamePanel->Property(bustamove_move_names)->Array()->Size()
    );
}

void BustAMovePanel::PlayVO(Symbol s) {
    static Message playVOMsg("play", 0);
    playVOMsg[0] = s;
    mHUDPanel->Handle(playVOMsg, true);
}

void BustAMovePanel::QueueMovePromptVO() {
    float voLength = GetMovePromptVOLength();
    TempoMap *tempoMap = TheMaster->SongData()->GetTempoMap();
    float bpm = tempoMap->GetTempoBPM(0);
    float secondsPerBeat = 60.0f / bpm;
    int reps = unk988;
    float beatsToWait = (float)((reps * 4) - 4);
    float timeOffset = beatsToWait * secondsPerBeat;
    float currentTime = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    unk9a0 = currentTime + timeOffset - voLength - 1.0f;
}

void BustAMovePanel::PollCaptureFlashcard() {
    if (unk92c != 0) {
        float flashcardTweak = 0.17f;
        if (DataVarExists("flashcard_tweak")) {
            flashcardTweak = DataVariable("flashcard_tweak").Float();
        }
        if (unk930 >= flashcardTweak) {
            BaseSkeleton *liveSkel = unk40->GetLiveSkeleton();
            if (liveSkel != nullptr) {
                unka4[unk92c - 1].Set(*unk40->GetLiveSkeleton());
            } else {
                unka4[unk92c - 1].SetTracked(false);
            }
            if (unk92c == 3) {
                float score1 =
                    unk40->CompareSkeletonPositions(&unka4[0], &unka4[1], 1.0f);
                float score2 =
                    unk40->CompareSkeletonPositions(&unka4[0], &unka4[2], 1.0f);
                if (score2 < 0.5f) {
                    unka4[1].SetTracked(false);
                } else {
                    unka4[2].SetTracked(false);
                }
                if (score1 >= 0.5f) {
                    unka4[1].SetTracked(false);
                }
                unk934 = 4;
            }
            unk92c = 0;
        } else {
            unk930 += TheTaskMgr.DeltaUISeconds();
        }
    }
}

void BustAMovePanel::AnimateFlashcard(int) {}
void BustAMovePanel::AdvanceFlashcards() {}
int BustAMovePanel::RepsToNextPhrase() { return 0; }

void BustAMovePanel::SetFlashcardImage(int side, int index, int i3) {
    RndMat *flashcardMat =
        mBAMColumns[side]->Find<RndMat>(MakeString("flashcard%d.mat", index));
    RndMat *flashcardBgMat =
        mBAMColumns[side]->Find<RndMat>(MakeString("flashcard_background%d.mat", index));
    RndTex *blankTex = DataDir()->Find<RndTex>("blank.tex");

    RndTex *flashcardTex;
    RndTex *bgTex;
    if (i3 >= 0) {
        flashcardTex = DataDir()->Find<RndTex>(MakeString("flashcard%i.tex", i3));
    } else if (i3 == -2) {
        flashcardTex = blankTex;
        bgTex = mBAMColumns[side]->Find<RndTex>("blank_bustamove.tex");
    } else {
        flashcardTex = DataDir()->Find<RndTex>("blank.tex");
        bgTex = flashcardTex;
    }

    Hmx::Color color(1.0f, 1.0f, 1.0f);
    if (i3 >= 0) {
        String bgName(MakeString("flashcard_slot_background%i.mat", i3));
        RndMat *slotBgMat = DataDir()->Find<RndMat>(bgName.c_str());
        color = slotBgMat->GetColor();
    } else if (i3 == -2) {
        UIColor *grayColor = DataDir()->Find<UIColor>("gray.color");
        color = grayColor->GetColor();
    }

    flashcardMat->SetDiffuseTex(flashcardTex);
    flashcardBgMat->SetColor(color.red, color.green, color.blue);
    flashcardBgMat->SetDiffuseTex(bgTex);

    // Handle the other side
    RndMat *otherFlashcardMat =
        mBAMColumns[side == 0]->Find<RndMat>(MakeString("flashcard%d.mat", index));
    RndMat *otherBgMat =
        mBAMColumns[side == 0]->Find<RndMat>(MakeString("flashcard_background%d.mat", index));

    if (mState == kBAMState_ShowMoveSequence || mState == kBAMState_ShowMoveSequenceSetup) {
        otherFlashcardMat->SetDiffuseTex(flashcardTex);
        otherBgMat->SetColor(color.red, color.green, color.blue);
        otherBgMat->SetDiffuseTex(bgTex);
    } else {
        otherFlashcardMat->SetDiffuseTex(blankTex);
        otherBgMat->SetDiffuseTex(blankTex);
    }
}

void BustAMovePanel::OnBeat() {
    if (!InBustAMove())
        return;
    if (TheGamePanel->IsGameOver())
        return;

    static Symbol beat("beat");
    static int sLastBeat = -1;
    int currentBeat = TheHamProvider->Property(beat, true)->Int();
    if (currentBeat == sLastBeat)
        return;
    sLastBeat = currentBeat;

    // beat 4 handling
    if (currentBeat == 4) {
        // Animate advance.anim for each column
        RndDir *col = mBAMColumns[1];
        for (int i = 0; i < 2; i++) {
            RndPropAnim *anim = col->Find<RndPropAnim>("advance.anim", true);
            anim->Animate(0.0f, false, 0.0f, nullptr, kEaseLinear, 0.0f, false);
            col = mBAMColumns[0];
        }

        static Symbol mulligan_count("mulligan_count");
        if (mState == kBAMState_Recording) {
            if (unk44 == 3) {
                static Message endMessage("bustamove_end_create");
                TheHamProvider->Handle(endMessage, false);
            }

            if (!unk970) {
                if (unk84 == 0) {
                    if (unk44 < 1) {
                        PlayVO(Symbol("nar_bam_take2_firsttime"));
                    } else if (unk44 == 1) {
                        PlayVO(Symbol("nar_bam_take3_firsttime"));
                    } else if (unk44 < 3) {
                        PlayVO(Symbol("nar_bam_take4_firsttime"));
                    }
                } else {
                    if (unk44 < 1) {
                        PlayVO(Symbol("nar_bam_take2"));
                    } else if (unk44 == 1) {
                        PlayVO(Symbol("nar_bam_take3"));
                    } else if (unk44 < 3) {
                        PlayVO(Symbol("nar_bam_take4"));
                    }
                }
            }

            if (unk970 && unk44 < 3) {
                int beatNum = (int)(TheTaskMgr.Beat() + 0.5f) + 1;
                static Message countInMsg(mulligan_count, 0);
                countInMsg[0] = beatNum;
                Handle(countInMsg, true);
            }
        }

        if (mState == kBAMState_RecordCountIn && unk970 && unk988 == 1) {
            int beatNum = (int)(TheTaskMgr.Beat() + 0.5f) + 1;
            static Message countInMsg(mulligan_count, 0);
            countInMsg[0] = beatNum;
            Handle(countInMsg, true);
        }
    }

    if (currentBeat != 1)
        return;

    unk40->ClearFrameScores();

    BAMState nextState = kBAMState_None;
    if (unk70 != kBAMState_None) {
        nextState = (BAMState)unk70;
        unk70 = kBAMState_None;
    } else if (mState <= kBAMState_ShowMoveSequence) {
        switch (mState) {
        case kBAMState_CountIn:
            if (unk44 == unk68 + 3)
                nextState = kBAMState_Recording;
            break;
        case kBAMState_Recording:
            if (unk44 >= 3) {
                static Symbol stay_on_bam_play("stay_on_bam_play");
                if (DataVariable(stay_on_bam_play).Int() == 0)
                    nextState = kBAMState_RecordCountIn;
            }
            break;
        case kBAMState_Playing:
            nextState = kBAMState_PlayCountIn;
            break;
        case kBAMState_ShowMove:
            if (unk988 == 1)
                nextState = kBAMState_Playing;
            break;
        case kBAMState_PlayCountIn:
            if (unk988 == 1) {
                if (unk44 == unk68 + 3)
                    nextState = kBAMState_Recording;
            }
            break;
        case kBAMState_RecordCountIn:
            if (unk44 == 1)
                nextState = kBAMState_RecordCountIn;
            break;
        case kBAMState_FailureToBust:
            if (unk988 == 1)
                nextState = kBAMState_ShowMoveSequence;
            break;
        case kBAMState_ShowMoveSequenceSetup:
            if (unk44 == 0xf)
                nextState = kBAMState_End;
            break;
        default:
            break;
        }
    }

    unk44++;
    if (unk988 > 0)
        unk988--;

    mStatusLabel->SetTextToken(gNullStr);
    mMovePromptLabel->SetTextToken(gNullStr);

    AdvanceFlashcards();

    if (nextState != kBAMState_None) {
        mState = nextState;
        unk44 = 0;
        unk988 = RepsToNextPhrase();
    }

    if (unk98c) {
        unk988 = RepsToNextPhrase();
        unk98c = false;
    }

    unk40->SetVal44(unk44);

    switch (mState) {
    case kBAMState_CountIn:
        break;
    case kBAMState_Recording:
        break;
    case kBAMState_Playing:
        break;
    case kBAMState_ShowMove:
        break;
    case kBAMState_PlayCountIn:
        break;
    case kBAMState_RecordCountIn:
        break;
    case kBAMState_FailureToBust:
        break;
    case kBAMState_ShowMoveSequenceSetup:
        break;
    case kBAMState_ShowMoveSequence:
        break;
    case kBAMState_End:
        break;
    default:
        break;
    }

    // End handling - check for recording trigger
    int loopTrigger = 0;
    if (mState == kBAMState_Recording && unk44 == 2 && currentBeat == 1) {
        loopTrigger = 1;
    }
    if (mState == kBAMState_Recording && unk44 == 2 && currentBeat == 2) {
        loopTrigger = 2;
    }
    if (mState == kBAMState_Recording && unk44 == 2 && currentBeat == 3) {
        loopTrigger = 3;
    }

    if (loopTrigger != 0) {
        unk930 = 0.0f;
        unk92c = loopTrigger;
    }
}
