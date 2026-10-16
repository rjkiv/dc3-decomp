#include "hamobj/RhythmBattlePlayer.h"
#include "char/Waypoint.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/RhythmBattle.h"
#include "hamobj/RhythmDetector.h"
#include "hamobj/ScoreUtl.h"
#include "math/Easing.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/PartLauncher.h"
#include "rndobj/Poll.h"
#include "ui/UIPanel.h"
#include "utl/Loader.h"
#include "utl/Option.h"
#include "utl/Symbol.h"
#include "world/Dir.h"

namespace {
    bool gDebugGroove;
    bool gDebugFresh;
}

static const float sThreshold = 0.6f;

RhythmBattlePlayer::RhythmBattlePlayer()
    : mComboPositionAnim(this), mComboColorAnim(this), mOneXComboEventAnim(this),
      mTwoXComboEventAnim(this), mThreeXComboEventAnim(this), mFourXComboEventAnim(this),
      mInTheZoneAnim(this), mLoopingMultAnim(this), mStaleAnim(this), mInAnim(this),
      mInAnim2(this), mOutAnim(this), mBoxyDir(this), mComboLabel(this),
      mScoreLabel(this), mInTheZoneFlow(this), mOutTheZoneOkFlow(this),
      mOutTheZoneBadFlow(this), mSwagJackedFlow(this), mPhraseMeter(this),
      mTransConstraint(this), mBoxyWaistTrans(this), mBoxyman1(this), mBoxyman2(this),
      mTextFeedback(this), mMoveFeedback(this), mStealPartSys(this), mStealAnim(this),
      mPlayer(0), mRhythmBattle(0), mGroove(0), mFreshness(0), mBeatGrooveAccum(0),
      mBeatFreshAccum(0), mAccumBeat(0), mLastPollBeat(0), mGrooveState(kNoGroove),
      mInTheZone(-2), mMeasureGroove(0), mMeasureFresh(0), mScore(0),
      mInTheZoneProgress(0), mGoofySwap(false), mDebugInt(-1), mToken("none"),
      mSwitchUpCooldown(0), mForceFail(false), unk2a5(false), unk2a8(0) {}

RhythmBattlePlayer::~RhythmBattlePlayer() {}

BEGIN_HANDLERS(RhythmBattlePlayer)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RhythmBattlePlayer)
    SYNC_PROP(score_label, mScoreLabel)
    SYNC_PROP(combo_position_anim, mComboPositionAnim)
    SYNC_PROP(combo_color_anim, mComboColorAnim)
    SYNC_PROP(reset_combo_anim, mOneXComboEventAnim)
    SYNC_PROP(2x_mult_anim, mTwoXComboEventAnim)
    SYNC_PROP(3x_mult_anim, mThreeXComboEventAnim)
    SYNC_PROP(4x_mult_anim, mFourXComboEventAnim)
    SYNC_PROP(player, mPlayer)
    SYNC_PROP(in_anim, mInAnim)
    SYNC_PROP(out_anim, mOutAnim)
    SYNC_PROP(boxydir, mBoxyDir)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RhythmBattlePlayer)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mComboPositionAnim;
    bs << mComboColorAnim;
    bs << mOneXComboEventAnim;
    bs << mTwoXComboEventAnim;
    bs << mThreeXComboEventAnim;
    bs << mFourXComboEventAnim;
    mComboLabel = nullptr;
    bs << mComboLabel;
    mScoreLabel = nullptr;
    bs << mScoreLabel;
    bs << mPlayer;
    bs << mBoxyDir;
END_SAVES

BEGIN_COPYS(RhythmBattlePlayer)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY_AS(RhythmBattlePlayer, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mComboPositionAnim)
        COPY_MEMBER(mComboColorAnim)
        COPY_MEMBER(mOneXComboEventAnim)
        COPY_MEMBER(mTwoXComboEventAnim)
        COPY_MEMBER(mThreeXComboEventAnim)
        COPY_MEMBER(mFourXComboEventAnim)
        COPY_MEMBER(mPlayer)
        COPY_MEMBER(mScoreLabel)
        COPY_MEMBER(mInAnim)
        COPY_MEMBER(mInAnim2)
        COPY_MEMBER(mOutAnim)
        COPY_MEMBER(mPhraseMeter)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(RhythmBattlePlayer)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mComboPositionAnim;
    d >> mComboColorAnim;
    d >> mOneXComboEventAnim;
    d >> mTwoXComboEventAnim;
    d >> mThreeXComboEventAnim;
    d >> mFourXComboEventAnim;
    d >> mComboLabel;
    mComboLabel = nullptr;
    d >> mScoreLabel;
    mScoreLabel = nullptr;
    d >> mPlayer;
    if (d.rev >= 1)
        d >> mBoxyDir;
