#include "BustAMovePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "game/GamePanel.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DepthBuffer3D.h"
#include "gesture/GestureMgr.h"
#include "hamobj/BustAMoveData.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/ScoreUtl.h"
#include "lazer/game/GameMode.h"
#include "lazer/meta_ham/HamPanel.h"
#include "math/Color.h"
#include "math/Easing.h"
#include "math/Rand.h"
#include "math/Utl.h"
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
#include "rndobj/Graph.h"
#include "rndobj/Mat.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Tex.h"
#include "rndobj/TexRenderer.h"
#include "ui/UIColor.h"
#include "ui/UIPanel.h"
#include "utl/DebugGraph.h"
#include "utl/KnownIssues.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"
#include "utl/TempoMap.h"
#include "utl/TimeConversion.h"
#include <float.h>

namespace {
    void GetShuffledInts(std::vector<int> &vec, int num) {
        vec.clear();
        for (int i = 0; i < num; i++) {
            vec.push_back(i);
        }
        for (int i = 0; i < num - 1; i++) {
            int random = RandomInt(i, num);
            std::swap(vec[i], vec[random]);
        }
    }
}

BustAMovePanel::BustAMovePanel()
    : unk40(0), unk44(0), unk58(-1), unk5c(0), mHUDPanel(0), unk64(0), unk68(4), unk6c(0),
      unk70(10), unk80(0), unka0(kSkeletonRight), unk92c(0), unk930(0), unk934(0),
      unk958(-1), unk95c(-1), unk968(-1), unk970(0), unk988(0), unk98c(0), unk99c(0),
      unk9a0(FLT_MAX), unk9b9(0), unk9bc(-1) {
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
        return TheGameData->Player(i1)->Side() == kSkeletonRight ? "pink" : "blue";
    }
    return i1 == 0 ? "pink" : "blue";
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
    HamLabel *label2 =
        mBAMColumns[side == 0]->Find<HamLabel>(MakeString("flashcard_%d.lbl", index));
    if (mState == kBAMState_ShowMoveSequence
        || mState == kBAMState_ShowMoveSequenceSetup) {
        label2->SetTextToken(s3);
    } else {
        label2->SetTextToken(gNullStr);
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
    int flashCardIdx = index;
    Symbol s = gNullStr;
    if (i3 >= 0) {
        Symbol moveName = GetMoveNameData(i3)->Sym(1);
        s = moveName;
    }
    HamLabel *label = mBAMColumns[side]->Find<HamLabel>(
        MakeString("flashcard_name_%d.lbl", flashCardIdx)
    );
    label->SetTextToken(s);
    HamLabel *label2 = mBAMColumns[side == 0]->Find<HamLabel>(
        MakeString("flashcard_name_%d.lbl", flashCardIdx)
    );
    if (mState == kBAMState_ShowMoveSequence
        || mState == kBAMState_ShowMoveSequenceSetup) {
        label2->SetTextToken(s);
    } else {
        label2->SetTextToken(gNullStr);
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
        feedbackDir->Find<RndAnimatable>("move_flawless_right.anim")->Animate(0, 0, 0);
        moveFinishedMsg[1] = move_perfect;
    }
    if (mr == kMoveRatingPerfect) {
        feedbackDir->Find<RndAnimatable>("move_nice_right.anim")->Animate(0, 0, 0);
        moveFinishedMsg[1] = move_awesome;
    }
    if (mr == kMoveRatingAwesome) {
        feedbackDir->Find<RndAnimatable>("move_okay_right.anim")->Animate(0, 0, 0);
        moveFinishedMsg[1] = move_ok;
    }
    if (mr == kMoveRatingOk) {
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
    PlayVO(GetMoveNameData(unk84)->Sym((int)unka0 + 2));
}

float BustAMovePanel::GetMovePromptVOLength() {
    float len = 0;
    Symbol sym = GetMoveNameData(unk84)->Sym((int)unka0 + 2);
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
        it->SetGrooviness(1.0f);
        it->SetUnk18C(nullptr);
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
    unk94c = 0;
    unk950 = 0;
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
        const Hmx::Color &color = DataDir()->Find<UIColor>("gray.color")->GetColor();
        mat->SetColor(color.red, color.green, color.blue);
    }
    unk40->SetUnk3C(MetaPerformer::Current()->GetSong());
    for (int i = 0; i < 2; i++) {
        unk9a4[i] = true;
    }
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

void BustAMovePanel::AnimateFlashcard(int i) {
    UIColor *pColor = DataDir()->Find<UIColor>(MakeString("color%d.color", i + 1));
    const Hmx::Color color = pColor->GetColor();

    RndMat *pFlashcardMat = DataDir()->Find<RndMat>("flashcard.mat");
    String flashcardStrMakeString(MakeString("flashcard%i.tex", i));
    RndTex *pFlashcardTex = DataDir()->Find<RndTex>(flashcardStrMakeString.c_str());
    pFlashcardMat->SetDiffuseTex(pFlashcardTex);

    RndMat *pFlashcardCapBgMat =
        DataDir()->Find<RndMat>("flashcard_capture_background.mat");
    pFlashcardCapBgMat->SetColor(color.red, color.green, color.blue);

    HamLabel *pFlashcardCapLabel = DataDir()->Find<HamLabel>("flashcard_capture.lbl");
    Symbol moveDataSym = GetMoveNameData(i)->Sym(1);
    pFlashcardCapLabel->SetTextToken(moveDataSym);

    String flashcardSlotStr(MakeString("flashcard_slot%i.mat", i));
    RndMat *pFlashcardSlot = DataDir()->Find<RndMat>(flashcardSlotStr.c_str());
    RndTex *pFlashcardSlotTex = DataDir()->Find<RndTex>(flashcardStrMakeString.c_str());
    pFlashcardSlot->SetDiffuseTex(pFlashcardSlotTex);

    String flashcardSlotBgStr = MakeString("flashcard_slot_background%i.mat", i);
    RndMat *pFlashcardSlotBg = DataDir()->Find<RndMat>(flashcardSlotBgStr.c_str());
    pFlashcardSlotBg->SetColor(color.red, color.green, color.blue);

    String flashcardSlotLblStr = MakeString("flashcard_slot%i.lbl", i);
    HamLabel *pFlashcardSlotLabel =
        DataDir()->Find<HamLabel>(flashcardSlotLblStr.c_str());
    Symbol moveData = GetMoveNameData(i)->Sym(1);
    pFlashcardSlotLabel->SetTextToken(moveData);

    DataDir()->Find<RndPropAnim>("capture_flashcard.anim")->Animate(0.0f, false, 0.0f);
}

void BustAMovePanel::AdvanceFlashcards() {
    for (int i = 0; i < 2; i++) {
        RndPropAnim *pAnim = mBAMColumns[i]->Find<RndPropAnim>("advance.anim");
        pAnim->StopAnimation();
        pAnim->SetFrame(0.0f, 1.0f);
    }
    SkeletonSide side = unka0;
    if (!unk48.empty()) {
        unk48.pop_front();
    }

    auto it = unk48.begin();
    for (int i = 0; i < 4; i++) {
        if (it != unk48.end()) {
            SetFlashcardText(side, i, *it);
            ++it;
        } else {
            SetFlashcardText(side, i, gNullStr);
        }
    }

    if (!unk50.empty()) {
        unk50.pop_front();
    }

    auto it2 = unk50.begin();
    for (int i = 0; i < 4; i++) {
        int x = -1;
        if (it2 != unk50.end()) {
            x = *it2;
            ++it2;
        }
        SetFlashcardImage(side, i, x);
        SetFlashcardName(side, i, x);
    }
}

int BustAMovePanel::RepsToNextPhrase() {
    int beat1 = TheTaskMgr.Beat() + 0.5f;
    int beat2 = beat1;
    if (unk98c) {
        TheMaster->GetAudio()->GetCurrLoopBeats(beat1, beat2);
    }
    int u5 = beat1;
    for (int i = 0; i < mSongStructure.size(); i++) {
        u5 += mSongStructure[i] * -4;
        if (u5 < 0) {
        }
    }
    return 0;
}

void BustAMovePanel::PlayIntroVO() {
    if (unk9b8) {
        unk9b8 = false;
        float f = 0.0f;
        static Symbol nar_bam_intro("nar_bam_intro");
        static Message voLengthMsg("get_seq_length", 0);
        voLengthMsg[0] = nar_bam_intro;
        DataNode handle = mHUDPanel->Handle(voLengthMsg, true);
        if (handle != DATA_UNHANDLED) {
            f = handle.Float();
        }
        TempoMap *pTempoMap = TheMaster->SongData()->GetTempoMap();
        float tempoBPM = pTempoMap->GetTempoBPM(0.0f);
        float seconds_per_beat = 60.0f / tempoBPM;
        float songStructureVal = mSongStructure[0] * 4;
        float v = (seconds_per_beat * songStructureVal);
        float intro_s = (f - v);
        TheGame->SetIntroRealTime(-intro_s);
        PlayVO(nar_bam_intro);
    }
}

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
        bgTex = mBAMColumns[side]->Find<RndTex>("blank_bustamove.tex");
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
    RndMat *otherBgMat = mBAMColumns[side == 0]->Find<RndMat>(
        MakeString("flashcard_background%d.mat", index)
    );

    if (mState == kBAMState_ShowMoveSequence
        || mState == kBAMState_ShowMoveSequenceSetup) {
        otherFlashcardMat->SetDiffuseTex(flashcardTex);
        otherBgMat->SetColor(color.red, color.green, color.blue);
        otherBgMat->SetDiffuseTex(bgTex);
    } else {
        otherFlashcardMat->SetDiffuseTex(blankTex);
        otherBgMat->SetDiffuseTex(blankTex);
    }
}

