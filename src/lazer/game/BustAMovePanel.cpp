#include "BustAMovePanel.h"
#include "flow/Flow.h"
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
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
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
    : mRecorder(0), mReps(0), mRecordedSkeletonIndex(-1), mMoveScore(0), mHudPanel(0),
      mRecordingPlayer(0), mStartOffset(4), mSuccesses(0), mQueueState(kBAMState_None),
      mRecordTime(0), mActiveSide(kSkeletonRight), mCaptureFlashcard(0),
      mCaptureFlashcardTimer(0), mRenderFlashcard(0), mSongLoopStart(-1),
      mSongLoopEnd(-1), mHideTransitionOnBeat(-1), mUsingMulligan(0), mRepsLeft(0),
      mStreamJumped(0), mShuffledMoveNameIndex(0), mPlayMovePromptAt(FLT_MAX), unk9b9(0),
      unk9bc(-1) {
    mRecorder = new FreestyleMoveRecorder();
    mRecorder->AssignStaticInstance();
}

BustAMovePanel::~BustAMovePanel() { delete mRecorder; }

BEGIN_HANDLERS(BustAMovePanel)
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_ACTION(cache_objects, CacheObjects())
    HANDLE_ACTION(set_up_song_structure, SetUpSongStructure(_msg->Sym(2)))
    HANDLE_ACTION(on_stream_jump, mStreamJumped = true)
    HANDLE_ACTION(play_intro_vo, PlayIntroVO())
    HANDLE_SUPERCLASS(HamPanel)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BustAMovePanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void BustAMovePanel::Draw() {
    UIPanel::Draw();
    mRecorder->DrawDebug();
    if (mRenderFlashcard) {
        RndDir *renderer = DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
        String flashcard(MakeString("flashcard%i.tex", mNumCreatedMoves));
        RndTexRenderer *texRenderer =
            renderer->Find<RndTexRenderer>("TexRenderer.rndtex");
        int numPoses = 0;
        for (int i = 0; i < 3; i++) {
            if (mFlashcardPose[i].Tracked()) {
                numPoses++;
                MILO_ASSERT(numPoses <= 2, 0x1BC);
                String pose(MakeString("pose%i.tex", numPoses));
                RndDir *renderer =
                    DataDir()->Find<RndDir>("bustamove_flashcard_renderer");
                RndTex *tex = renderer->Find<RndTex>(pose.c_str());
                TheHamDirector->PoseIconMan(&mFlashcardPose[i], tex);
            }
        }
        if (numPoses == 0) {
            unk9b9 = true;
        }
        RndAnimatable *anim = renderer->Find<RndAnimatable>("num_poses.anim");
        anim->SetFrame(numPoses, 1);
        texRenderer->SetOutputTexture(DataDir()->Find<RndTex>(flashcard.c_str()));
        renderer->DrawShowing();
        mRenderFlashcard--;
    }
}

void BustAMovePanel::Enter() {
    HamPanel::Enter();
    mHudPanel = 0;
    if (InBustAMove()) {
        CacheObjects();
        mNeedToPlayIntroVO = true;
    }
}