END_LOADS

void RhythmBattlePlayer::Poll() {
    static UIPanel *sRhythmDetectorPanel =
        ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
    if (!TheLoadMgr.EditMode() && sRhythmDetectorPanel) {
        float f12 = TheTaskMgr.Beat();
        if (mRhythmBattle && mRhythmBattle->InFullKTB()) {
            HamPlayerData *hpd = TheGameData->Player(mPlayer);
            hpd->Provider()->Export(Message("hide_hud", 0), true);
        }
        if (mActive && mRhythmBattle && !mRhythmBattle->IsPaused()) {
            int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
                TheGameData->Player(mPlayer)->GetSkeletonTrackingID()
            );
            RhythmDetector *rd = nullptr;
            mGroove = 0;
            mFreshness = 0;
            if (skelIdx != -1 && sRhythmDetectorPanel->LoadedDir()) {
                String name = MakeString("RhythmDetectorX%d.rhy", skelIdx);
                rd = sRhythmDetectorPanel->LoadedDir()->Find<RhythmDetector>(
                    name.c_str(), false
                );
            }
            if (mAccumBeat >= 1) {
                if (rd) {
                    const RhythmDetector::RecordData &recordData = rd->GetRecord(
                        mMeasureStartBeat, mMeasureEndBeat, false, "", nullptr
                    );
                    mGroove = recordData.mRating;
                    if (Unk2a8Check() && 1 < recordData.unk14) {
                        mGroove = 1;
                    }
                    mFreshness = Unk2a8Check() ? 1 : rd->Freshness();
                }
                Symbol autoplay = TheGameData->Player(mPlayer)->Autoplay();
                if (!autoplay.Null()) {
                    static Symbol move_ok("move_ok");
                    static Symbol maximum("maximum");
                    if (autoplay == move_ok) {
                        mGroove = 1;
                        mFreshness = 0;
                    } else if (autoplay == maximum) {
                        mGroove = 1;
                        mFreshness = 1;
                    } else {
                        mGroove = RatingToDetectFrac(autoplay, nullptr);
                        mFreshness = 1;
                    }
                }
            }
            float f17 = f12 - mLastPollBeat;
            if (f17 < 0) {
                f17 = 0;
            }
            float f13 = mGroove <= 0 && mFreshness <= 0 ? 0.0f : 1.0f;

            mBeatGrooveAccum = mBeatGrooveAccum <= mGroove ? mGroove : mBeatGrooveAccum;
            if (mForceFail) {
                mBeatGrooveAccum = 0;
            }
            mBeatFreshAccum += mFreshness * f17;
            mBeatMoveAccum += f13 * f17;
            mAccumBeat += f17;
            float frac = mBeatGrooveAccum;
            if (frac > 1) {
                frac = 1;
            }
            float f16 = 4.0f - mAccumBeat;
            if (mPhraseMeter) {
                mPhraseMeter->SetRatingFrac(frac, Max(f16, 0.0f));
            }
            if (mInTheZone == 1 && mRhythmBattle && mRhythmBattle->InFullKTB()) {
                mInTheZoneProgress -= f17 * 1.125f;
                if (mInTheZoneProgress < 0) {
                    mInTheZoneProgress = 0;
                    int i7 = 0;
                    if (!mActive) {
                        i7 = -1;
                    }
                    if (i7 != mInTheZone) {
                        AnimateBoxyState(i7, true, false);
                    }
                }
                if (mComboPositionAnim) {
                    mComboPositionAnim->Animate(
                        mInTheZoneProgress,
                        mInTheZoneProgress,
                        mComboPositionAnim->Units()
                    );
                }
            }
            if (mBoxyman1) {
                mBoxyman1->SetGrooviness(rd, rd);
                mBoxyman2->SetGrooviness(rd, rd);
            }
        }
        mLastPollBeat = f12;
    }
}