void BustAMovePanel::Poll() {
    if (!InBustAMove() || TheGamePanel->IsGameOver()) {
        return;
    }

    HamPanel::Poll();
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    pPlayer1Data->Provider()->Export(Message("hide_hud", 0), true);
    pPlayer2Data->Provider()->Export(Message("hide_hud", 0), true);
    unk40->Poll();
    int which = unk64;
    if ((mState == kBAMState_PlayCountIn || mState == kBAMState_Playing)
        || mState == kBAMState_ShowMove) {
        which = !unk64;
    }
    unka0 = TheGameData->Player(which)->Side();
    const Skeleton *skel = TheGameData->Player(which)->GetSkeleton();
    int id = skel ? skel->SkeletonIndex() : -1;
    unk40->SetVal44(id);
    if (mState == kBAMState_Recording || mState == kBAMState_CountIn) {
        unk58 = id;
    }
    if (mState == kBAMState_Recording && unk44 >= 3) {
        unk974 = unk40->GetScore(id, 0, unk80, true);
        unk978 = unk40->GetScore(id, 1, unk80, false);
        unk80 += TheTaskMgr.DeltaUISeconds();
    }
    if (mState == kBAMState_Playing) {
        unk5c = unk40->GetScore(id, 0, -1.0f, false);
        mPhraseMeters[unka0]->SetShowing(true);
        float f36 = unk5c;
        int i22 = 2;
        float f33 = 1;
        while (true) {
            if (i22 & 1) {
                f33 *= f36;
            }
            i22 = (i22 << 0x20) >> 0x21;
            if (i22 == 0)
                break;
            f36 *= f36;
        }
        f36 = MsToBeat(unk80 * 1000);
        mPhraseMeters[unka0]->SetRatingFrac(f33 * 1.40f, 4.0f - f36);
        id = unk58;
    } else if (mState == kBAMState_ShowMoveSequence) {
        for (int i = 0; i < 2; i++) {
            int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
                TheGameData->Player(i)->GetSkeletonTrackingID()
            );
            SkeletonSide side = TheGameData->Player(i)->Side();
            unk90 = unk40->GetScore(skelIdx, i, -1, false);
            mPhraseMeters[side]->SetShowing(true);
            float f36 = unk90;
            int i22 = 2;
            float f33 = 1;
            while (true) {
                if (i22 & 1) {
                    f33 *= f36;
                }
                i22 = (i22 << 0x20) >> 0x21;
                if (i22 == 0)
                    break;
                f36 *= f36;
            }
            f36 = MsToBeat(unk80 * 1000);
            mPhraseMeters[unka0]->SetRatingFrac(f33 * 1.40f, 4.0f - f36);
        }
        id = unk40->GetUnkB8();
    } else {
        mPhraseMeters[0]->SetRatingFrac(0, -1);
        mPhraseMeters[1]->SetRatingFrac(0, -1);
        mPhraseMeters[0]->SetShowing(false);
        mPhraseMeters[1]->SetShowing(false);
    }
    if (mState == 8) {
        RndTex *pink = mBAMVisualizerPanel->DataDir()->Find<RndTex>("gradient_pink.tex");
        RndTex *blue = mBAMVisualizerPanel->DataDir()->Find<RndTex>("gradient_blue.tex");
        bool b15 = TheGameData->Player(0)->Side() == 0 && GetPlayerColor(0) == "pink";
        for (ObjDirItr<DepthBuffer3D> it(mBAMVisualizerPanel->DataDir(), true);
             it != nullptr;
             ++it) {
            const char *left = strstr(it->Name(), "_left");
            if ((!left || !b15) && (left || b15)) {
                it->SetPlayerPalette(blue);
            } else {
                it->SetPlayerPalette(pink);
            }
        }
        unk9bc = -1;
    } else if (unk9bc != which) {
        bool pink = GetPlayerColor(0) == "pink";
        RndTex *tex;
        if (pink) {
            tex = mBAMVisualizerPanel->DataDir()->Find<RndTex>("gradient_pink.tex");
        } else {
            tex = mBAMVisualizerPanel->DataDir()->Find<RndTex>("gradient_blue.tex");
        }
        for (ObjDirItr<DepthBuffer3D> it(mBAMVisualizerPanel->DataDir(), true);
             it != nullptr;
             ++it) {
            it->SetPlayerPalette(tex);
        }
        unk9bc = which;
    }
    bool b21 = mState != 1 && mState != 9;
    for (ObjDirItr<DepthBuffer3D> it(mBAMVisualizerPanel->DataDir(), true); it != nullptr;
         ++it) {
        it->ForceDrawSkeletonIndex(id, b21);
    }
    PollCaptureFlashcard();
    int beat = Round(MsToBeat(TheMaster->StreamMs()));
    if (beat == unk968) {
        unk968 = -1;
        static Message hideTransitionMsg("bustamove_hide_transition");
        TheHamProvider->Handle(hideTransitionMsg, false);
    }
    if (unk9a0 <= TheTaskMgr.Seconds(TaskMgr::kRealTime)) {
        PlayMovePromptVO();
        unk9a0 = FLT_MAX;
    }
    if (!DataVariable("bam_debug").Int())
        return;
    static DebugGraph scoreGraph(
        0.1f, 0.1f, 0.8f, 0.2f, Hmx::Color(0, 0, 0), Hmx::Color(1, 1, 1), 0, 0, 1, ""
    );
    scoreGraph.AddData(unk5c, false);
    scoreGraph.Draw();
    String stateStr;
    switch (mState) {
    case 0:
        stateStr = "kBAMState_CountIn";
        break;
    case 1:
        stateStr = "kBAMState_Recording";
        break;
    case 2:
        stateStr = "kBAMState_Playing";
        break;
    case 3:
        stateStr = "kBAMState_ShowMove";
        break;
    case 4:
        stateStr = "kBAMState_PlayCountIn";
        break;
    case 5:
        stateStr = "kBAMState_RecordCountIn";
        break;
    case 6:
        stateStr = "kBAMState_FailureToBust";
        break;
    case 7:
        stateStr = "kBAMState_ShowMoveSequenceSetup";
        break;
    case 8:
        stateStr = "kBAMState_ShowMoveSequence";
        break;
    case 9:
        stateStr = "kBAMState_End";
        break;
    case 10:
        stateStr = "kBAMState_None";
        break;
    default:
        break;
    }
    RndGraph *frame = RndGraph::GetOneFrame();
    frame->AddScreenString(
        MakeString("State: %s  Reps left: %d", stateStr, unk988),
        Vector2(0.1f, 0.05f),
        Hmx::Color(1, 1, 1)
    );
    int i68 = TheTaskMgr.Beat() + 0.5f;
    int i;
    for (i = 0; i < mSongStructure.size(); i++) {
        i68 += mSongStructure[i];
        if (i68 < 0)
            break;
    }
    for (int j = 0; j < mSongStructure.size(); j++) {
        frame->AddScreenString(
            MakeString("%d", mSongStructure[i]),
            Vector2((float)i68 * 0.02f + 0.1f, 0.08f),
            i == j ? Hmx::Color(0, 1, 0) : Hmx::Color(1, 1, 1)
        );
    }
}

void BustAMovePanel::SetUpSongStructure(Symbol s) {
    mSongStructure.clear();
    BustAMoveData *pBAMData =
        TheGame->GetMoveDir()->Find<BustAMoveData>("BustAMoveData.bam", false);
    if (pBAMData) {
        for (int i = 0; i < pBAMData->PhraseSize(); i++) {
            for (int j = 0; j < pBAMData->PhraseAt(i)->count; j++) {
                int bars = pBAMData->PhraseAt(i)->bars;
                mSongStructure.push_back(bars);
            }
        }
    } else {
        for (int i = 0; i < 8; i++) {
            mSongStructure.push_back(4);
        }
        TheKnownIssues.Display("bustamove_wrong_song", 5.0f);
    }
    MILO_ASSERT(mSongStructure.size() >= 2, 0x62c);
    unk68 = mSongStructure[0];
    unk988 = unk68 + 4;
    float total = 0.0f;
    for (int i = 1; i < mSongStructure.size(); i++) {
        total += mSongStructure[i];
    }
    unk958 = unk68 * 4.0f;
    unk95c = total * 4.0f + unk958;
    TheMaster->GetAudio()->SetLoop(unk958, unk95c);
}