void BustAMovePanel::Exit() {
    UIPanel::Exit();
    TheMaster->RemoveSink(this);
    if (mRecorder) {
        mRecorder->Free();
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
        mPlayerColumn[side]->Find<HamLabel>(MakeString("flashcard_%d.lbl", index));
    label->SetTextToken(s3);
    HamLabel *label2 =
        mPlayerColumn[side == 0]->Find<HamLabel>(MakeString("flashcard_%d.lbl", index));
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
    int nameIndex = mFlashcardName[index];
    MILO_ASSERT(nameIndex >= 0 && nameIndex < mShuffledMoveNames.size(), 0x658);
    DataArray *arr = TheGamePanel->Property(bustamove_move_names)->Array();
    return arr->Array(nameIndex);
}

void BustAMovePanel::SetMovePrompt() {
    Symbol sym = GetMoveNameData(mNumCreatedMoves)->Sym(0);
    mMovePrompt->SetTextToken(sym);
    UIColor *movePromptColor = DataDir()->Find<UIColor>("move_prompt.color");
    UIColor *playerColor = DataDir()->Find<UIColor>(
        MakeString("%s.color", GetPlayerColor(mRecordingPlayer))
    );
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
    HamLabel *label = mPlayerColumn[side]->Find<HamLabel>(
        MakeString("flashcard_name_%d.lbl", flashCardIdx)
    );
    label->SetTextToken(s);
    HamLabel *label2 = mPlayerColumn[side == 0]->Find<HamLabel>(
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
    resultMessage[0] = mRecordingPlayer == 0;
    resultMessage[1] = 0;
    resultMessage[2] = 0;
    TheHamProvider->Handle(resultMessage, false);
}

void BustAMovePanel::PlayMovePromptVO() {
    PlayVO(GetMoveNameData(mNumCreatedMoves)->Sym((int)mActiveSide + 2));
}

float BustAMovePanel::GetMovePromptVOLength() {
    float len = 0;
    Symbol sym = GetMoveNameData(mNumCreatedMoves)->Sym((int)mActiveSide + 2);
    static Message voLengthMsg("get_seq_length", 0);
    voLengthMsg[0] = sym;
    DataNode handled = mHudPanel->Handle(voLengthMsg, true);
    if (handled != DATA_UNHANDLED) {
        len = handled.Float();
    }
    return len;
}

void BustAMovePanel::ShowGetReadyCard(Symbol s, SkeletonSide side) {
    mPlayerColumn[side]->Find<HamLabel>("get_ready.lbl")->SetTextToken(s);
    static Message getReadyMsg("bustamove_get_ready", 0);
    getReadyMsg[0] = side;
    TheHamProvider->Handle(getReadyMsg, false);
}

void BustAMovePanel::CacheObjects() {
    mVisualizer = ObjectDir::Main()->Find<HamPanel>("bustamove_visualizer_panel");
    mVisualizer->DataDir()->Find<RndAnimatable>("num_players.anim")->SetFrame(1, 1);
    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true); it != nullptr; ++it) {
        it->SetGrooviness(1.0f);
        it->SetUnk18C(nullptr);
    }
    TheMaster->AddSink(this, "beat");
    mStatus = DataDir()->Find<HamLabel>("status.lbl");
    mMovePrompt = DataDir()->Find<HamLabel>("move_prompt.lbl");
    mStatus->SetTextToken(gNullStr);
    mMovePrompt->SetTextToken(gNullStr);
    if (SystemLanguage() == "jpn" || SystemLanguage() == "kor"
        || SystemLanguage() == "cht") {
        DataDir()
            ->Find<RndAnimatable>("asian_prompt_adjust.anim")
            ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    }
    mState = kBAMState_CountIn;
    mReps = 0;
    mSuccesses = 0;
    mNumCreatedMoves = 0;
    mRecordingPlayer = RandomInt(0, 2);
    mBustedMoveSuccessfully = false;
    mHudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
    for (int i = 0; i < 4; i++) {
        String flashcard = MakeString("flashcard_slot%i.mat", i);
        RndMat *flashcardMat = DataDir()->Find<RndMat>(flashcard.c_str());
        RndTex *blank = DataDir()->Find<RndTex>("blank.tex");
        flashcardMat->SetDiffuseTex(blank);
    }
    mPlayerColumn[kSkeletonRight] = DataDir()->Find<RndDir>("bustamove_column_right");
    mPlayerColumn[kSkeletonLeft] = DataDir()->Find<RndDir>("bustamove_column_left");
    mFlashcardText.clear();
    mFlashcardImage.clear();
    ResetScores();
    mAllowedBustFailures = 1;
    mBustFailures[0] = 0;
    mBustFailures[1] = 0;
    mPhraseMeter[kSkeletonRight] = DataDir()->Find<HamPhraseMeter>("phrase_meter_right");
    mPhraseMeter[kSkeletonLeft] = DataDir()->Find<HamPhraseMeter>("phrase_meter_left");
    mPlayMovePromptAt = FLT_MAX;
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
    mRecorder->SetSongName(MetaPerformer::Current()->GetSong());
    for (int i = 0; i < 2; i++) {
        mHasFlawlessedAllMoves[i] = true;
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
    mHudPanel->Handle(playVOMsg, true);
}

void BustAMovePanel::QueueMovePromptVO() {
    float voLength = GetMovePromptVOLength();
    TempoMap *tempoMap = TheMaster->SongData()->GetTempoMap();
    float bpm = tempoMap->GetTempoBPM(0);
    float secondsPerBeat = 60.0f / bpm;
    int reps = mRepsLeft;
    float beatsToWait = (float)((reps * 4) - 4);
    float timeOffset = beatsToWait * secondsPerBeat;
    float currentTime = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);
    mPlayMovePromptAt = currentTime + timeOffset - voLength - 1.0f;
}