void RhythmBattlePlayer::Enter() {
    RndPollable::Enter();
    mStaleAnim = Dir()->Find<RndAnimatable>(
        mPlayer != 0 ? "battle_meter_L_stale.anim" : "battle_meter_R_stale.anim", false
    );
    mInAnim = Dir()->Find<RndAnimatable>(
        mPlayer != 0 ? "battle_meter_l_in.anim" : "battle_meter_r_in.anim", false
    );
    mOutAnim = Dir()->Find<RndAnimatable>(
        mPlayer != 0 ? "battle_meter_l_out.anim" : "battle_meter_r_out.anim", false
    );
    mInAnim2 = Dir()->Dir()
        ? Dir()->Dir()->Find<RndAnimatable>(
              mPlayer != 0 ? "show_left_score.anim" : "show_right_score.anim", false
          )
        : nullptr;
    ObjectDir *rhythmBattleDir = Dir()->Find<ObjectDir>("rhythmbattle_audio", false);
    if (rhythmBattleDir) {
        mInTheZoneFlow = rhythmBattleDir->Find<Flow>(
            mPlayer != 0 ? "inthezone_p2.flow" : "inthezone_p1.flow", false
        );
        mOutTheZoneOkFlow = rhythmBattleDir->Find<Flow>(
            mPlayer != 0 ? "outthezone_ok_p2.flow" : "outthezone_ok_p1.flow", false
        );
        mOutTheZoneBadFlow = rhythmBattleDir->Find<Flow>(
            mPlayer != 0 ? "outthezone_bad_p2.flow" : "outthezone_bad_p1.flow", false
        );
        mSwagJackedFlow = rhythmBattleDir->Find<Flow>(
            mPlayer != 0 ? "swag_jacked_p2.flow" : "swag_jacked_p1.flow", false
        );
    }
    mActive = false;
    mLoopingMultAnim = nullptr;
    mTransConstraint = nullptr;
    if (!mBoxyDir && TheHamDirector) {
        WorldDir *wdir = TheHamDirector->GetVenueWorld();
        if (wdir) {
            mBoxyDir = wdir->Find<RndDir>("boxyman", false);
        }
    }
    if (mBoxyDir) {
        mTransConstraint = mBoxyDir->Find<TransConstraint>(
            mPlayer != 0 ? "TransConstraint1.tc" : "TransConstraint.tc", false
        );
        mPhraseMeter = mBoxyDir->Find<HamPhraseMeter>(
            mPlayer != 0 ? "phrase_meter1" : "phrase_meter0", false
        );
        mBoxyman1 = mBoxyDir->Find<DepthBuffer3D>(
            mPlayer != 0 ? "boxyman1_p2.db3d" : "boxyman1_p1.db3d", false
        );
        mBoxyman2 = mBoxyDir->Find<DepthBuffer3D>(
            mPlayer != 0 ? "boxyman2_p2.db3d" : "boxyman2_p1.db3d", false
        );
        mInTheZoneAnim = mBoxyDir->Find<RndAnimatable>(
            mPlayer != 0 ? "rhythmbattle_p2.anim" : "rhythmbattle_p1.anim", false
        );
        mBoxyWaistTrans = mBoxyDir->Find<RndTransformable>(
            mPlayer != 0 ? "boxywaist_p2.trans" : "boxywaist_p1.trans", false
        );
        mStealPartSys = mBoxyDir->Find<RndParticleSys>(
            mPlayer != 0 ? "steal_p2.part" : "steal_p1.part", false
        );
        mStealAnim = mBoxyDir->Find<RndAnimatable>(
            mPlayer != 0 ? "steal_p2.anim" : "steal_p1.anim", false
        );
        mTextFeedback = mBoxyDir->Find<ObjectDir>(
            mPlayer != 0 ? "text_feedback1" : "text_feedback0", false
        );
        mMoveFeedback = mBoxyDir->Find<ObjectDir>(
            mPlayer != 0 ? "move_feedback1" : "move_feedback0", false
        );
        if (mStealPartSys && !TheLoadMgr.EditMode()) {
            mStealPartSys->SetEmitRate(0, 0);
        }
        if (mBoxyman1) {
            DepthBuffer3DAttachment attachment;
            attachment.obj = mBoxyWaistTrans;
            attachment.player = mPlayer;
            attachment.bone = kJointHipCenter;
            attachment.offset = Vector3::GetZero();
            attachment.offset_from_ground = false;
            attachment.prev_parent = nullptr;
            mBoxyman1->AddAttachment(attachment);
        }
    }
    AnimateBoxyState(-1, false, false);
}

