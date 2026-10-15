#include "hamobj/RhythmBattle.h"
#include "RhythmBattlePlayer.h"
#include "char/CharDriver.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/ArchiveSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/Difficulty.h"
#include "hamobj/FreestyleMoveRecorder.h"
#include "hamobj/HamCamShot.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/RhythmDetector.h"
#include "hamobj/RhythmDetectorGroup.h"
#include "hamobj/SongUtl.h"
#include "macros.h"
#include "math/Easing.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Trans.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/Option.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"

namespace {
    bool gShortenSong;
}

void SetJump(int x, int y) {
    ClearJump();
    int x_adj = x - 1;
    int y_adj = y - 1;
    if (x_adj == y_adj) {
        x_adj *= 4;
        TheMaster->GetAudio()->SetLoop(y_adj * 4, x_adj);
    } else {
        x_adj *= 4;
        float f4 = BeatToMs(x_adj);
        float f5 = BeatToMs(y_adj * 4);
        float crossfade_beats = SystemConfig("synth", "crossfade_beats")->Float(1);
        crossfade_beats = BeatToMs(x_adj + crossfade_beats);
        TheMaster->GetAudio()->SetCrossfadeJump(f4, f5, crossfade_beats - f4);
    }
}

void ClearJump() {
    if (TheMaster && TheMaster->GetAudio()) {
        TheMaster->GetAudio()->ClearLoop();
    }
}

RhythmBattle::RhythmBattle()
    : mCommandLabel(this), mCommandLabel2(this), mPlayer0(this), mPlayer1(this),
      mBoxyLeadHead(this), mInAnim(this), mOutAnim(this), mTugAnim(this),
      mJackAnim1R(this), mJackAnim1L(this), mJackAnim2R(this), mJackAnim2L(this),
      mGoofySide(false), mFullOnKtB(true), mFinale(false), mActive(false),
      mOutroBegun(false), mPaused(false), mMindControlProgress(0),
      mLastMindControlVOTime(0), mMidwayBeat(0), mAlmostOverBeat(0), mBeatCount(0),
      mSynchronizedRating(0), mJackState(kNoJack), mWarnJackCounter(0), mRecorder(0),
      mBeatsToNextScriptedStage(0), mScriptedStageNum(0) {}

RhythmBattle::~RhythmBattle() { End(); }

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
    HANDLE_EXPR(
        both_players_dancing_bad,
        !(mPlayer0->InTheZone() == 0 && mPlayer1->InTheZone() == 0)
    )
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RhythmBattle)
    SYNC_PROP(command_label, mCommandLabel)
    SYNC_PROP(player0, mPlayer0)
    SYNC_PROP(player1, mPlayer1)
    SYNC_PROP(full_ktb, mFullOnKtB)
    SYNC_PROP(finale, mFinale)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RhythmBattle)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mCommandLabel;
    bs << mPlayer0;
    bs << mPlayer1;
    bs << mFullOnKtB;
END_SAVES

BEGIN_COPYS(RhythmBattle)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RhythmBattle, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mPlayer0)
        COPY_MEMBER(mPlayer1)
        COPY_MEMBER(mCommandLabel)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(RhythmBattle)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(RndPollable)
    if (d.rev >= 2) {
        d >> mCommandLabel;
        d >> mPlayer0;
        d >> mPlayer1;
    } else if (d.rev == 1) {
        ObjPtr<RhythmDetectorGroup> group1(this);
        ObjPtr<RhythmDetectorGroup> group2(this);
        ObjPtr<RndAnimatable> anim1(this);
        ObjPtr<RndAnimatable> anim2(this);
        ObjPtr<RndAnimatable> anim3(this);
        ObjPtr<HamLabel> label1(this);
        ObjPtr<HamLabel> label2(this);
        ObjPtr<HamLabel> label3(this);
        ObjPtr<HamLabel> label4(this);
        ObjPtr<RndMat> mat1(this);
        ObjPtr<RndMat> mat2(this);
        d >> group1;
        d >> group2;
        d >> anim1;
        d >> anim2;
        d >> anim3;
        d >> label1;
        d >> label2;
        d >> label3;
        d >> label4;
    }
    if (d.rev >= 3) {
        d >> mFullOnKtB;
    }
END_LOADS

void RhythmBattle::Poll() {
    if (!TheLoadMgr.EditMode()) {
        if (mActive && !mPaused) {
            bool goofy = GetGoofy();
            if (goofy != mGoofySide) {
                mPlayer0->SwapObjs(mPlayer1);
                mGoofySide = goofy;
            }
            Symbol leader = GetLeader();
            static Symbol left("left");
            static Symbol right("right");
            mLastLeader = leader;
            static UIPanel *sRhythmDetectorPanel =
                ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
            if (mRecorder != 0) {
                static Symbol playing("playing");
                if (TheUI->FocusPanel() && mIntroOver && !mOutroBegun) {
                    const Skeleton *skeleton1 = TheGameData->Player(0)->GetSkeleton();
                    if (skeleton1) {
                        mRecorder->SetVal44(skeleton1->SkeletonIndex());
                    } else {
                        mRecorder->SetVal44(-1);
                    }
                    mRecorder->Poll();
                    Skeleton *skeleton2 = TheGestureMgr->GetSkeletonByTrackingID(
                        TheGameData->Player(1)->GetSkeletonTrackingID()
                    );
                    if (skeleton2) {
                        mRecorderPair.push_back(ArchiveSkeleton());
                        ArchiveSkeleton &back = mRecorderPair.back();
                        if (mRecorderPair.size() > 200) {
                            MILO_WARN(
                                "bustajack recordings are getting big %d\n",
                                mRecorderPair.size()
                            );
                        }
                        back.Set(*skeleton2);
                    } else {
                        mRecorderPair.clear();
                    }
                }
            }
            int beat = TheTaskMgr.Beat();
            if (beat != mLastBeat && beat != 0) {
                OnBeat();
                mLastBeat = beat;
            }
            UpdateMindControl();
        }
        if (mFinale) {
            TheHamDirector->GetVenueWorld()
                ->Find<RndDir>("score_star_display")
                ->SetShowing(false);
        }
    }
}

void RhythmBattle::Enter() {
    RndPollable::Enter();
    if (mPlayer0)
        mPlayer0->OnReset(this);
    if (mPlayer1)
        mPlayer1->OnReset(this);
    mMindControlProgress = 0.0f;
    CheckIsFinale();
    if (mFullOnKtB)
        Begin();
}