void BustAMovePanel::PollCaptureFlashcard() {
    if (mCaptureFlashcard != 0) {
        float flashcardTweak = 0.17f;
        if (DataVarExists("flashcard_tweak")) {
            flashcardTweak = DataVariable("flashcard_tweak").Float();
        }
        if (mCaptureFlashcardTimer >= flashcardTweak) {
            BaseSkeleton *liveSkel = mRecorder->GetLiveSkeleton();
            if (liveSkel != nullptr) {
                mFlashcardPose[mCaptureFlashcard - 1].Set(*mRecorder->GetLiveSkeleton());
            } else {
                mFlashcardPose[mCaptureFlashcard - 1].SetTracked(false);
            }
            if (mCaptureFlashcard == 3) {
                float score1 = mRecorder->CompareSkeletonPositions(
                    &mFlashcardPose[0], &mFlashcardPose[1], 1.0f
                );
                float score2 = mRecorder->CompareSkeletonPositions(
                    &mFlashcardPose[0], &mFlashcardPose[2], 1.0f
                );
                if (score2 < 0.5f) {
                    mFlashcardPose[1].SetTracked(false);
                } else {
                    mFlashcardPose[2].SetTracked(false);
                }
                if (score1 >= 0.5f) {
                    mFlashcardPose[1].SetTracked(false);
                }
                mRenderFlashcard = 4;
            }
            mCaptureFlashcard = 0;
        } else {
            mCaptureFlashcardTimer += TheTaskMgr.DeltaUISeconds();
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
        RndPropAnim *pAnim = mPlayerColumn[i]->Find<RndPropAnim>("advance.anim");
        pAnim->StopAnimation();
        pAnim->SetFrame(0.0f, 1.0f);
    }
    SkeletonSide side = mActiveSide;
    if (!mFlashcardText.empty()) {
        mFlashcardText.pop_front();
    }

    auto it = mFlashcardText.begin();
    for (int i = 0; i < 4; i++) {
        if (it != mFlashcardText.end()) {
            SetFlashcardText(side, i, *it);
            ++it;
        } else {
            SetFlashcardText(side, i, gNullStr);
        }
    }

    if (!mFlashcardImage.empty()) {
        mFlashcardImage.pop_front();
    }

    auto it2 = mFlashcardImage.begin();
    for (int i = 0; i < 4; i++) {
        int x = -1;
        if (it2 != mFlashcardImage.end()) {
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
    if (mStreamJumped) {
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
    if (mNeedToPlayIntroVO) {
        mNeedToPlayIntroVO = false;
        float f = 0.0f;
        static Symbol nar_bam_intro("nar_bam_intro");
        static Message voLengthMsg("get_seq_length", 0);
        voLengthMsg[0] = nar_bam_intro;
        DataNode handle = mHudPanel->Handle(voLengthMsg, true);
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
        mPlayerColumn[side]->Find<RndMat>(MakeString("flashcard%d.mat", index));
    RndMat *flashcardBgMat = mPlayerColumn[side]->Find<RndMat>(
        MakeString("flashcard_background%d.mat", index)
    );
    RndTex *blankTex = DataDir()->Find<RndTex>("blank.tex");

    RndTex *flashcardTex;
    RndTex *bgTex;
    if (i3 >= 0) {
        flashcardTex = DataDir()->Find<RndTex>(MakeString("flashcard%i.tex", i3));
        bgTex = mPlayerColumn[side]->Find<RndTex>("blank_bustamove.tex");
    } else if (i3 == -2) {
        flashcardTex = blankTex;
        bgTex = mPlayerColumn[side]->Find<RndTex>("blank_bustamove.tex");
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
        mPlayerColumn[side == 0]->Find<RndMat>(MakeString("flashcard%d.mat", index));
    RndMat *otherBgMat = mPlayerColumn[side == 0]->Find<RndMat>(
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
    mRecorder->Poll();
    int which = mRecordingPlayer;
    if ((mState == kBAMState_PlayCountIn || mState == kBAMState_Playing)
        || mState == kBAMState_ShowMove) {
        which = !mRecordingPlayer;
    }
    mActiveSide = TheGameData->Player(which)->Side();
    const Skeleton *skel = TheGameData->Player(which)->GetSkeleton();
    int id = skel ? skel->SkeletonIndex() : -1;
    mRecorder->SetVal44(id);
    if (mState == kBAMState_Recording || mState == kBAMState_CountIn) {
        mRecordedSkeletonIndex = id;
    }
    if (mState == kBAMState_Recording && mReps >= 3) {
        mDuringBustMoveRatings[0] = mRecorder->GetScore(id, 0, mRecordTime, true);
        mDuringBustMoveRatings[1] = mRecorder->GetScore(id, 1, mRecordTime, false);
        mRecordTime += TheTaskMgr.DeltaUISeconds();
    }
    if (mState == kBAMState_Playing) {
        mMoveScore = mRecorder->GetScore(id, 0, -1.0f, false);
        mPhraseMeter[mActiveSide]->SetShowing(true);
        float f36 = mMoveScore;
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
        f36 = MsToBeat(mRecordTime * 1000);
        mPhraseMeter[mActiveSide]->SetRatingFrac(f33 * 1.40f, 4.0f - f36);
        id = mRecordedSkeletonIndex;
    } else if (mState == kBAMState_ShowMoveSequence) {
        for (int i = 0; i < 2; i++) {
            int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
                TheGameData->Player(i)->GetSkeletonTrackingID()
            );
            SkeletonSide side = TheGameData->Player(i)->Side();
            mSequenceMoveScore[0] = mRecorder->GetScore(skelIdx, i, -1, false);
            mPhraseMeter[side]->SetShowing(true);
            float f36 = mSequenceMoveScore[0];
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
            f36 = MsToBeat(mRecordTime * 1000);
            mPhraseMeter[mActiveSide]->SetRatingFrac(f33 * 1.40f, 4.0f - f36);
        }
        id = mRecorder->GetUnkB8();
    } else {
        mPhraseMeter[0]->SetRatingFrac(0, -1);
        mPhraseMeter[1]->SetRatingFrac(0, -1);
        mPhraseMeter[0]->SetShowing(false);
        mPhraseMeter[1]->SetShowing(false);
    }
    if (mState == 8) {
        RndTex *pink = mVisualizer->DataDir()->Find<RndTex>("gradient_pink.tex");
        RndTex *blue = mVisualizer->DataDir()->Find<RndTex>("gradient_blue.tex");
        bool b15 = TheGameData->Player(0)->Side() == 0 && GetPlayerColor(0) == "pink";
        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true); it != nullptr;
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
            tex = mVisualizer->DataDir()->Find<RndTex>("gradient_pink.tex");
        } else {
            tex = mVisualizer->DataDir()->Find<RndTex>("gradient_blue.tex");
        }
        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true); it != nullptr;
             ++it) {
            it->SetPlayerPalette(tex);
        }
        unk9bc = which;
    }
    bool b21 = mState != 1 && mState != 9;
    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true); it != nullptr; ++it) {
        it->ForceDrawSkeletonIndex(id, b21);
    }
    PollCaptureFlashcard();
    int beat = Round(MsToBeat(TheMaster->StreamMs()));
    if (beat == mHideTransitionOnBeat) {
        mHideTransitionOnBeat = -1;
        static Message hideTransitionMsg("bustamove_hide_transition");
        TheHamProvider->Handle(hideTransitionMsg, false);
    }
    if (mPlayMovePromptAt <= TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo)) {
        PlayMovePromptVO();
        mPlayMovePromptAt = FLT_MAX;
    }
    if (!DataVariable("bam_debug").Int())
        return;
    static DebugGraph scoreGraph(
        0.1f, 0.1f, 0.8f, 0.2f, Hmx::Color(0, 0, 0), Hmx::Color(1, 1, 1), 0, 0, 1, ""
    );
    scoreGraph.AddData(mMoveScore, false);
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
        MakeString("State: %s  Reps left: %d", stateStr, mRepsLeft),
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
    mStartOffset = mSongStructure[0];
    mRepsLeft = mStartOffset + 4;
    float total = 0.0f;
    for (int i = 1; i < mSongStructure.size(); i++) {
        total += mSongStructure[i];
    }
    mSongLoopStart = mStartOffset * 4.0f;
    mSongLoopEnd = total * 4.0f + mSongLoopStart;
    TheMaster->GetAudio()->SetLoop(mSongLoopStart, mSongLoopEnd);
}