int RhythmBattlePlayer::SwagJacked(Hmx::Object *, RhythmBattleJackState) {
    MILO_ASSERT(mInTheZone == 1, 0x44a);
    static Symbol rhythmbattle_swagjackeddd1("rhythmbattle_swagjackeddd1");
    mToken = rhythmbattle_swagjackeddd1;
    mGrooveState = kNoGroove;
    if (mSwagJackedFlow)
        mSwagJackedFlow->Activate();
    return 1;
}

void RhythmBattlePlayer::HackPlayerQuit() {
    HamPlayerData *player = TheGameData->Player(mPlayer);
    int trackingID = player->GetSkeletonTrackingID();
    int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(trackingID);
    if (skelIdx != -1 && mInAnim2) {
        mInAnim2->Animate(mInAnim2->GetFrame(), mInAnim2->EndFrame(), mInAnim2->Units());
    }
}

int RhythmBattlePlayer::InTheZone() const { return mInTheZone == 1; }

float RhythmBattlePlayer::InAnimBeatLength() const { return 4.0f; }

void RhythmBattlePlayer::SetWindow(float f1, float f2) {
    mMeasureStartBeat = f1;
    mMeasureEndBeat = f2;
}

void RhythmBattlePlayer::SetInTheZone(int i, bool b1, bool b2) {
    if (!mActive) {
        i = -1;
    }
    if (mInTheZone != i) {
        AnimateBoxyState(i, b1, b2);
    }
}

void RhythmBattlePlayer::SetActive(bool b1) {
    mActive = b1;
    if (!mActive && mRhythmBattle && mRhythmBattle->InFullKTB() && mInTheZone != -1) {
        AnimateBoxyState(-1, true, false);
    }
}

void RhythmBattlePlayer::AnimateIn() { AnimateBoxyState(0, true, false); }

bool RhythmBattlePlayer::UpdateState() {
    mLastInTheZone = mInTheZone;
    mLastGrooveState = mGrooveState;
    if (mMeasureGroove < 0.5f) {
        mGrooveState = kNoGroove;
    } else if (mMeasureFresh >= sThreshold) {
        mGrooveState = kFreshMoves;
    } else if (mGrooveState <= 1) {
        mGrooveState = kSwitchUp;
    }
    return mLastGrooveState != mGrooveState;
}

void RhythmBattlePlayer::ResetCombo() {
    int i = 0;
    if (!mActive)
        i = -1;

    if (mInTheZone != i)
        AnimateBoxyState(i, false, false);

    mLastGrooveState = kNoGroove;
    unk2a8 = 0;
    mLastInTheZone = mInTheZone;
    mInTheZoneProgress = 0.0f;
}

void RhythmBattlePlayer::SwagJackedBonus(Hmx::Object *, RhythmBattleJackState, int i) {
    if (mStealAnim) {
        mStealAnim->Animate(
            mStealAnim->StartFrame(), mStealAnim->EndFrame(), mStealAnim->Units()
        );
    }
    static Symbol swag_jacked("swag_jacked");
    TheGameData->Player(mPlayer)->Provider()->Export(Message(swag_jacked), true);
    static Symbol rhythmbattle_swagjackeddd2("rhythmbattle_swagjackeddd2");
    mToken = rhythmbattle_swagjackeddd2;
    if (i != 0) {
        mGrooveState = kFreshMoves;
        mInTheZoneProgress = 16.0f;
    }
}

void SwapAnim(ObjPtr<RndAnimatable> &a1, ObjPtr<RndAnimatable> &a2) {
    RndAnimatable *tmp = a1;
    a1 = a2;
    a2 = tmp;
}