void RhythmBattle::Exit() {
    static Symbol mind_control("mind_control");
    static Symbol gameplay_mode("gameplay_mode");
    Symbol mindControlCheck = TheHamProvider->Property(gameplay_mode)->Sym();
    if (mFullOnKtB || mindControlCheck == mind_control) {
        End();
    }
    RndPollable::Exit();
}

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
    Symbol ret = both;
    int p1 = mPlayer0 ? mPlayer0->GetScore() : 0;
    int p2 = mPlayer1 ? mPlayer1->GetScore() : 0;
    if (p1 != p2) {
        bool u4 = p2 < p1;
        if (goofy) {
            u4 = !u4;
        }
        ret = u4 ? right : left;
    }
    return ret;
}

void RhythmBattle::ResetCombo() {
    if (mPlayer0)
        mPlayer0->ResetCombo();
    if (mPlayer1)
        mPlayer1->ResetCombo();
}

void RhythmBattle::End() {
    if (mActive) {
        mActive = false;
        if (mPlayer0)
            mPlayer0->SetActive(false);
        if (mPlayer1)
            mPlayer1->SetActive(false);
        if (mRecorder) {
            mRecorder->Free();
            RELEASE(mRecorder);
        }
        if (!mPaused) {
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
        if (mFinale) {
            ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
            hudPanel->Find<RndDir>("score_right")->SetShowing(true);
            hudPanel->Find<RndDir>("score_left")->SetShowing(true);
        }
        mPaused = false;
    }
}

void RhythmBattle::OnReset() {
    if (mPaused) {
        OnUnpause();
    }
    CheckIsFinale();
    if (!mActive) {
        Begin();
    }
    mAnimIn = false;
    mIntroVO = false;
    mLastBeat = TheTaskMgr.Beat();
    mWarnJackCounter = 0;
    mJackState = kNoJack;
    mMidwayVO = false;
    mSynchronizedRating = 0.0f;
    mAlmostOverVO = false;
    mIntroOver = false;
    mOutroBegun = false;
    mBeatsToNextScriptedStage = 0;
    mScriptedStageNum = 0;
    mStealCooldown = 0;
    if (mPlayer0)
        mPlayer0->OnReset(this);
    if (mPlayer1)
        mPlayer1->OnReset(this);
    if (mOutAnim) {
        mOutAnim->Animate(mOutAnim->EndFrame(), mOutAnim->EndFrame(), mOutAnim->Units());
    }
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol mind_control("mind_control");

    Symbol gameplay_sym = TheHamProvider->Property(gameplay_mode)->Sym();
    if (TheHamDirector && mBeatCount == 0) {
        RndPropAnim *songAnim = TheHamDirector->SongAnimByDifficulty(kDifficultyEasy);
        if (songAnim) {
            mBeatCount = songAnim->GetNumKeys(TheHamDirector, "move");
        }
    }
    if (gameplay_sym == mind_control || mFinale) {
        float moveKeys = mBeatCount;
        if (moveKeys >= 1) {
            mStartBeat = 0;
            mEndBeat = (moveKeys - 2.0f) * 4.0f;
        }
    }
    if (gameplay_sym == mind_control) {
        TheMaster->GetAudio()->SetLoop(32, 128);
    }
    if (mFinale) {
        if (mCommandLabel) {
            mCommandLabel->SetTextToken(gNullStr);
        }
        if (mCommandLabel2) {
            mCommandLabel2->SetTextToken(gNullStr);
        }
        mFinaleVOQueue.clear();
        static Symbol finale_intro_01("finale_intro_01");
        static Symbol finale_intro_02("finale_intro_02");
        QueueFinaleVO(finale_intro_01);
        QueueFinaleVO(finale_intro_02);
        ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
        hudPanel->Find<RndDir>("score_right")->SetShowing(false);
        hudPanel->Find<RndDir>("score_left")->SetShowing(false);
        TheHamDirector->GetVenueWorld()
            ->Find<RndAnimatable>("set_bid.anim")
            ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    } else {
        mCommandLabel = Dir()->Find<HamLabel>("intro_line1.lbl", false);
        static Symbol rhythm_battle_title("rhythm_battle_title");
        if (mCommandLabel) {
            mCommandLabel->SetTextToken(rhythm_battle_title);
        }
        if (mCommandLabel2) {
            mCommandLabel2->SetTextToken(gNullStr);
        }
    }
}

void RhythmBattle::PlayMindControlVO(Symbol s) {
    static Message mind_control_vo("mind_control_vo", 0);
    mind_control_vo[0] = s;
    TheHamProvider->Handle(mind_control_vo, false);
    mLastMindControlVOTime = 0;
}

void RhythmBattle::UpdateFinaleVO(int &vo_ms) {
    if (!mFinaleVOQueue.empty() && vo_ms > 500) {
        UIPanel *game_panel = TheUI->FocusPanel();
        MILO_ASSERT(game_panel != NULL, 0x708);
        Symbol first = mFinaleVOQueue.front();
        mFinaleVOQueue.erase(mFinaleVOQueue.begin());
        static Message play_finale_vo("play_finale_vo", 0);
        play_finale_vo[0] = first;
        game_panel->HandleType(play_finale_vo);
        vo_ms = -1;
    }
}

void RhythmBattle::OnPause() {
    if (mActive) {
        mPaused = true;
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
    if (mPaused) {
        MILO_ASSERT(mActive, 0x7cc);
        mPaused = false;
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
    if (mFinale) {
        bool b = false;
        for (int i = 0; i < mFinaleVOQueue.size(); i++) {
            if (mFinaleVOQueue[i] == s)
                b = true;
        }

        if (!b)
            mFinaleVOQueue.push_back(s);
    }
}

void RhythmBattle::UpdateMindControl() {
    static Symbol mind_control("mind_control");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol game_stage("game_stage");
    static Symbol playing("playing");

    Symbol gameplaySym = TheHamProvider->Property(gameplay_mode)->Sym();
    Symbol stageSym = TheHamProvider->Property(game_stage)->Sym();

    if (gameplaySym != mind_control) {
        return;
    }
    if (mPlayer0) {
        mPlayer0->SetActive(true);
    }
    if (mPlayer1) {
        mPlayer1->SetActive(true);
    }
    HamCamShot *curShot = TheHamDirector->CurShot();
    if (stageSym != playing) {
        mLastMindControlVOTime = 0;
        mMindControlProgress = 0;
    } else if (curShot && strcmp(curShot->Category().Str(), "CAMP_MINDCONTROL") == 0) {
        mMindControlProgress = 0;
        mLastMindControlVOTime = 0;
        if (curShot->ShotOver()) {
            if (strcmp(curShot->Name(), "CAMP_6.3_DCI_mind_control_03.shot")) {
                TheHamDirector->ForceShot(gNullStr);
                static Symbol CAMP_MINDCONTROL_DANCE("CAMP_MINDCONTROL_DANCE");
                TheHamDirector->SetProperty("shot", CAMP_MINDCONTROL_DANCE);
            }
        }
    } else if (mPlayer0->GetGrooveState() || mPlayer1->GetGrooveState()) {
        mMindControlProgress += TheTaskMgr.DeltaBeat() / 10.0f;
    }
    for (int i = 0; i < 2; i++) {
        HamCharacter *hc = TheHamDirector->GetCharacter(i);
        RndAnimatable *mcAnim =
            hc ? hc->Find<RndAnimatable>("mind_control.anim", false) : nullptr;
        float f10 = sin(TheTaskMgr.UISeconds() * 18.84955596923828f);
        f10 = f10 * 0.4f + 0.6f;
        f10 *= mMindControlProgress;
        if (mcAnim) {
            mcAnim->SetFrame(f10, 1);
        }
        RndAnimatable *mcSoundAnim =
            hc ? hc->Find<RndAnimatable>("mind_control_sound.anim", false) : nullptr;
        if (mcSoundAnim) {
            mcSoundAnim->SetFrame(mMindControlProgress, 1);
        }
    }

    if (mMindControlProgress > 0.5f && mMindControlProgress < 0.95f
        && mLastMindControlVOTime > 5.0f) {
        static Symbol grooving("grooving");
        PlayMindControlVO(grooving);
    } else if (0.2f > mMindControlProgress && mLastMindControlVOTime > 12.0f) {
        static Symbol not_grooving("not_grooving");
        PlayMindControlVO(not_grooving);
    }
    mLastMindControlVOTime += TheTaskMgr.DeltaSeconds();
}

void RhythmBattle::CheckIsFinale() {
    DataArray *arr = new DataArray(1);
    arr->Node(0) = "is_finale";
    mFinale = arr->Execute(false).Int() && mFullOnKtB;
    arr->Release();
}

void RhythmBattle::PlayTanClip(int clipIndex, bool blend) {
    TheHamProvider->SetProperty("use_char_projection", true);
    static Message showCharProjection("show_char_projection");
    TheHamProvider->Handle(showCharProjection, false);
    CharDriver *driver = TheHamDirector->GetCharacter(0)->Driver();
    String str88;
    float fvar;
    if (clipIndex == 0) {
        str88 = "tan_rigged_01";
        fvar = 28;
        static Message tanPhaseIn("tan_finale_phasein01");
        TheHamProvider->Handle(tanPhaseIn, false);
    } else if (clipIndex == 1) {
        str88 = "tan_rigged_02";
        fvar = 22;
        static Message tanPhaseIn("tan_finale_phasein02");
        TheHamProvider->Handle(tanPhaseIn, false);
    } else if (clipIndex == 2) {
        str88 = "tan_rigged_03";
        fvar = 40;
        static Message tanPhaseIn("tan_finale_phasein03");
        TheHamProvider->Handle(tanPhaseIn, false);
    } else if (clipIndex == 3) {
        str88 = "tan_rigged_04";
        fvar = kHugeFloat;
    } else {
        str88 = "pose_fatalities_tan";
        fvar = 2.7f;
    }
    CharClip *clip = driver->FindClip(str88.c_str());
    driver->Play(clip, blend ? 1 : 2, -1, fvar, 0);
}

void RhythmBattle::Begin() {
    if (!mActive) {
        mActive = true;
        if (mFullOnKtB) {
            PropKeys *keys = TheHamDirector->GetPropKeys(kDifficultyExpert, "move");
            if (keys) {
                Keys<Symbol, Symbol> *symKeys = keys->AsSymbolKeys();
                int i = 0;
                Symbol Rest("Rest.move");
                Symbol rest("rest.move");
                float f26 = 0;
                Symbol startSym = Rest;
                for (; i < symKeys->size(); i++) {
                    if (startSym == Rest || startSym == rest) {
                        startSym = (*symKeys)[i].value;
                        f26 = (*symKeys)[i].frame;
                    } else {
                        break;
                    }
                }
                mStartBeat = FrameToBeat(f26);
                MILO_ASSERT(mStartBeat > 0, 0xE3);
                Symbol endSym = Rest;
                for (i = symKeys->size() - 1; i >= 0; i--) {
                    if (endSym == Rest || endSym == rest) {
                        endSym = (*symKeys)[i].value;
                        f26 = (*symKeys)[i].frame;
                    } else {
                        break;
                    }
                }
                mEndBeat = FrameToBeat(f26);
                MILO_ASSERT(mEndBeat > 0, 0xEF);
                float jumpStart = 0;
                float jumpEnd = 0;
                DataArray *arr = SystemConfig()->FindArray("party_jumps", false);
                if (arr) {
                    arr = arr->FindArray(TheGameData->GetSong(), false);
                    if (arr && gShortenSong) {
                        jumpStart = arr->Int(1) * 4.0f;
                        jumpEnd = (arr->Int(2) - 1) * 4.0f;
                    }
                }
                float diff = jumpEnd - jumpStart;
                float f28 = mEndBeat - mStartBeat - diff;
                mMidwayBeat = (f28 / 2.0f) + mStartBeat;
                mAlmostOverBeat = (f28 * 0.8f) + mStartBeat;
                if (mMidwayBeat > jumpStart) {
                    mMidwayBeat += diff;
                }
                if (mAlmostOverBeat > jumpStart) {
                    mAlmostOverBeat += diff;
                }
            }
            mPlayer0->SetInTheZone(-1, false, false);
            mPlayer1->SetInTheZone(-1, false, false);
        } else {
            mIntroVO = true;
            mAnimIn = true;
            mStartBeat = -1;
            mEndBeat = -1;
            mMidwayBeat = -1;
            mAlmostOverBeat = -1;
            mPlayer0->SetActive(true);
            mPlayer1->SetActive(true);
        }
        mCommandLabel2 = Dir()->Find<HamLabel>("intro_line2.lbl", false);
        if (TheHamDirector) {
            WorldDir *wdir = TheHamDirector->GetVenueWorld();
            if (wdir) {
                RndDir *boxy = wdir->Find<RndDir>("boxyman", false);
                if (boxy) {
                    mBoxyLeadHead =
                        boxy->Find<RndTransformable>("boxyleadhead.trans", false);
                }
            }
        }
        HamLabel *ml2x = Dir()->Find<HamLabel>("multiplier_L_2X.lbl", false);
        HamLabel *ml3x = Dir()->Find<HamLabel>("multiplier_L_3X.lbl", false);
        HamLabel *ml4x = Dir()->Find<HamLabel>("multiplier_L_4X.lbl", false);
        HamLabel *mr2x = Dir()->Find<HamLabel>("multiplier_R_2X.lbl", false);
        HamLabel *mr3x = Dir()->Find<HamLabel>("multiplier_R_3X.lbl", false);
        HamLabel *mr4x = Dir()->Find<HamLabel>("multiplier_R_4X.lbl", false);
        mJackAnim1L = Dir()->Find<RndAnimatable>("swag_jack_1bar_p1_to_p2.anim", false);
        mJackAnim1R = Dir()->Find<RndAnimatable>("swag_jack_1bar_p2_to_p1.anim", false);
        mJackAnim2L = Dir()->Find<RndAnimatable>("swag_jack_2bar_p1_to_p2.anim", false);
        mJackAnim2R = Dir()->Find<RndAnimatable>("swag_jack_2bar_p2_to_p1.anim", false);
        if (ml2x) {
            ml2x->SetTextToken("2x");
        }
        if (ml3x) {
            ml3x->SetTextToken("3x");
        }
        if (ml4x) {
            ml4x->SetTextToken("4x");
        }
        if (mr2x) {
            mr2x->SetTextToken("2x");
        }
        if (mr3x) {
            mr3x->SetTextToken("3x");
        }
        if (mr4x) {
            mr4x->SetTextToken("4x");
        }
        mInAnim = nullptr;
        mOutAnim = nullptr;
        mTugAnim = nullptr;
        if (mFullOnKtB && !mFinale) {
            mRecorder = new FreestyleMoveRecorder();
        }
        static UIPanel *sRhythmDetectorPanel =
            ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
        if (sRhythmDetectorPanel && sRhythmDetectorPanel->LoadedDir()) {
            for (int i = 0; i < 6; i++) {
                String s = MakeString("RhythmDetectorX%d.rhy", i);
                RhythmDetector *rh =
                    sRhythmDetectorPanel->LoadedDir()->Find<RhythmDetector>(s.c_str());
                rh->StartRecording();
            }
        }
    }
}

void RhythmBattle::OnBeat() {
    MILO_ASSERT(mActive, 0x290);
    static Symbol playing("playing");
    UIPanel *focusPanel = TheUI->FocusPanel();
    bool goofy = GetGoofy();
    if (!focusPanel || !mPlayer0 || !mPlayer1)
        return;
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol mind_control("mind_control");
    bool inMindControl = TheHamProvider->Property(gameplay_mode)->Sym() == mind_control;
    if (!mFullOnKtB || mFinale || inMindControl) {
        mPlayer0->SetUnk2a5(true);
        mPlayer1->SetUnk2a5(true);
    } else {
        mPlayer0->SetUnk2a5(false);
        mPlayer1->SetUnk2a5(false);
    }
    static Message remaining_vo_ms("remaining_vo_ms");
    DataNode handled = focusPanel->HandleType(remaining_vo_ms);
    int remainingValue = handled.Type() == kDataInt ? handled.Int() : 0;
    static Message play_vo("play_vo", 0, 0);
    static Message play_finale_vo("play_finale_vo", 0);
    static Symbol none("");
    play_finale_vo[0] = none;
    static Symbol both("both");
    static Symbol left("left");
    static Symbol right("right");
    play_vo[1] = both;
    float curBeat = TheTaskMgr.Beat();
    int beat = Round(curBeat);
    int i6cc = beat % 4;
    inMindControl = i6cc == 0;
    if (beat == 4) {
        static Message countInMsg("count_in", 0, 0);
        countInMsg[0] = mStartBeat - 4.0f;
        countInMsg[1] = mStartBeat - 4.0f;
        focusPanel->Handle(countInMsg, true);
    }
    UpdateFinaleVO(remainingValue);
    Symbol leader = GetLeader();
    if (remainingValue > 0 && !mIntroVO) {
        static Symbol intro("intro");
        play_vo[0] = intro;
        if (!mFinale) {
            focusPanel->HandleType(play_vo);
            remainingValue = -1;
        }
        mIntroVO = true;
    }
    if (remainingValue > 2000) {
        if (!mMidwayVO && mMidwayBeat < curBeat) {
            static Symbol halftime("halftime");
            play_vo[0] = halftime;
            play_vo[1] = leader;
            if (!mFinale) {
                focusPanel->HandleType(play_vo);
                remainingValue = -1;
            }
            mMidwayVO = true;
        }
        if (remainingValue > 2000 && !mAlmostOverVO && mAlmostOverBeat < curBeat) {
            static Symbol almost_over("almost_over");
            play_vo[0] = almost_over;
            play_vo[1] = leader;
            if (!mFinale) {
                focusPanel->HandleType(play_vo);
                remainingValue = -1;
            }
            mAlmostOverVO = true;
        }
    }
    bool b22 = false;
    bool bd4 = false;
    RndAnimatable *r28;
    if (mStartBeat > curBeat) {
        if (!mAnimIn) {
            if (mStartBeat <= mPlayer0->InAnimBeatLength() + curBeat) {
                mPlayer0->AnimateIn();
                mPlayer1->AnimateIn();
                if (mInAnim) {
                    mInAnim->Animate(
                        mInAnim->GetFrame(), mInAnim->EndFrame(), mInAnim->Units()
                    );
                }
                mAnimIn = true;
            } else {
                mPlayer0->SetActive(false);
                mPlayer1->SetActive(false);
            }
        }
        return;
    }
    if (!mIntroOver) {
        b22 = true;
        mIntroOver = true;
        inMindControl = false;
        if (mCommandLabel) {
            mCommandLabel->SetTextToken(gNullStr);
        }
        if (mCommandLabel2) {
            mCommandLabel2->SetTextToken(gNullStr);
        }
        mCommandLabel = nullptr;
        mCommandLabel2 = nullptr;
        mPlayer0->SetActive(true);
        mPlayer1->SetActive(true);
        mPlayer0->SetInTheZone(0, false, false);
        mPlayer1->SetInTheZone(0, false, false);
        static Message finish_intro("finished_intro");
        focusPanel->HandleType(finish_intro);
    }
    if (mFullOnKtB) {
        if (mEndBeat >= curBeat || mFinale) {
            if (!mFinale) {
                mPlayer0->SetActive(true);
                mPlayer1->SetActive(true);
            } else {
                if (mBeatsToNextScriptedStage == 0 || (int)curBeat > 0x10) {
                    mPlayer0->SetActive(true);
                } else {
                    mPlayer0->SetActive(false);
                }
                if (mBeatsToNextScriptedStage == 0 || (int)curBeat > 0x10) {
                    mPlayer1->SetActive(true);
                } else {
                    mPlayer0->SetActive(false);
                }
            }
        } else {
            mOutroBegun = true;
            mPlayer0->SetActive(false);
            mPlayer1->SetActive(false);
            if (mAnimIn) {
                static Symbol winner("winner");
                play_vo[0] = winner;
                play_vo[1] = leader;
                if (!mFinale) {
                    focusPanel->HandleType(play_vo);
                    remainingValue = -1;
                }
                mPlayer0->AnimateOut();
                mPlayer1->AnimateOut();
                r28 = mOutAnim;
                if (r28) {
                    mOutAnim->Animate(
                        mOutAnim->StartFrame(), mOutAnim->EndFrame(), mOutAnim->Units()
                    );
                }
                mAnimIn = false;
            }
        }
    }
    mPlayer0->HackPlayerQuit();
    mPlayer1->HackPlayerQuit();
    bool b40 = false;
    if (inMindControl) {
        // there is a LOT that happens in here
        if (mFullOnKtB && !mFinale) {
            if (!b22) {
                mRecorder->StopRecording();
                if (mStealCooldown && (mPlayer0->InTheZone() || mPlayer1->InTheZone())) {
                    b22 = true;
                } else {
                    b22 = false;
                }
                float f48 = 0;
                if (b22 && mRecorder) { // more checks go on in this if statement
                    float f45 = 0.10f;
                    for (int i = 0; i < mRecorderPair.size(); i++) {
                        DancerSkeleton dancerSkeleton;
                        if (i > 0) {
                            f45 += (float)mRecorderPair[i].ElapsedMs() / 1000.0f;
                        }
                        for (int j = 0; j < kNumJoints; j++) {
                            Vector3 v;
                            mRecorderPair[i].JointPos(kCoordCamera, (SkeletonJoint)j, v);
                            dancerSkeleton.SetCamJointPos((SkeletonJoint)j, v);
                            dancerSkeleton.SetCamJointDisplacement((SkeletonJoint)j, v);
                        }
                        dancerSkeleton.SetDisplacementElapsedMs(
                            mRecorderPair[i].ElapsedMs()
                        );
                        dancerSkeleton.Set(mRecorderPair[i]);
                        f48 = mRecorder->GetScore(&dancerSkeleton, 1, f45, false);
                    }
                }
                static bool autojack = OptionBool("autojack", false);
                if (b22 && autojack) {
                    f48 = 1;
                }
                if (f48 > 0.70f) {
                    RhythmBattlePlayer *jacker = mPlayer0;
                    RhythmBattlePlayer *jackee = mPlayer1;
                    // some logic here checking the players' InTheZone's
                    MILO_ASSERT(jackee->InTheZone(), 0x398);
                    b40 = true;
                    mStealCooldown = 4;
                }
                mRecorderPair.clear();
                mRecorder->ClearRecording();
                mRecorder->ClearFrameScores();
                mStealCooldown--;
                if (mStealCooldown < 0) {
                    mStealCooldown = 0;
                }
            }
            if (!mOutroBegun) {
                mRecorder->StartRecording();
            }
        }
        mPlayer0->UpdateScore(focusPanel);
        mPlayer1->UpdateScore(focusPanel);
    }
    static Message bars_between_vo_suggestion("bars_between_vo_suggestion");
    handled = focusPanel->HandleType(bars_between_vo_suggestion);
    if (handled.Type() == kDataInt) {
        handled.Int();
    }
    bool p1Update = false;
    bool p2Update = false;
    if (inMindControl) {
        p1Update = mPlayer0->UpdateState();
        p2Update = mPlayer1->UpdateState();
    }
    int i16 = p1Update ? mPlayer0->GetGrooveState() : -1;
    int i19 = p2Update ? mPlayer1->GetGrooveState() : -1;
    int i6b4;
    if (i19 < i16) {
        if (p1Update) {
            i6b4 = mPlayer0->GetGrooveState();
        } else {
            i6b4 = -1;
        }
    } else {
        if (p2Update) {
            i6b4 = mPlayer1->GetGrooveState();
        } else {
            i6b4 = -1;
        }
    }
    int i28 = Max(mPlayer0->GetGrooveState(), mPlayer1->GetGrooveState());
    bool i35 = i6b4 == mPlayer0->GetGrooveState();
    bool i27 = i6b4 == mPlayer1->GetGrooveState();
    bool b6f0 = i35;
    if (goofy) {
        b6f0 = i27;
        int tmp = i35;
        i35 = i27;
        i27 = tmp;
    }
    if (b40) {
        mWarnJackCounter++;
        if (inMindControl) {
            mWarnJackCounter = 0;
            if (mJackState == kNoJack
                || (mJackState > 2 && mJackState < kJackPenaltyLeft)) {
                bool b36 = true;
                RhythmBattlePlayer *first = mPlayer0;
                RhythmBattlePlayer *second = mPlayer1;
                if (mPlayer0->InTheZone() && !mPlayer1->InTheZone()
                    || (mPlayer1->InTheZone()
                        && mPlayer1->GetScore() < mPlayer0->GetScore())) {
                    b36 = false;
                    first = mPlayer1;
                    second = mPlayer0;
                }
                if (goofy) {
                    b36 = !b36;
                }
                mJackState = b36 == 0 ? kJackPenaltyLeft : kJackPenaltyRight;
                first->SwagJackedBonus(
                    focusPanel,
                    (RhythmBattleJackState)mJackState,
                    second->SwagJacked(focusPanel, (RhythmBattleJackState)mJackState)
                );
                i6b4 = (int)mJackState;
                b6f0 = !b36;
                i27 = !b6f0;
                i35 = b6f0;
                mJackState = kNoJack;
            }
        }
    } else {
        mWarnJackCounter = 0;
        mJackState = kNoJack;
    }
    bool b43 = i6b4 == 1 || i6b4 == 2;
    if ((i28 != 0 && i28 != 1 && i28 != 2) || b43 || (mEndBeat < curBeat + 12.0f)) {
        remainingValue = -1;
    }
    play_vo[0] = none;
    if (i27 || i35 || (mPlayer0->ZoneValue() != 0 && mPlayer0->GetLastInTheZone() == 0)
        || (mPlayer1->ZoneValue() != 0 && mPlayer1->GetLastInTheZone() == 0)) {
        static Symbol rhythmbattle_off_beat_p1p2("rhythmbattle_off_beat_p1p2");
        static Symbol rhythmbattle_off_beat_p1("rhythmbattle_off_beat_p1");
        static Symbol rhythmbattle_off_beat_p2("rhythmbattle_off_beat_p2");
        static Symbol rhythmbattle_nice_moves_p1p2("rhythmbattle_nice_moves_p1p2");
        static Symbol rhythmbattle_nice_moves_p1("rhythmbattle_nice_moves_p1");
        static Symbol rhythmbattle_nice_moves_p2("rhythmbattle_nice_moves_p2");
        static Symbol rhythmbattle_lemme_see_something_different_p1p2(
            "rhythmbattle_lemme_see_something_different_p1p2"
        );
        static Symbol rhythmbattle_lemme_see_something_different_p1(
            "rhythmbattle_lemme_see_something_different_p1"
        );
        static Symbol rhythmbattle_lemme_see_something_different_p2(
            "rhythmbattle_lemme_see_something_different_p2"
        );
        static Symbol rhythmbattle_stale_moves_warning_p1p2(
            "rhythmbattle_stale_moves_warning_p1p2"
        );
        static Symbol rhythmbattle_stale_moves_warning_p1(
            "rhythmbattle_stale_moves_warning_p1"
        );
        static Symbol rhythmbattle_stale_moves_warning_p2(
            "rhythmbattle_stale_moves_warning_p2"
        );
        static Symbol rhythmbattle_stale_moves_penalty_p1p2(
            "rhythmbattle_stale_moves_penalty_p1p2"
        );
        static Symbol rhythmbattle_stale_moves_penalty_p1(
            "rhythmbattle_stale_moves_penalty_p1"
        );
        static Symbol rhythmbattle_stale_moves_penalty_p2(
            "rhythmbattle_stale_moves_penalty_p2"
        );
        static Symbol rhythmbattle_synchronized1("rhythmbattle_synchronized1");
        static Symbol rhythmbattle_swagjackeddd1("rhythmbattle_swagjackeddd1");
        static Symbol rhythmbattle_synchronized2("rhythmbattle_synchronized2");
        static Symbol rhythmbattle_swagjackeddd2("rhythmbattle_swagjackeddd2");
        static Symbol intro("intro");
        static Symbol winner("winner");
        static Symbol groove_expired("groove_expired");
        static Symbol swag_jacked("swag_jacked");
        static Symbol no_groove_yet("no_groove_yet");
        static Symbol change_groove("change_groove");
        static Symbol almost_over("almost_over");
        static Symbol halftime("halftime");
        static Symbol trick_performed("trick_performed");
        static Symbol new_groove("new_groove");
        static Symbol jack_swag("jack_swag");
        static Symbol max_multiplier("max_multiplier");
        static Symbol getting_idea("getting_idea");
        static Symbol new_groove_working("new_groove_working");
        static Symbol stole_congrats("stole_congrats");
        if (i27 && b6f0) {
            play_vo[1] = both;
        } else if (i27) {
            play_vo[1] = right;
        } else {
            play_vo[1] = left;
        }
        int i6d8 = 1000000;
        static Symbol finale_no_groove_yet("finale_no_groove_yet");
        switch (i6b4) {
        case 0: {
            play_vo[0] = no_groove_yet;
            play_finale_vo[0] = finale_no_groove_yet;
            i6d8 = 8000;
            if (i27 && b6f0) {
                i6d8 = 6000;
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_off_beat_p1p2);
                }
                static Message msg("rhythm_battle_no_groove_p1p2");
                TheHamProvider->Handle(msg, false);
            } else if (i27) {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_off_beat_p1);
                }
                static Message msg("rhythm_battle_no_groove_p1");
                TheHamProvider->Handle(msg, false);
            } else {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_off_beat_p2);
                }
                static Message msg("rhythm_battle_no_groove_p2");
                TheHamProvider->Handle(msg, false);
            }
            break;
        }
        case 1: {
            i6d8 = 6000;
            play_vo[0] = new_groove;
            if (i27 && b6f0) {
                i6d8 = 5000;
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_nice_moves_p1p2);
                }
                static Message msg("rhythm_battle_fresh_moves_p1p2");
                TheHamProvider->Handle(msg, false);
            } else if (i27) {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_nice_moves_p1);
                }
                static Message msg("rhythm_battle_fresh_moves_p1");
                TheHamProvider->Handle(msg, false);
            } else {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(rhythmbattle_nice_moves_p2);
                }
                static Message msg("rhythm_battle_fresh_moves_p2");
                TheHamProvider->Handle(msg, false);
            }
            break;
        }
        case 2: {
            if (i27 && b6f0) {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(
                        rhythmbattle_lemme_see_something_different_p1p2
                    );
                }
                static Message msg("rhythm_battle_switch_it_up_p1p2");
                TheHamProvider->Handle(msg, false);
            } else if (i27) {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(
                        rhythmbattle_lemme_see_something_different_p1
                    );
                }
                static Message msg("rhythm_battle_switch_it_up_p1");
                TheHamProvider->Handle(msg, false);
            } else {
                if (mCommandLabel) {
                    mCommandLabel->SetTextToken(
                        rhythmbattle_lemme_see_something_different_p2
                    );
                }
                static Message msg("rhythm_battle_switch_it_up_p2");
                TheHamProvider->Handle(msg, false);
            }
            break;
        }
        case 3: {
            if (mCommandLabel) {
                mCommandLabel->SetTextToken(rhythmbattle_synchronized1);
            }
            static Message msg("rhythm_battle_synchronized");
            TheHamProvider->Handle(msg, false);
            break;
        }
        case 4: {
            if (mCommandLabel) {
                mCommandLabel->SetTextToken(rhythmbattle_synchronized2);
            }
            static Message msg("rhythm_battle_synchronized");
            TheHamProvider->Handle(msg, false);
            break;
        }
        case 5: {
            play_vo[0] = intro;
            if (mCommandLabel) {
                mCommandLabel->SetTextToken(rhythmbattle_swagjackeddd1);
            }
            static Message msg("rhythm_battle_swagjackeddd");
            TheHamProvider->Handle(msg, false);
            break;
        }
        case 6: {
            play_vo[0] = intro;
            if (mCommandLabel) {
                mCommandLabel->SetTextToken(rhythmbattle_swagjackeddd2);
            }
            static Message msg("rhythm_battle_swagjackeddd");
            TheHamProvider->Handle(msg, false);
            break;
        }
        }
        if (play_vo[0].Sym() != intro && remainingValue > 0) {
            static Symbol inzone("inzone");
            static Symbol inzone_warning("inzone_warning");
            if ((mPlayer0->ZoneValue() && !mPlayer0->GetLastInTheZone())
                || (mPlayer1->ZoneValue() && !mPlayer1->GetLastInTheZone())) {
                bool b43 = false;
                bool b42 = false;
                if (mPlayer0->InTheZone() || mPlayer1->InTheZone()) {
                    if (!mPlayer0->InTheZone() || !mPlayer1->InTheZone()) {
                        play_vo[0] = inzone_warning;
                    } else {
                        play_vo[0] = inzone;
                    }
                    b42 = mPlayer0->InTheZone();
                    b43 = mPlayer1->InTheZone();
                }
                bool b5 = b42;
                if (goofy) {
                    b5 = b43;
                    b43 = b42;
                }
                if (b5 && b43) {
                    play_vo[1] = both;
                } else if (b5) {
                    play_vo[1] = right;
                } else {
                    play_vo[1] = left;
                }
            }
        }
        if (play_vo[0].Sym() != none && i6d8 < remainingValue && !mFinale) {
            focusPanel->HandleType(play_vo);
            remainingValue = -1;
        }
        if (play_finale_vo[0].Sym() != none && i6d8 < remainingValue && mFinale) {
            focusPanel->HandleType(play_finale_vo);
            remainingValue = -1;
        }
    }
    int zone1 = mPlayer0->InTheZone();
    int zone2 = mPlayer1->InTheZone();
    if (r28) {
        mPlayer0->UpdateComboProgress();
        mPlayer1->UpdateComboProgress();
        mPlayer0->UpdateAnimations(focusPanel);
        mPlayer1->UpdateAnimations(focusPanel);
    }

    if (play_vo[0].Sym() == none && remainingValue > 15000) {
        static Symbol jack_swag("jack_swag");
        static Symbol max_multiplier("max_multiplier");
        static Symbol new_groove_working("new_groove_working");
        static Symbol inzone("inzone");
        static Symbol inzone_warning("inzone_warning");
        if (zone1 != mPlayer0->InTheZone() || zone2 != mPlayer1->InTheZone()) {
            bool b42 = false;
            bool b43 = false;
            if (mPlayer0->InTheZone() || mPlayer1->InTheZone()) {
                if (!mPlayer0->InTheZone() || !mPlayer1->InTheZone()) {
                    play_vo[0] = inzone_warning;
                } else {
                    play_vo[0] = inzone;
                }
                b42 = mPlayer0->InTheZone();
                b43 = mPlayer1->InTheZone();
            }
            bool b5 = b42;
            if (goofy) {
                b5 = b43;
                b43 = b42;
            }
            if (b5 && b43) {
                play_vo[1] = both;
            } else if (b5) {
                play_vo[1] = right;
            } else {
                play_vo[1] = left;
            }
        }
        if (play_vo[0].Sym() != none && !mFinale) {
            focusPanel->HandleType(play_vo);
            remainingValue = -1;
        }
    }

    if (inMindControl && (mPlayer0->InTheZone() || mPlayer1->InTheZone())) {
        static bool s14bc = false;
    }
    if (inMindControl && !mOutroBegun) {
        mPlayer0->SetWindow(curBeat, curBeat + 4.0f);
        mPlayer1->SetWindow(curBeat, curBeat + 4.0f);
    }
    if (mOutroBegun) {
        End();
    }
    float min84 = Min(mPlayer0->GetInTheZoneProgress(), mPlayer1->GetInTheZoneProgress());
    static UIPanel *sLoadingPanel =
        ObjectDir::Main()->Find<UIPanel>("loading_panel", false);
    if (sLoadingPanel && sLoadingPanel->LoadedDir()) {
        static Message worked_it_progress("worked_it_progress", 0);
        worked_it_progress[0] = min84;
        sLoadingPanel->HandleType(worked_it_progress);
    }
    if (DataVariable("reset_finale").Int()) {
        DataVariable("reset_finale") = 0;
        mScriptedStageNum = 0;
        mBeatsToNextScriptedStage = 0;
    }
    if (mFinale) {
        TheHamDirector->GetVenueWorld()->Find<Flow>("show_timeywimey.flow")->Activate();
    }
    if (inMindControl) {
        TheHamDirector->SetPlayerSpotlightsEnabled(false);
    }

    if (inMindControl || mFinale) {
        if (mBeatsToNextScriptedStage >= 0) {
            mBeatsToNextScriptedStage--;
            if (mFinale) {
                if (mBeatsToNextScriptedStage == 0x16) {
                    TheHamDirector->GetVenueWorld()->Find<RndDir>("boxyman")->SetShowing(
                        false
                    );
                }
                if (mBeatsToNextScriptedStage == 0x11 && mScriptedStageNum < 3) {
                    PlayTanClip(mScriptedStageNum, false);
                }
                if (mBeatsToNextScriptedStage == 0x10) {
                    static Message charProjectionActive("char_projection_move_active");
                    TheHamProvider->Handle(charProjectionActive, false);
                }
                if (mBeatsToNextScriptedStage == 0xc && mScriptedStageNum < 2) {
                    TheHamDirector->SetPlayerSpotlightsEnabled(true);
                }
                if (mBeatsToNextScriptedStage == 8 && mScriptedStageNum == 2) {
                    TheHamDirector->SetPlayerSpotlightsEnabled(true);
                }
                if (mBeatsToNextScriptedStage == 4) {
                    static Symbol finale_tandance_01("finale_tandance_01");
                    static Symbol finale_tandance_02("finale_tandance_02");
                    static Symbol finale_tandance_03("finale_tandance_03");
                    switch (mScriptedStageNum) {
                    case 0:
                        QueueFinaleVO(finale_tandance_01);
                        break;
                    case 1:
                        QueueFinaleVO(finale_tandance_02);
                        break;
                    case 2:
                        QueueFinaleVO(finale_tandance_03);
                        break;
                    default:
                        break;
                    }
                }
                if (mBeatsToNextScriptedStage == 3 && mScriptedStageNum < 2) {
                    TheHamDirector->SetPlayerSpotlightsEnabled(false);
                }
                if (mScriptedStageNum == 3 && mBeatsToNextScriptedStage == 0x14) {
                    PlayTanClip(3, true);
                }
                if (mScriptedStageNum == 3 && mBeatsToNextScriptedStage == 0x14) {
                    TheHamDirector->SetPlayerSpotlightsEnabled(false);
                }
                if (mScriptedStageNum == 3 && mBeatsToNextScriptedStage == 4) {
                    PlayTanClip(4, true);
                    static Message destroyCharProjection("destroy_char_projection");
                    TheHamProvider->Handle(destroyCharProjection, false);
                }
            }
            if (mBeatsToNextScriptedStage) {
                UpdateFinaleVO(remainingValue);
                return;
            }
            mScriptedStageNum++;
            if (mScriptedStageNum == 1) {
                if (!mFinale) {
                    static Symbol finale_phaseout_01("finale_phaseout_01");
                    static Symbol finale_phaseout_01b("finale_phaseout_01b");
                    QueueFinaleVO(finale_phaseout_01);
                    QueueFinaleVO(finale_phaseout_01b);
                    static Message charProjectionInactive("char_projection_move_inactive");
                    TheHamProvider->Handle(charProjectionInactive, false);
                    static Message hideCharProjection("hide_char_projection_keepdrawing");
                    TheHamProvider->Handle(hideCharProjection, false);
                    static Message tanPhaseOut("tan_finale_phaseout01");
                    TheHamProvider->Handle(tanPhaseOut, false);
                    mPlayer0->SetForceFail(false);
                    mPlayer1->SetForceFail(false);
                }
            } else if (mScriptedStageNum == 2) {
                if (!mFinale) {
                    static Symbol finale_phaseout_02("finale_phaseout_02");
                    static Symbol finale_phaseout_02b("finale_phaseout_02b");
                    QueueFinaleVO(finale_phaseout_02);
                    QueueFinaleVO(finale_phaseout_02b);
                    static Message charProjectionInactive("char_projection_move_inactive");
                    TheHamProvider->Handle(charProjectionInactive, false);
                    static Message hideCharProjection("hide_char_projection_keepdrawing");
                    TheHamProvider->Handle(hideCharProjection, false);
                    static Message tanPhaseOut("tan_finale_phaseout02");
                    TheHamProvider->Handle(tanPhaseOut, false);
                    mPlayer0->SetForceFail(false);
                    mPlayer1->SetForceFail(false);
                }
                ResetCombo();
            } else if (mScriptedStageNum == 3) {
                if (mFinale) {
                    static Symbol finale_phaseout_03("finale_phaseout_03");
                    static Symbol finale_phaseout_03b("finale_phaseout_03b");
                    QueueFinaleVO(finale_phaseout_03);
                    QueueFinaleVO(finale_phaseout_03b);
                    static Symbol tan_destroyed("tan_destroyed");
                    TheUI->FocusPanel()->SetProperty(tan_destroyed, true);
                    mBeatsToNextScriptedStage = 0x1c;
                    UpdateFinaleVO(remainingValue);
                } else {
                    static Message game_outro_mind_control("game_outro_mind_control");
                    focusPanel->HandleType(game_outro_mind_control);
                    UpdateFinaleVO(remainingValue);
                }
                return;
            } else if (mScriptedStageNum == 4) {
                MILO_ASSERT(mFinale, 0x66C);
                static Message game_outro_finale("game_outro_finale");
                focusPanel->HandleType(game_outro_finale);
                UpdateFinaleVO(remainingValue);
                return;
            } else {
                UpdateFinaleVO(remainingValue);
                return;
            }
        } else {
            if (mFinale && r28) {
                TheHamDirector->GetVenueWorld()->Find<RndDir>("boxyman")->SetShowing(true);
            }
            if ((!mFinale || min84 != 16.0f)
                && (mFinale || mMindControlProgress < 1 || mLastMindControlVOTime <= 5)) {
                UpdateFinaleVO(remainingValue);
                return;
            }
            if (mFinale) {
                mPlayer0->SetForceFail(true);
                mPlayer1->SetForceFail(true);
                if (mScriptedStageNum == 0) {
                    static Symbol finale_setcomplete_01("finale_setcomplete_01");
                    static Symbol finale_phasein_01("finale_phasein_01");
                    QueueFinaleVO(finale_setcomplete_01);
                    QueueFinaleVO(finale_phasein_01);
                } else if (mScriptedStageNum == 1) {
                    static Symbol finale_setcomplete_02("finale_setcomplete_02");
                    static Symbol finale_phasein_02("finale_phasein_02");
                    QueueFinaleVO(finale_setcomplete_02);
                    QueueFinaleVO(finale_phasein_02);
                } else if (mScriptedStageNum == 2) {
                    static Symbol finale_setcomplete_03("finale_setcomplete_03");
                    static Symbol finale_phasein_03("finale_phasein_03");
                    QueueFinaleVO(finale_setcomplete_03);
                    QueueFinaleVO(finale_phasein_03);
                }
                TheHamDirector->ForceShot("tan_finale.shot");
            } else {
                mPlayer0->OnReset(this);
                mPlayer1->OnReset(this);
                static UIPanel *sRhythmDetectorPanel =
                    ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
                if (sRhythmDetectorPanel && sRhythmDetectorPanel->LoadedDir()) {
                    for (int i = 0; i < 6; i++) {
                        String name = MakeString("RhythmDetectorX%d.rhy", i);
                        sRhythmDetectorPanel->LoadedDir()
                            ->Find<RhythmDetector>(name.c_str())
                            ->ClearData();
                    }
                }
                static Symbol working("working");
                static Message mindControlCompleteMsg("mind_control_complete");
                switch (mScriptedStageNum) {
                case 0:
                    PlayMindControlVO(working);
                    TheHamDirector->ForceShot("CAMP_6.3_DCI_mind_control_01.shot");
                    break;
                case 1:
                    PlayMindControlVO(working);
                    TheHamDirector->ForceShot("CAMP_6.3_DCI_mind_control_02.shot");
                    break;
                case 2:
                    TheHamProvider->Handle(mindControlCompleteMsg, false);
                    TheHamDirector->ForceShot("CAMP_6.3_DCI_mind_control_03.shot");
                    break;
                default:
                    break;
                }
            }
            mBeatsToNextScriptedStage = mFinale;
        }
    }
    UpdateFinaleVO(remainingValue);
}

bool RhythmBattle::CanTrick(Symbol) { return false; }