void BustAMovePanel::OnBeat() {
    if (InBustAMove() && !TheGamePanel->IsGameOver()) {
        int beat = TheHamProvider->Property("beat")->Int();
        static int sBeat = -1;
        if (beat != sBeat) {
            sBeat = beat;
            if (beat == 4) {
                for (int i = 0; i < 2; i++) {
                    mPlayerColumn[i]
                        ->Find<RndPropAnim>("advance.anim")
                        ->Animate(0, false, 0);
                }
                if (mState == 1) {
                    if (mReps == 3) {
                        static Message endMessage("bustamove_end_create");
                        TheHamProvider->Handle(endMessage, false);
                    }
                    if (!mUsingMulligan) {
                        if (mNumCreatedMoves == 0) {
                            switch (mReps) {
                            case 0:
                                PlayVO("nar_bam_take2_firsttime");
                                break;
                            case 1:
                                PlayVO("nar_bam_take3_firsttime");
                                break;
                            case 2:
                                PlayVO("nar_bam_take4_firsttime");
                                break;
                            default:
                                break;
                            }
                        } else {
                            switch (mReps) {
                            case 0:
                                PlayVO("nar_bam_take2");
                                break;
                            case 1:
                                PlayVO("nar_bam_take3");
                                break;
                            case 2:
                                PlayVO("nar_bam_take4");
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    if (mUsingMulligan && mReps < 3) {
                        int taskMgrBeat = TheTaskMgr.Beat() + 0.5f;
                        taskMgrBeat += 1;
                        static Message countInMsg("mulligan_count", 0);
                        countInMsg[0] = taskMgrBeat;
                        Handle(countInMsg, true);
                    }
                }
                if (mState == 5 && mUsingMulligan && mRepsLeft == 1) {
                    int taskMgrBeat = TheTaskMgr.Beat() + 0.5f;
                    taskMgrBeat += 1;
                    static Message countInMsg("mulligan_count", 0);
                    countInMsg[0] = taskMgrBeat;
                    Handle(countInMsg, true);
                }
            } else if (beat == 1) {
                mRecorder->ClearFrameScores();
                int i13 = 10;

                // some switch happens here
                if (mQueueState != kBAMState_None) {
                    mQueueState = kBAMState_None;
                    i13 = mQueueState;
                } else {
                    // this entire switch for some reason, ghidra doesn't decompile
                    // so you gotta read asm for this
                    switch (mState) {
                    case 0:
                        if (mReps == 3) {
                            for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                                 it != nullptr;
                                 ++it) {
                                it->SetShowing(false);
                            }
                            mRecorder->StopPlayback();
                            MILO_LOG(
                                "1: %f(%d)   2: %f(%d)\n",
                                mDuringBustMoveRatings[0],
                                mRecorder->GetDancerTakeFrameCount(),
                                mDuringBustMoveRatings[1],
                                mRecorder->GetUnkB8()
                            );
                            // more
                            static Message createdMessage("bustamove_move_created");
                        }
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    default:
                        break;
                    }
                }
                // end switch

                mReps++;
                if (mRepsLeft > 0) {
                    mRepsLeft--;
                }
                mStatus->SetTextToken(gNullStr);
                mMovePrompt->SetTextToken(gNullStr);
                AdvanceFlashcards();
                if (i13 != 10) {
                    mState = (BAMState)i13;
                    mReps = 0;
                    mRepsLeft = RepsToNextPhrase();
                }
                if (mStreamJumped) {
                    mRepsLeft = RepsToNextPhrase();
                    mStreamJumped = false;
                }
                mRecorder->SetUnk40(mReps);
                switch (mState) {
                case 0:
                    if (mReps == 1) {
                        SetUpMoveNames();
                        for (int i = 0; i < mShuffledMoveNames.size(); i++) {
                            mFlashcardName[mNumCreatedMoves] =
                                mShuffledMoveNames[mShuffledMoveNameIndex];
                            mShuffledMoveNameIndex =
                                (mShuffledMoveNameIndex + 1) % mShuffledMoveNames.size();
                            if (GetMoveNameData(0)->Int(4) != 0) {
                                break;
                            }
                        }
                    }
                    if (mReps == mStartOffset - 2) {
                        DataDir()->Find<Flow>("intro.flow")->Activate();
                        QueueMovePromptVO();
                    }
                    if (mReps == mStartOffset - 1) {
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol("bam_record1"));
                        mFlashcardText.push_back(Symbol("bam_record2"));
                        mFlashcardText.push_back(Symbol("bam_record3"));
                        mFlashcardText.push_back(Symbol("bam_record4"));
                        CountIn(16);
                    }
                    if (mRepsLeft == 2 || mRepsLeft == 1) {
                        SetMovePrompt();
                    }
                    if (mRepsLeft == 2) {
                        ShowGetReadyCard("get_ready", mActiveSide);
                    }
                    break;

                case 1:
                    if (mReps == 0) {
                        unk9b9 = false;
                        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                             it != nullptr;
                             ++it) {
                            it->SetUnk18C(nullptr);
                            it->SetShowing(true);
                        }
                    } else {
                        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                             it != nullptr;
                             ++it) {
                            DepthBuffer3D *cur = it;
                            if (strstr(cur->Name(), "live")) {
                                cur->SetShowing(!DataVariable("hide_bam_ghost").Int());
                                cur->SetUnk18C(nullptr);
                            } else {
                                cur->SetUnk18C(mRecorder->GetOutputTex());
                            }
                        }
                    }
                    if (mReps == 0) {
                        mRecorder->StopPlayback();
                        mRecorder->SetFreestyleMove(mNumCreatedMoves);
                        static Message startMessage("bustamove_start_create", 0);
                        startMessage[0] = mActiveSide;
                        TheHamProvider->Handle(startMessage, false);
                        mRecorder->ClearRecording();
                        mRecorder->StartRecording();
                    }
                    if (mReps == 1) {
                        mRecorder->ClearDancerTake();
                        mRecorder->StartRecordingDancerTake();
                        mRecorder->StartPlayback(true);
                    }
                    if (mReps == 2) {
                        mRecorder->StartRecording();
                        mRecorder->StopPlayback();
                        mRecorder->StartPlayback(true);
                    }
                    if (mReps == 3) {
                        mRecorder->StopRecording();
                        mRecorder->StopPlayback();
                        mRecorder->StartPlayback(true);
                    }
                    mRecordTime = 0;
                    mMoveScore = 0;
                    break;

                case 2:
                    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                         it != nullptr;
                         ++it) {
                        it->SetShowing(true);
                    }
                    if (mReps == 0) {
                        mRecorder->StopRecording();
                        mSuccesses = 0;
                    } else {
                        mRecorder->StopPlayback();
                        MoveRating mr = GetMoveRating(mMoveScore);
                        ShowMoveRating(mr, mActiveSide);
                        if (mr == 0
                            || (mHasFlawlessedAllMoves[mRecordingPlayer == 0] = false,
                                mr == 1)) {
                            mSuccesses++;
                            IncreaseScore(mRecordingPlayer == 0, mr == 0 ? 50000 : 40000);
                            static Message matchedMessage("bustamove_move_matched", 0);
                            matchedMessage[0] = mSuccesses;
                            TheHamProvider->Handle(matchedMessage, false);
                        }
                    }
                    mMoveScore = 0;
                    mRecorder->StartPlayback(false);
                    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                         it != nullptr;
                         ++it) {
                        it->SetUnk18C(mRecorder->GetOutputTex());
                    }
                    break;

                case 3:
                    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                         it != nullptr;
                         ++it) {
                        it->SetShowing(false);
                    }
                    mRecorder->StopRecording();
                    mRecorder->StartPlayback(false);
                    MILO_ASSERT(mReps == 0, 0x328);
                    mFlashcardText.push_back(Symbol(gNullStr));
                    break;

                case 4:
                    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                         it != nullptr;
                         ++it) {
                        it->SetShowing(false);
                    }
                    mRecorder->StopPlayback();
                    if (mRepsLeft > 3) {
                        mFlashcardText.push_back(Symbol(gNullStr));
                    }
                    if (mRepsLeft == 3) {
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(mNumCreatedMoves - 1);
                        mFlashcardImage.push_back(mNumCreatedMoves - 1);
                        mFlashcardImage.push_back(mNumCreatedMoves - 1);
                        mFlashcardImage.push_back(mNumCreatedMoves - 1);
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        PlayVO(MakeString(
                            "nar_bam_%s_needstorepeat",
                            mActiveSide == 0 ? "left" : "right"
                        ));
                        CountIn(8);
                    }
                    if (mRepsLeft == 2) {
                        ShowGetReadyCard("get_ready_to_dance", mActiveSide);
                    }
                    break;

                case 5:
                    for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                         it != nullptr;
                         ++it) {
                        it->SetUnk18C(nullptr);
                        it->SetShowing(true);
                    }
                    if (mReps == 0) {
                        mFlashcardName[mNumCreatedMoves] =
                            mShuffledMoveNames[mShuffledMoveNameIndex];
                        mUsingMulligan = false;
                        mShuffledMoveNameIndex =
                            (mShuffledMoveNameIndex + 1) % mShuffledMoveNames.size();
                        if (!mBustedMoveSuccessfully) {
                            if (mBustFailures[mRecordingPlayer] < mAllowedBustFailures) {
                                mBustFailures[mRecordingPlayer]++;
                                mUsingMulligan = true;
                            }
                        } else {
                            mRecorder->PlaybackComplete();
                            MoveRating mr = GetMoveRating(mMoveScore);
                            ShowMoveRating(mr, mActiveSide);
                            if (mr == 0
                                || (mHasFlawlessedAllMoves[mRecordingPlayer == 0] = false,
                                    mr == 1)) {
                                mSuccesses++;
                                IncreaseScore(
                                    mRecordingPlayer == 0, mr == 0 ? 50000 : 40000
                                );
                                static Message matchedMessage("bustamove_move_matched", 0);
                                matchedMessage[0] = mSuccesses;
                                TheHamProvider->Handle(matchedMessage, false);
                            } else if (mSuccesses > 0) {
                                static Message successMessage(
                                    "bustamove_successfully_matched"
                                );
                                TheHamProvider->Handle(successMessage, false);
                                mStatus->SetTextToken("bam_matched");
                            } else if (mSuccesses == 0) {
                                SetRoundFailure();
                                mStatus->SetTextToken("bam_failed");
                                HamProfile *profile = TheProfileMgr.GetProfileFromPad(
                                    TheGameData->Player(mRecordingPlayer)->PadNum()
                                );
                                if (profile && profile->HasValidSaveData()) {
                                    static Symbol acc_inimitable("acc_inimitable");
                                    TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                                        profile, acc_inimitable, false
                                    );
                                }
                                static Message failMessage("bustamove_fail_match");
                                TheHamProvider->Handle(failMessage, false);
                            }
                        }
                        if (mBustedMoveSuccessfully || !mUsingMulligan) {
                            mRecordingPlayer = !mRecordingPlayer;
                        }
                        mMoveScore = 0;
                        if (mNumCreatedMoves == 4) {
                            mQueueState = kBAMState_ShowMoveSequenceSetup;
                            mFlashcardText.push_back(Symbol(gNullStr));
                        }
                        if (mSongLoopStart != -1) {
                            TheMaster->GetAudio()->SetLoop(mSongLoopStart, mSongLoopEnd);
                        }
                    }
                    if (mRepsLeft == 4 && mNumCreatedMoves != 4) {
                        QueueMovePromptVO();
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardImage.push_back(-2);
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol("bam_record1"));
                        mFlashcardText.push_back(Symbol("bam_record2"));
                        mFlashcardText.push_back(Symbol("bam_record3"));
                        mFlashcardText.push_back(Symbol("bam_record4"));
                    }
                    if (mRepsLeft == 3) {
                        CountIn(8);
                    }
                    if (mRepsLeft == 2 || mRepsLeft == 1) {
                        SetMovePrompt();
                    }
                    if (mRepsLeft == 2) {
                        ShowGetReadyCard("get_ready", mActiveSide);
                    }
                    break;
                case 6:
                    if (mReps == 0) {
                        static Message failMessage("bustamove_fail_bust");
                        TheHamProvider->Handle(failMessage, false);
                        mSuccesses = 0;
                        if (!mUsingMulligan) {
                            PlayVO("nar_bam_gen_fail");
                        } else {
                            PlayVO(MakeString(
                                "nar_bam_gen_second_fail_%s",
                                mActiveSide == 0 ? "left" : "right"
                            ));
                        }
                        int masterBeat = Round(MsToBeat(TheMaster->StreamMs()));
                        TheMaster->GetAudio()->SetLoop(
                            masterBeat, (float)masterBeat + 8.0f
                        );
                        mHideTransitionOnBeat =
                            Round(MsToBeat(TheMaster->StreamMs())) + 7;
                    }
                    break;
                case 9:
                    if (mReps == 0) {
                        static Symbol score("score");
                        int p0Score =
                            TheGameData->Player(0)->Provider()->Property(score)->Int();
                        int p1Score =
                            TheGameData->Player(1)->Provider()->Property(score)->Int();
                        int idx = -1;
                        if (p0Score > p1Score) {
                            idx = 0;
                        } else if (p1Score > p0Score) {
                            idx = 1;
                        }
                        static Message winnerMessage("bustamove_winner", 0);
                        if (idx >= 0) {
                            winnerMessage[0] = TheGameData->Player(idx)->Side();
                        } else {
                            winnerMessage[0] = -1;
                        }
                        TheHamProvider->Handle(winnerMessage, false);
                        ObjectDir *dataDir = mVisualizer->DataDir();
                        if (idx < 0) {
                            for (ObjDirItr<DepthBuffer3D> it(dataDir, true);
                                 it != nullptr;
                                 ++it) {
                                it->SetShowing(false);
                            }
                        } else {
                            for (ObjDirItr<DepthBuffer3D> it(dataDir, true);
                                 it != nullptr;
                                 ++it) {
                                it->SetUnk18C(nullptr);
                            }
                            DataDir()
                                ->Find<RndAnimatable>("num_players.anim")
                                ->SetFrame(1, 1);
                            mVisualizer->DataDir()
                                ->Find<RndAnimatable>("num_players.anim")
                                ->SetFrame(1, 1);
                            mRecordingPlayer = idx;
                        }
                        for (int i = 0; i < 2; i++) {
                            if (mHasFlawlessedAllMoves[i]) {
                                HamProfile *profile = TheProfileMgr.GetProfileFromPad(
                                    TheGameData->Player(i)->PadNum()
                                );
                                static Symbol acc_flawless_every_move(
                                    "acc_flawless_every_move"
                                );
                                TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                                    profile, acc_flawless_every_move, false
                                );
                            }
                        }
                        if (winnerMessage[0].Equal(0, nullptr, true)) {
                            PlayVO("nar_bam_win_left");
                        } else if (winnerMessage[0].Equal(1, nullptr, true)) {
                            PlayVO("nar_bam_win_right");
                        } else {
                            PlayVO("nar_bam_tie");
                        }
                    }
                    if (mReps == 3) {
                        TheGamePanel->SetGameOver(true);
                        TheMaster->GetAudio()->SetPaused(true);
                    }
                    break;
                case 7:
                    if (mReps == 0) {
                        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                             it != nullptr;
                             ++it) {
                            it->SetShowing(false);
                        }
                    }
                    if (mRepsLeft > 3) {
                        mFlashcardText.push_back(Symbol(gNullStr));
                    }
                    if (mRepsLeft == 3) {
                        static Message bothMessage("bustamove_both_dance");
                        TheHamProvider->Handle(bothMessage, false);
                        PlayVO("nar_bam_trans");
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardText.push_back(Symbol(gNullStr));
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFlashcardImage.push_back(-1);
                        mFinalSequenceType = DataVariable("bam_final_sequence").Int();
                        if (mFinalSequenceType == 0) {
                            mFinalSequenceType = 1;
                        }
                        switch (mFinalSequenceType) {
                        case 1: {
                            std::vector<int> ints;
                            GetShuffledInts(ints, 4);
                            int randInt = RandomInt(1, 4);
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back(ints[randInt]);
                            }
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back(ints[i]);
                                mFlashcardImage.push_back(ints[i]);
                            }
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back((i + 2) / 4);
                            }
                            break;
                        }
                        case 2: {
                            std::vector<int> vec1;
                            GetShuffledInts(vec1, 4);
                            std::vector<int> vec2;
                            GetShuffledInts(vec2, 4);
                            if (vec1.back() == vec2.front()) {
                                std::swap(vec2.front(), vec2.back());
                            }
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back(vec1[i]);
                                mFlashcardImage.push_back(vec1[i]);
                            }
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back(vec2[i]);
                                mFlashcardImage.push_back(vec2[i]);
                            }
                            break;
                        }
                        case 3: {
                            std::vector<int> vec1;
                            GetShuffledInts(vec1, 4);
                            std::vector<int> vec2;
                            GetShuffledInts(vec2, 4);
                            if (vec1.back() == vec2.front()) {
                                for (int i = 0; i < 4; i++) {
                                    mFlashcardImage.push_back(vec1[i]);
                                    mFlashcardImage.push_back(vec1[i]);
                                }
                            }
                            for (int i = 0; i < 2; i++) {
                                mFlashcardImage.push_back(vec2[i]);
                                mFlashcardImage.push_back(vec2[i]);
                            }
                            for (int i = 0; i < 4; i++) {
                                mFlashcardImage.push_back((i + 2) / 4);
                            }
                            break;
                        }
                        }
                        CountIn(8);
                    }
                    if (mRepsLeft == 2) {
                        ShowGetReadyCard("get_ready", kSkeletonLeft);
                        ShowGetReadyCard("get_ready", kSkeletonRight);
                    }
                    break;
                case 8:
                    if (mReps == 0) {
                        DataDir()->Find<RndAnimatable>("num_players.anim")->SetFrame(2, 1);
                        mVisualizer->DataDir()
                            ->Find<RndAnimatable>("num_players.anim")
                            ->SetFrame(2, 1);
                        DataDir()
                            ->Find<RndAnimatable>("finalsequence_crowdaudio.anim")
                            ->Animate(0, false, 0);
                    }
                    if (mReps < 16) {
                        for (ObjDirItr<DepthBuffer3D> it(mVisualizer->DataDir(), true);
                             it != nullptr;
                             ++it) {
                            it->SetUnk18C(mRecorder->GetOutputTex());
                            it->SetShowing(true);
                        }
                        mRecorder->SetFreestyleMove(mFlashcardImage.back());
                        mRecorder->StopPlayback();
                        mRecorder->StartPlayback(false);
                    }
                    if (mReps > 0) {
                        bool b19 = false;
                        for (int i = 0; i < 2; i++) {
                            MoveRating mr = GetMoveRating(mSequenceMoveScore[i]);
                            ShowMoveRating(mr, TheGameData->Player(i)->Side());
                            if (mr != 0) {
                                if (i != mMoveCreator[mFlashcardImage.back()]) {
                                    mHasFlawlessedAllMoves[i] = false;
                                }
                                if (mr == 1) {
                                    IncreaseScore(i, 40000);
                                    goto handle;
                                }
                            }
                            if (mr == 0) {
                                IncreaseScore(i, 50000);
                            handle:
                                if (!b19) {
                                    static Message matchedMessage(
                                        "bustamove_move_matched_finalsequence"
                                    );
                                    TheHamProvider->Handle(matchedMessage, false);
                                }
                                b19 = true;
                            }
                        }
                    }
                    if (mReps == 11
                        && (mFinalSequenceType == 1 || mFinalSequenceType == 3)) {
                        PlayVO("nar_bam_finale_fast");
                    }
                    mSequenceMoveScore[0] = 0;
                    mSequenceMoveScore[1] = 0;
                    break;
                default:
                    break;
                }
            }
            int i9 = 0;
            bool recording = mState == kBAMState_Recording;
            if (recording && mReps == 2 && beat == 1) {
                i9 = 1;
            }
            if (recording && mReps == 2 && beat == 2) {
                i9 = 2;
            }
            if (recording && mReps == 2 && beat == 3) {
                i9 = 3;
            }
            if (i9 != 0) {
                mCaptureFlashcardTimer = 0;
                mCaptureFlashcard = i9;
            }
        }
    }
}