void RhythmBattlePlayer::SwapObjs(RhythmBattlePlayer *player) {
    SwapAnim(player->mComboPositionAnim, mComboPositionAnim);
    SwapAnim(player->mComboColorAnim, mComboColorAnim);
    SwapAnim(player->mOneXComboEventAnim, mOneXComboEventAnim);
    SwapAnim(player->mTwoXComboEventAnim, mTwoXComboEventAnim);
    SwapAnim(player->mThreeXComboEventAnim, mThreeXComboEventAnim);
    SwapAnim(player->mFourXComboEventAnim, mFourXComboEventAnim);
    SwapAnim(player->mStaleAnim, mStaleAnim);
    SwapAnim(player->mInAnim, mInAnim);
    SwapAnim(player->mInAnim2, mInAnim2);
    SwapAnim(player->mOutAnim, mOutAnim);

    HamLabel *tempLabel = player->mComboLabel;
    player->mComboLabel = mComboLabel;
    mComboLabel = tempLabel;

    tempLabel = player->mScoreLabel;
    player->mScoreLabel = mScoreLabel;
    mScoreLabel = tempLabel;

    SwapAnim(player->mLoopingMultAnim, mLoopingMultAnim);

    mGoofySwap = !mGoofySwap;
    player->mGoofySwap = !player->mGoofySwap;
}

void RhythmBattlePlayer::UpdateScore(int i1) {
    int inthezone = InTheZone() + 1;
    mScore += inthezone * i1;
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheHamProvider->Property(gameplay_mode)->Sym() == rhythm_battle) {
        static Symbol score("score");
        PropertyEventProvider *provider = TheGameData->Player(mPlayer)->Provider();
        provider->SetProperty(score, mScore);
    }
}

void RhythmBattlePlayer::OnReset(RhythmBattle *rb) {
    static Symbol none("none");
    mMeasureTrick = none;
    mRhythmBattle = rb;
    mSwitchUpCooldown = 0;
    mGrooveState = kNoGroove;
    mMeasureMove = 0;
    mLastGrooveState = kNoGroove;
    mBeatMoveAccum = 0;
    mScore = 0;
    mGroove = 0;
    mLastInTheZone = -1;
    mFreshness = 0;
    mInTheZone = -2;
    mInTheZoneProgress = 0;
    mBeatFreshAccum = 0;
    mBeatGrooveAccum = 0;
    mAccumBeat = 0;
    mMeasureGroove = 0;
    mMeasureFresh = 0;
    mLastPollBeat = 0;
    mLastFeets = -1.0f;
    if (mOneXComboEventAnim) {
        mOneXComboEventAnim->Animate(
            mOneXComboEventAnim->StartFrame(),
            mOneXComboEventAnim->EndFrame(),
            mOneXComboEventAnim->Units()
        );
    }
    if (mLoopingMultAnim) {
        mLoopingMultAnim->Animate(
            mLoopingMultAnim->StartFrame(),
            mLoopingMultAnim->StartFrame(),
            mLoopingMultAnim->Units()
        );
    }
    mLoopingMultAnim = nullptr;
    if (mOutAnim) {
        mOutAnim->Animate(mOutAnim->EndFrame(), mOutAnim->EndFrame(), mOutAnim->Units());
    }
    if (mComboColorAnim) {
        mComboColorAnim->SetFrame(0, 1);
    }
    if (mComboPositionAnim) {
        mComboPositionAnim->SetFrame(mInTheZoneProgress, 1);
    }
    if (mStaleAnim) {
        mStaleAnim->SetFrame(mStaleAnim->EndFrame(), 1);
    }
    unk2a8 = 0;
    UpdateScore(0);
    AnimateBoxyState(-1, false, false);
}

void RhythmBattlePlayer::UpdateAnimations(Hmx::Object *handler) {
    if (TheGestureMgr->GetSkeletonIndexByTrackingID(
            TheGameData->Player(mPlayer)->GetSkeletonTrackingID()
        )
        != -1) {
        MILO_ASSERT(handler, 0x378);
        static Symbol rhythmbattle_nogroove("rhythmbattle_nogroove");
        static Symbol rhythmbattle_groovelost("rhythmbattle_groovelost");
        static Symbol rhythmbattle_fresh("rhythmbattle_fresh");
        static Symbol rhythmbattle_switchup("rhythmbattle_switchup");
        static Symbol rhythmbattle_stale("rhythmbattle_stale");
        static Symbol rhythmbattle_unison("rhythmbattle_unison");
        static Symbol rhythmbattle_jacked("rhythmbattle_jacked");
        static Symbol rhythmbattle_jacked_bonus("rhythmbattle_jacked_bonus");
        static Symbol rhythmbattle_samegroove("rhythmbattle_samegroove");
        static Symbol rhythmbattle_inthezone("rhythmbattle_inthezone");
        static Symbol move_perfect("move_perfect");
        static Symbol move_awesome("move_awesome");
        static Symbol move_ok("move_ok");
        static Symbol move_bad("move_bad");
        static Symbol none("none");
        static Message groove_passed("groove_passed", 0, 0, 0, none, none);
        groove_passed[0] = mPlayer;
        int player = mGoofySwap ? !mPlayer : mPlayer;
        player += 3;
        groove_passed[player] = none;
        Symbol playerNodeValue;
        if (mMeasureTrick != none) {
            groove_passed[1] = move_perfect;
            playerNodeValue = mMeasureTrick;
        } else if (mMeasureGroove < 0.5f) {
            groove_passed[1] = move_bad;
            if (mLastGrooveState) {
                playerNodeValue = rhythmbattle_groovelost;
            } else {
                playerNodeValue = rhythmbattle_nogroove;
            }
        } else {
            groove_passed[1] = move_awesome;
            if (InTheZone() && mLastInTheZone != 1) {
                groove_passed[1] = move_perfect;
                playerNodeValue = rhythmbattle_inthezone;
            } else {
                if (mLastGrooveState) {
                    playerNodeValue = rhythmbattle_samegroove;
                } else {
                    playerNodeValue = rhythmbattle_fresh;
                }
            }
        }
        bool d13; // lol. lmao even
        if (groove_passed[1] != move_ok && groove_passed[1] != move_awesome) {
            mSwitchUpCooldown = 0;
            d13 = false;
        } else if (mSwitchUpCooldown > 0) {
            mSwitchUpCooldown--;
            d13 = false;
        } else {
            d13 = false;
        }
        if (gDebugGroove) {
            mDebugInt = mMeasureGroove * 100.0f;
        } else if (gDebugFresh) {
            mDebugInt = mMeasureFresh * 100.0f;
        }
        if (mDebugInt != -1) {
            MILO_LOG("measure score: %d\n", mDebugInt);
            mDebugInt = -1;
        } else if (mToken != none) {
            d13 = true;
            static Symbol rhythmbattle_swagjackeddd1("rhythmbattle_swagjackeddd1");
            static Symbol rhythmbattle_swagjackeddd2("rhythmbattle_swagjackeddd2");
            if (mToken == rhythmbattle_swagjackeddd1) {
                groove_passed[1] = move_bad;
                playerNodeValue = rhythmbattle_jacked;
            } else if (mToken == rhythmbattle_swagjackeddd2) {
                groove_passed[1] = move_perfect;
                playerNodeValue = rhythmbattle_jacked_bonus;
            } else {
                MILO_LOG("unknown token %s\n", playerNodeValue);
            }
            mToken = none;
        }
        groove_passed[player] = playerNodeValue;
        groove_passed[2] = d13;
        static UIPanel *sLoadingPanel =
            ObjectDir::Main()->Find<UIPanel>("loading_panel", false);
        if (sLoadingPanel && sLoadingPanel->LoadedDir() && sLoadingPanel->IsLoaded()) {
            mMoveFeedback->Find<Flow>("flow.flow")->Activate();
            mTextFeedback->Find<Flow>("flow.flow")->Activate();
            sLoadingPanel->HandleType(groove_passed);
        } else {
            handler->HandleType(groove_passed);
        }
        RndAnimatable *anim = nullptr;
        if (mInTheZone == 1U) {
            anim = mFourXComboEventAnim;
        }
        if (anim != mLoopingMultAnim) {
            if (anim) {
                static Symbol loop("loop");
                static Symbol dest("dest");
                anim->EndFrame();
                anim->EndFrame();
            }
            mLoopingMultAnim = anim;
        }
        if (mComboPositionAnim) {
            mComboPositionAnim->Animate(
                mInTheZoneProgress, mInTheZoneProgress, mComboPositionAnim->Units()
            );
        }
    }
}

void RhythmBattlePlayer::UpdateScore(Hmx::Object *handler) {
    unk2a8++;
    if (mStealPartSys) {
        mStealPartSys->SetEmitRate(0, 0);
    }
    int skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
        TheGameData->Player(mPlayer)->GetSkeletonTrackingID()
    );
    static UIPanel *sRhythmDetectorPanel =
        ObjectDir::Main()->Find<UIPanel>("rhythm_detector_panel", false);
    if (sRhythmDetectorPanel && skelIdx != -1 && sRhythmDetectorPanel->LoadedDir()) {
        String name = MakeString("RhythmDetectorX%d.rhy", skelIdx);
        RhythmDetector *rd =
            sRhythmDetectorPanel->LoadedDir()->Find<RhythmDetector>(name.c_str(), false);
        if (rd) {
            const RhythmDetector::RecordData &recordData =
                rd->GetRecord(mMeasureStartBeat, mMeasureEndBeat, true, "", nullptr);
            mGroove = recordData.mRating;
            if (Unk2a8Check() && 1 < recordData.unk14) {
                mGroove = 1;
            }
            mFreshness = Unk2a8Check() ? 1 : rd->Freshness();
            Symbol autoplay = TheGameData->Player(mPlayer)->Autoplay();
            if (!autoplay.Null()) {
                static Symbol move_ok("move_ok");
                static Symbol maximum("maximum");
                if (autoplay == move_ok) {
                    mGroove = 1;
                    mFreshness = 0;
                } else if (autoplay == maximum) {
                    mGroove = 1;
                    mFreshness = 1;
                } else {
                    mGroove = RatingToDetectFrac(autoplay, nullptr);
                    mFreshness = 1;
                }
            }
            mBeatGrooveAccum = mBeatGrooveAccum > mGroove ? mBeatGrooveAccum : mGroove;
        }
    }
    if (mForceFail) {
        mBeatGrooveAccum = 0;
    }
    static Symbol none("none");
    static Symbol pose("pose");
    static Symbol getlow("getlow");
    static Symbol jump("jump");
    static Symbol rhythmbattle_trickpose("rhythmbattle_trickpose");
    static Symbol rhythmbattle_trickgetlow("rhythmbattle_trickgetlow");
    static Symbol rhythmbattle_trickjump("rhythmbattle_trickjump");
    int i10 = 0;
    mMeasureTrick = none;
    mMeasureMove = mBeatMoveAccum / mAccumBeat;
    static Symbol autotrick(OptionStr("autotrick", "none"));
    if ((mMeasureMove < 0.5f || autotrick == pose) && mRhythmBattle->CanTrick(pose)) {
        mMeasureTrick = rhythmbattle_trickpose;
    }
    skelIdx = TheGestureMgr->GetSkeletonIndexByTrackingID(
        TheGameData->Player(mPlayer)->GetSkeletonTrackingID()
    );
    if (skelIdx >= 0) {
        Skeleton &skeleton = TheGestureMgr->GetSkeleton(skelIdx);
        float yLeft = skeleton.TrackedJoints()[kJointFootLeft].mPos[0].y;
        float yRight = skeleton.TrackedJoints()[kJointFootRight].mPos[0].y;
        float yMin = yLeft < yRight ? yLeft : yRight;
        float yMax = yLeft > yRight ? yLeft : yRight;
        if (mLastFeets != -1 && (yMin - mLastFeets > 0.1f)
            && mRhythmBattle->CanTrick(jump)) {
            mMeasureTrick = rhythmbattle_trickjump;
        }
        mLastFeets = yMax;
    }
    mMeasureGroove = mBeatGrooveAccum;
    if (mMeasureGroove > 1) {
        mMeasureGroove = 1;
    }
    mMeasureFresh = mBeatFreshAccum / mAccumBeat;
    if (mMeasureFresh > 1) {
        mMeasureFresh = 1;
    }
    mBeatMoveAccum = 0;
    mBeatFreshAccum = 0;
    mBeatGrooveAccum = 0;
    mAccumBeat = 0;
    if (mPhraseMeter) {
        mPhraseMeter->SetRatingFrac(0, -1);
    }
    MILO_ASSERT(handler != NULL, 0x305);
    if (mMeasureTrick == none) {
        static Message m("rating_to_score", 0, 0);
        m[0] = mMeasureGroove;
        m[1] = mMeasureFresh;
        DataNode handled = handler->HandleType(m);
        if (handled.Type() == kDataInt) {
            i10 = handled.Int();
        }
    }
    UpdateScore(i10);
}

void RhythmBattlePlayer::AnimateOut() {}

void RhythmBattlePlayer::UpdateComboProgress() {
    float f;
    if (!mRhythmBattle || !mRhythmBattle->InFullKTB()) {
        f = 8.0f;
    } else {
        f = 4.0f;
    }

    if (mGrooveState == kNoGroove) {
        mInTheZoneProgress = 0.0f;
        int i = 0;
        if (!mActive) {
            i = -1;
        }
        if (mInTheZone != i) {
            AnimateBoxyState(i, true, true);
        }
    } else if (mInTheZone < 1) {
        mInTheZoneProgress += f;
        if (mInTheZoneProgress >= 16.0f) {
            int i = 1;
            if (!mActive) {
                i = -1;
            }
            if (mInTheZone != i) {
                AnimateBoxyState(i, true, false);
            }
        }
    } else if (mInTheZone != 1) {
        return;
    } else {
        mInTheZoneProgress += f;
    }
}

void RhythmBattlePlayer::AnimateBoxyState(int i1, bool b2, bool b3) {
    b3 = mInTheZone != -1 ? b3 : false;
    if (mInTheZoneAnim) {
        float f30 = 0;
        static Symbol none("none");
        Symbol lmao = none;
        RndAnimatable::Rate rate = mInTheZoneAnim->GetRate();
        if (i1 > 0) {
            if (b2) {
                mInTheZoneAnim->Animate(0, false, 0, rate, 8, 12, 0, 1, lmao);
                f30 = 4;
            }
            static Symbol loop("loop");
            mInTheZoneAnim->Animate(0, false, f30, rate, 12, 20, 0, 1, Symbol(loop));
        } else if (i1 == 0) {
            if (b2) {
                if (mInTheZone == -1) {
                    mInTheZoneAnim->Animate(0, false, 0, rate, 36, 40, 0, 1, lmao);
                    f30 = 4;
                } else if (mInTheZone == 1) {
                    mInTheZoneAnim->Animate(0, false, 0, rate, 20, 24, 0, 1, lmao);
                    f30 = 4;
                }
            }
            static Symbol loop("loop");
            mInTheZoneAnim->Animate(0, false, f30, rate, 0, 8, 0, 1, Symbol(loop));
        } else if (i1 < 0) {
            if (b2) {
                if (mInTheZone == 0) {
                    mInTheZoneAnim->Animate(0, false, 0, rate, 24, 28, 0, 1, lmao);
                    f30 = 4;
                } else if (mInTheZone == 1) {
                    mInTheZoneAnim->Animate(0, false, 0, rate, 20, 28, 0, 2, lmao);
                    f30 = 4;
                }
            }
            static Symbol loop("loop");
            mInTheZoneAnim->Animate(0, false, f30, rate, 28, 36, 0, 1, Symbol(loop));
        }
    }

    mInTheZone = i1;
    static Symbol swag_jacked("swag_jacked");
    HamPlayerData *hpd = TheGameData->Player(mPlayer);
    static Symbol rhythmbattle_inthezone("rhythmbattle_inthezone");
    static Symbol rhythmbattle_outthezone("rhythmbattle_outthezone");
    if (i1 != -1) {
        if (i1 != 0) {
            if (mInTheZoneFlow) {
                mInTheZoneFlow->Activate();
            }
            hpd->Provider()->Export(Message(rhythmbattle_inthezone), true);
        } else {
            if (b3 && mOutTheZoneBadFlow && !mForceFail) {
                mOutTheZoneBadFlow->Activate();
            } else if (!b3 && mOutTheZoneOkFlow) {
                mOutTheZoneOkFlow->Activate();
            }
            hpd->Provider()->Export(Message(rhythmbattle_outthezone), true);
        }
    }
}
