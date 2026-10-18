#include "hamobj/PoseFatalities.h"
#include "PoseFatalities.h"
#include "char/CharClip.h"
#include "char/CharClipDriver.h"
#include "char/CharDriver.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/Ham.h"
#include "hamobj/CharCameraInput.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/HamPlayerData.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/PropKeys.h"
#include "rndobj/Rnd.h"
#include "synth/FxSendDelay.h"
#include "synth/Synth.h"
#include "utl/BeatMap.h"
#include "utl/DebugMeter.h"
#include "utl/MakeString.h"
#include "utl/OSCMessenger.h"
#include "utl/Symbol.h"
#include "world/Dir.h"

PoseFatalities::PoseFatalities()
    : mHoldDuration(0.5f), mHudPanel(0), mJumpStart(0), mJumpEnd(0), mBeat(0),
      mVOCooldownTime(0), mVORequests(0), mTimeSinceVORequest(0) {
    for (int i = 0; i < 2; i++) {
        mFatalActive[i] = 0;
        mSatisfaction[i] = 0;
        mFatalOver[i] = 0;
        mGotFullCombo[i] = 0;
        mPoseStartBeat[i] = 0;
    }
    static DataNode &n = DataVariable("pose_fatalities");
    n = this;
}

BEGIN_HANDLERS(PoseFatalities)
    HANDLE_EXPR(get_fatality_beat_lead_in, mFatalityBeatLeadIn)
    HANDLE_EXPR(fatal_active, FatalActive())
    HANDLE_ACTION(activate_fatalities, ActivateFatal(-1))
    HANDLE_ACTION(set_jump, SetJump(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(fatal_end_beat, mFatalEndBeat)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PoseFatalities)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

bool PoseFatalities::FatalActive() const {
    for (int i = 0; i < 2; i++) {
        if (mFatalActive[i]) {
            return true;
        }
    }
    return false;
}

void PoseFatalities::SetJump(int x, int y) {
    mJumpEnd = y;
    mJumpStart = x;
}

bool PoseFatalities::GotFullCombo(int player) const {
    MILO_ASSERT_RANGE(player, 0, 2, 0x3A5);
    return mGotFullCombo[player];
}

Symbol PoseFatalities::GetFatalityFace() {
    Symbol smiles[3] = { "Smile", "Open_Smile_02", "Grin" };
    Symbol ret = smiles[RandomInt(0, 3)];
    if (RandomInt(0, 100) == 0) {
        ret = "O_Face";
    }
    if (RandomInt(0, 100) == 0) {
        ret = "Sexy";
    }
    return ret;
}

bool PoseFatalities::InFatality(int player) const {
    int max = mBeat;
    if (player == -1) {
        bool b1 = false;
        for (int i = 0; i < 2; i++) {
            if (max >= mFatalStartBeats[i]) {
                b1 = true;
            }
        }
        if (b1) {
            return FatalActive();
        } else {
            return false;
        }
    } else {
        MILO_ASSERT_RANGE(player, 0, 2, 0x391);
        if (max < mFatalStartBeats[player]) {
            return false;
        } else {
            return mFatalActive[player];
        }
    }
}

bool PoseFatalities::InStrikeAPose() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol strike_a_pose("strike_a_pose");
    return TheHamProvider->Property(gameplay_mode)->Sym() == strike_a_pose;
}

void PoseFatalities::SetCombo(int player, int combo) {
    mComboLevel[player] = combo;
    if (combo != 0) {
        mComboLabel[TheGameData->Player(player)->Side()]->SetInt(combo, false);
    }
}

void PoseFatalities::SetFatalitiesStart(int player) {
    mFatalActive[player] = true;
    if (InStrikeAPose()) {
        mFatalStartBeats[player] = mBeat + 3;
        for (int i = 0; i < 4; i++) {
            if (TheBeatMap->IsDownbeat(mFatalStartBeats[player])) {
                return;
            }
            mFatalStartBeats[player]++;
        }
    } else {
        mFatalStartBeats[player] = mBeat + 7;
    }
}

void PoseFatalities::Reset() {
    for (int i = 0; i < 2; i++) {
        mFatalStartBeats[i] = -1;
        mFatalActive[i] = false;
        mHoldScore[i] = 0;
    }
    TheHamProvider->SetProperty("in_fatalities", 0);
    RndAnimatable *anim = TheSynth->Find<RndAnimatable>("beat_repeat.anim");
    if (anim) {
        anim->SetFrame(4, 1);
    }
}

void PoseFatalities::PlayVO(Symbol s) {
    static Message stopVOMsg("stop_narrator");
    mHudPanel->Handle(stopVOMsg, true);
    static Message playVOMsg("play", 0);
    playVOMsg[0] = s;
    mHudPanel->Handle(playVOMsg, true);
    mVOCooldownTime = 5;
}

String PoseFatalities::GetCelebrationClip(int player) {
    Symbol outfit = GetOutfitCharacter(TheHamDirector->GetCharacter(player)->Outfit());
    static Symbol strikeapose_celebrations("strikeapose_celebrations");
    DataArray *cfg = SystemConfig(strikeapose_celebrations);
    DataArray *a = cfg->FindArray(outfit);
    return a->Node(RandomInt(1, a->Size())).Str();
}

void PoseFatalities::EndFatal(int player) {
    bool b10 = true;
    mFatalActive[player] = false;
    mFatalOver[player] = true;
    static Message endFatalityMsg("fatality_end", 0);
    endFatalityMsg[0] = player;
    TheHamProvider->Handle(endFatalityMsg, false);
    if (!InStrikeAPose()) {
        ObjectDir *poseDisplay = TheHamDirector->GetVenueWorld()->Find<ObjectDir>(
            MakeString("final_pose_display%d", player)
        );
        HamLabel *label = poseDisplay->Find<HamLabel>("pose_combo.lbl");
        int num;
        if (mGotFullCombo[player]) {
            num = 8;
        } else {
            num = mComboLevel[player] - 1;
        }
        if (mComboLevel[player] > 1) {
            label->SetTokenFmt("pose_fatality_3d_combo", num);
        } else {
            label->SetTextToken(gNullStr);
        }
        poseDisplay->Find<RndAnimatable>("pose_combo.anim")->Animate(0, false, 0);
    }
    mComboLevel[player] = 0;
    if (!DataVariable("restart_fatals").Int() && !InStrikeAPose()) {
        for (int i = 0; i < 2; i++) {
            b10 = mFatalActive[i] ? false : b10;
        }
        if (b10) {
            mEndPoseFatalitiesIn = 4;
        }
        TheHamDirector->GetCharacter(player)->BlendOutFaceOverrides(100);
    } else {
        mFatalStartBeats[player] = -1;
        ActivateFatal(player);
    }
}

void PoseFatalities::ActivateFatal(int player) {
    if (!DataVariable("disable_fatalities").Int() || InStrikeAPose()) {
        if (!InStrikeAPose()) {
            static Symbol peak_behavior("peak_behavior");
            static Symbol strike_a_pose("strike_a_pose");
            static Symbol game_stage("game_stage");
            static Symbol none("none");
            TheHamProvider->SetProperty(peak_behavior, strike_a_pose);
            TheHamProvider->SetProperty(game_stage, none);
        }
        if (!InStrikeAPose()) {
            TheHamDirector->ForceShot("area1_far01.shot");
        }
        mInFatalityMode = true;
        if (!FatalActive()) {
            static Message fatalityActivateMsg("fatality_active");
            TheHamProvider->Handle(fatalityActivateMsg, false);
        }
        if (player >= 0) {
            MILO_ASSERT(player < MAX_NUM_PLAYERS, 0x29E);
            SetFatalitiesStart(player);
        } else {
            for (int i = 0; i < 2; i++) {
                SetFatalitiesStart(i);
            }
        }
        if (!InStrikeAPose()) {
            MILO_ASSERT(player >= 0, 0x2A9);
            mFatalEndBeat = mFatalStartBeats[player] + 32;
        }
        TheHamProvider->SetProperty("in_fatalities", true);
        static Message resetDBOvertakeMsg("db_overtake", -1);
        TheHamProvider->Handle(resetDBOvertakeMsg, false);
    }
}

float PoseFatalities::BeatsLeftToMatch(int player) {
    float beat = TheTaskMgr.Beat();
    if (fabsf(beat - mBeat) > 2.0f) {
        beat = (mJumpEnd - mJumpStart) + beat;
    }
    int num = 4;
    if (mFatalityIndex[player] == 1 && !InStrikeAPose()) {
        num = 8;
    }
    return (mPoseStartBeat[player] + num) - beat;
}

void PoseFatalities::BeginFatal(int player) {
    if (mFatalActive[player]) {
        mFatalOver[player] = false;
        if (!InStrikeAPose()) {
            mFatalityIndex[player] = 0;
        }
        mComboLevel[player] = 0;
        mGotFullCombo[player] = false;
        AddFatal(player);
        static Message beginFatalityMsg("fatality_begin", 0);
        beginFatalityMsg[0] = player;
        TheHamProvider->Handle(beginFatalityMsg, false);
        TheHamDirector->GetCharacter(player)->BlendInFaceOverrides(100);
    }
}

bool PoseFatalities::CheckMatchingPose(int player) {
    if (InFatality(player) && mHoldScore[player] >= mHoldDuration) {
        return true;
    }
    return false;
}

void PoseFatalities::LoadFatalityClips() {
    if (InStrikeAPose()) {
        mAllFatalityClips.clear();
        for (ObjDirItr<CharClip> it(mHudPanel, true); it != nullptr; ++it) {
            if (strstr(it->Name(), "pose_fatalities_")) {
                mAllFatalityClips.push_back(it);
            }
        }
        MILO_ASSERT(mAllFatalityClips.size() > 0, 0x2D1);
    }
}

void PoseFatalities::Enter() {
    mHudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
    Reset();
    LoadFatalityClips();
    PropKeys *propKeys = TheHamDirector->GetPropKeys(kDifficultyEasy, "move");
    if (propKeys) {
        Keys<Symbol, Symbol> *symKeys = propKeys->AsSymbolKeys();
        static Symbol Rest("Rest.move");
        static Symbol rest("rest.move");
        int idx = 0;
        for (; idx < symKeys->size(); idx++) {
            Key<Symbol> cur = (*symKeys)[idx];
            if (cur.value != Rest && cur.value != rest)
                break;
        }
        mFatalityBeatLeadIn = Max(idx * 4 - 5, 0);
        for (idx = symKeys->size() - 1; idx >= 0; idx--) {
            Key<Symbol> cur = (*symKeys)[idx];
            if (cur.value != Rest && cur.value != rest)
                break;
        }
        if (InStrikeAPose()) {
            mFatalEndBeat = idx * 4;
        }
        mComboLabel[kSkeletonLeft] =
            mHudPanel->Find<ObjectDir>("hud_left")->Find<HamLabel>("pose_combo.lbl");
        mComboLabel[kSkeletonRight] =
            mHudPanel->Find<ObjectDir>("hud_right")->Find<HamLabel>("pose_combo.lbl");
        mPoseBeatAnims[kSkeletonLeft] =
            mHudPanel->Find<ObjectDir>("hud_left")->Find<RndAnimatable>("pose_beat.anim");
        mPoseBeatAnims[kSkeletonRight] =
            mHudPanel->Find<ObjectDir>("hud_right")->Find<RndAnimatable>("pose_beat.anim");
        mInFatalityMode = InStrikeAPose();
        FxSendDelay *delay = TheSynth->Find<FxSendDelay>("BeatRepeat.send");
        if (delay) {
            float bpm = TheMaster->SongData()->GetTempoMap()->GetTempoBPM(0);
            delay->SetProperty("tempo", bpm);
        }
        mBeat = 0;
        mEndPoseFatalitiesIn = -1;
    }
}

void PoseFatalities::PollVO() {
    if (InStrikeAPose()) {
        bool flag1 = mVORequests & 1;
        bool flag2 = (mVORequests >> 1) & 1;
        if (flag1 || flag2) {
            if (flag1 && flag2) {
                PlayVO("nar_sap_both_fc");
            } else if (flag1) {
                PlayVO("nar_sap_left_fc");
            } else if (flag2) {
                PlayVO("nar_sap_right_fc");
            }
        }
        if (mVOCooldownTime <= 0) {
            if (mVORequests & 4) {
                PlayVO("nar_sap_gen_pos");
            } else if (mTimeSinceVORequest > 8.0f) {
                PlayVO("nar_sap_time_limit");
                mTimeSinceVORequest = 0;
            }
        } else {
            mVOCooldownTime -= TheTaskMgr.DeltaSeconds();
        }
        mVORequests = 0;
        if (InFatality(-1)) {
            mTimeSinceVORequest += Clamp(0.0f, 1.0f, TheTaskMgr.DeltaSeconds());
        }
    }
}

void PoseFatalities::Poll() {
    if (InStrikeAPose()) {
        TheHamDirector->GetVenueWorld()->Find<HamCharacter>("backup0")->SetShowing(false);
        TheHamDirector->GetVenueWorld()->Find<HamCharacter>("backup1")->SetShowing(false);
    }
    if (mEndPoseFatalitiesIn > 0) {
        mEndPoseFatalitiesIn -= TheTaskMgr.DeltaBeat();
        if (mEndPoseFatalitiesIn <= 0) {
            static Message msg("fatals_over");
            TheHamDirector->HandleType(msg);
            TheHamProvider->SetProperty("in_fatalities", 0);
        }
    }
    int deltaBeat = TheTaskMgr.Beat() + 0.2f;
    if (deltaBeat > mBeat) {
        mBeat = deltaBeat;
        if (mBeat == mJumpStart) {
            mBeat = mJumpEnd;
        }
        if (mBeat == mJumpEnd) {
            MILO_LOG("Jump detected! %d to %d\n", mJumpStart, mJumpEnd);
            int diff = mJumpEnd - mJumpStart;
            for (int i = 0; i < 2; i++) {
                if (mFatalStartBeats[i] != -1) {
                    mFatalStartBeats[i] += diff;
                    mPoseStartBeat[i] += diff;
                }
            }
        }
        OnBeat(mBeat);
    }
    if (mInFatalityMode) {
        for (int i = 0; i < 2; i++) {
            if (InFatality(i) || mFatalOver[i]) {
                UpdateClipDriver(i);
            }
            if (InFatality(i)) {
                CharCameraInput input(TheHamDirector->GetCharacter(i));
                input.SetFlip(true);
                input.PollTracking();
                const SkeletonFrame *frame = input.NewFrame();
                if (frame) {
                    mPoseSkel[i].Poll(0, *frame);
                }
            }
            UpdateMatchingPose(i);
            mPoseBeatAnims[TheGameData->Player(i)->Side()]->SetFrame(
                4.0f - BeatsLeftToMatch(i), 1
            );
        }

        // probably an inline
        bool p12 = mBeat < mFatalStartBeats[0] ? false : mFatalActive[0];

        bool b9 =
            p12 || mGotFullCombo[0] || !(InStrikeAPose() || mBeat >= mFatalStartBeats[0]);

        // also probably an inline
        p12 = mBeat < mFatalStartBeats[1] ? false : mFatalActive[1];
        bool b10 =
            p12 || mGotFullCombo[1] || !(InStrikeAPose() || mBeat >= mFatalStartBeats[1]);

        int prop;
        if (b9 && b10) {
            prop = 2;
        } else if (b9) {
            prop = 0;
        } else if (b10) {
            prop = 1;
        } else {
            prop = 3;
        }
        static Symbol dance_battle_config("dance_battle_config");
        TheHamProvider->SetProperty(dance_battle_config, prop);
        mHoldDuration = TheOSCMessenger.GetFloat("/holdduration", 0.5f);
        PollVO();
    }
}

void PoseFatalities::AddFatal(int player) {
    mPoseStartBeat[player] = mBeat;
    if (InStrikeAPose() && !DataVarExists("debug_pose_char")) {
        int cur = mFatalityIndex[player];
        int rand;
        do {
            rand = RandomInt(1, 9);
        } while (rand == cur);
        mFatalityIndex[player] = rand;
    } else {
        mFatalityIndex[player]++;
    }
    mHoldScore[player] = 0;
    mClipOffset[player] = -0.5f;
    HamCharacter *hChar = TheHamDirector->GetCharacter(player);
    CharDriver *driver = hChar->Driver();
    CharClip *randClip;
    if (InStrikeAPose()) {
        if (DataVarExists("debug_pose_char")) {
            FOREACH (it, mAllFatalityClips) {
                const char *fatalStr = MakeString(
                    "pose_fatalities_%s", DataVariable("debug_pose_char").Str()
                );
                randClip = *it;
                if (streq(randClip->Name(), fatalStr)) {
                    MILO_LOG(
                        "%s %s\n",
                        fatalStr,
                        MakeString("pose_fatality_%i", mFatalityIndex[player] - 1)
                    );
                    goto lab5548;
                }
            }
            goto lab55b4;
        }
        int randListIdx = RandomInt(0, mAllFatalityClips.size());
        auto it = mAllFatalityClips.begin();
        for (int i = 0; i < randListIdx; ++i, ++it)
            ;
        randClip = *it;
    } else {
        randClip = driver->FindClip(
            MakeString("pose_fatalities_%s", GetOutfitCharacter(hChar->Outfit()))
        );
    }
lab5548:
    if (randClip) {
        driver->Play(randClip, 0x402, -1, kHugeFloat, 0);
        TheHamDirector->CurShot()->Reteleport(
            Vector3::GetZero(), false, MakeString("player%d", player)
        );
    }
lab55b4:
    SetCombo(player, mComboLevel[player] + 1);
    static Message addedFatalityMsg("fatality_added", 0, 0);
    addedFatalityMsg[0] = player;
    addedFatalityMsg[1] = mComboLevel[player];
    TheHamProvider->Handle(addedFatalityMsg, false);
    hChar->BlendInFaceOverrideClip(GetFatalityFace(), 1, 1);
}

void PoseFatalities::OnFatalResult(int player, bool hit) {
    if (mFatalActive[player]) {
        if (hit) {
            static Symbol score("score");
            int scoreProp =
                TheGameData->Player(player)->Provider()->Property(score)->Int();
            int newScoreProp;
            if (mComboLevel[player] < 8) {
                newScoreProp = mComboLevel[player] * 2000 + scoreProp;
            } else {
                newScoreProp = scoreProp + 40000;
            }
            TheGameData->Player(player)->Provider()->SetProperty(score, newScoreProp);
            static Message poseMatchedMsg("fatality_matched", 0, 0, 0);
            poseMatchedMsg[0] = player;
            poseMatchedMsg[1] = mComboLevel[player];
            poseMatchedMsg[2] = TheGameData->Player(player)->Side();
            TheHamProvider->Handle(poseMatchedMsg, false);
            mTimeSinceVORequest = 0;
            mVORequests |= 4;
            if (InStrikeAPose()) {
                TheSynth->Find<RndAnimatable>("beat_repeat.anim")
                    ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
            }
        } else {
            static Message poseMissedMsg("fatality_missed", 0, 0);
            poseMissedMsg[0] = player;
            poseMissedMsg[1] = TheGameData->Player(player)->Side();
            TheHamProvider->Handle(poseMissedMsg, false);
            HamCharacter *hChar = TheHamDirector->GetCharacter(player);
            hChar->BlendInFaceOverrideClip("Angry", 1, 1);
        }
        static Symbol move_perfect("move_perfect");
        static Symbol move_bad("move_bad");
        static Message moveFinishedMsg("move_finished", 0, 0);
        moveFinishedMsg[0] = player;
        moveFinishedMsg[1] = hit ? move_perfect : move_bad;
        TheHamProvider->Handle(moveFinishedMsg, false);
        bool b11 = false;
        if (hit) {
            if (mComboLevel[player] == 8) {
                static Message poseAllMatchedMsg("fatality_all_matched", 0, 0);
                poseAllMatchedMsg[0] = player;
                poseAllMatchedMsg[1] = TheGameData->Player(player)->Side();
                TheHamProvider->Handle(poseAllMatchedMsg, false);
                mGotFullCombo[player] = true;
                mHoldScore[player] = 0;
                if (TheGameData->Player(player)->Side() == kSkeletonLeft) {
                    mTimeSinceVORequest = 0;
                    mVORequests |= 1;
                }
                if (TheGameData->Player(player)->Side() == kSkeletonRight) {
                    mTimeSinceVORequest = 0;
                    mVORequests |= 2;
                }
                CharDriver *driver = TheHamDirector->GetCharacter(player)->Driver();
                CharClip *celebrationClip = driver->FindClip(GetCelebrationClip(player));
                if (celebrationClip) {
                    driver->Play(celebrationClip, 2, -1, kHugeFloat, 0);
                }
            } else if (mBeat < mFatalEndBeat) {
                AddFatal(player);
                b11 = true;
            }
        }
        if (!b11) {
            EndFatal(player);
        }
    }
}

void PoseFatalities::OnBeat(int beat) {
    if (InStrikeAPose() && !DataVarExists("debug_pose_char")) {
        if (beat >= mFatalEndBeat) {
            for (int i = 0; i < 2; i++) {
                mFatalActive[i] = false;
                TheHamDirector->GetCharacter(i)->BlendOutFaceOverrides(100);
            }
            return;
        }
    } else {
        if (beat + 4 >= mFatalStartBeats[0] && (mFatalActive[0] || mFatalActive[1])) {
            static Symbol game_stage("game_stage");
            static Symbol playing("playing");
            TheHamProvider->SetProperty(game_stage, playing);
            if (beat + 4 == mFatalStartBeats[0]) {
                PlayVO("nar_pose_fatalities");
            }
        }
    }
    for (int i = 0; i < 2; i++) {
        if (beat == mFatalStartBeats[i]) {
            BeginFatal(i);
        }
    }
    if (InFatality(-1)) {
        static Message beatMsg("fatality_beat");
        TheHamProvider->Handle(beatMsg, false);
    } else if (FatalActive()) {
        int startBeat = 0;
        for (int i = 0; i < 2; i++) {
            int cur = mFatalStartBeats[i];
            if (cur != -1) {
                startBeat = cur;
                if (startBeat - beat >= 10) {
                    MILO_FAIL(
                        "Start beat is too far off!  Player %d, startBeat %d, beat %d\n",
                        i,
                        startBeat,
                        beat
                    );
                }
            }
        }
        static Message beatLeadInMsg("fatality_lead_in_beat", 0);
        beatLeadInMsg[0] = startBeat - beat;
        TheHamProvider->Handle(beatLeadInMsg, false);
    }
    if (!DataVariable("fatal_debug").Int()) {
        if (!DataVarExists("debug_pose_char")) {
            for (int i = 0; i < 2; i++) {
                if (InFatality(i)) {
                    if (CheckMatchingPose(i)) {
                        OnFatalResult(i, true);
                    } else if ((int)BeatsLeftToMatch(i) <= 0) {
                        OnFatalResult(i, false);
                    }
                }
            }
            return;
        }
    }
    DataVariable("debug_endless_strikeapose") = 1;
    bool p9 = mBeat < mFatalStartBeats[0] ? false : mFatalActive[0];
    if (p9) {
        JoypadData *jData = JoypadGetPadData(0);
        if (jData->RX() > 0.5f) {
            AddFatal(0);
        }
        if (jData->RX() < -0.5f) {
            mFatalityIndex[0] -= 2;
            AddFatal(0);
        }
        if (jData->LY() > 0.5f) {
            OnFatalResult(0, false);
        }
    }
}

void PoseFatalities::UpdateClipDriver(int player) {
    HamCharacter *character = TheHamDirector->GetCharacter(player);
    CharClipDriver *clipDriver = character->Driver()->First();
    if (clipDriver) {
        while (clipDriver != 0) {
            if (strstr(clipDriver->Clip()->Name(), "pose_fatalities_")) {
                break;
            }
            clipDriver = clipDriver->Next();
        }
        if (clipDriver && clipDriver->Clip()) {
            MILO_ASSERT(NUM_FATALITIES == clipDriver->NumBeatEvents(), 0x123);
            int num = mFatalityIndex[player];
            clipDriver->SetBeatOffset(
                mClipOffset[player], kTaskBeats, MakeString("pose_fatality_%i", num - 1)
            );
            if (mClipOffset[player] < 0) {
                mClipOffset[player] += TheTaskMgr.DeltaUISeconds();
            }
        }
    }
}

void PoseFatalities::UpdateMatchingPose(int player) {
    bool b2 = false;
    mSatisfaction[player] = 0;
    float f12 = Clamp(0.0f, 1.0f, TheTaskMgr.DeltaBeat());
    if (InFatality(player)) {
        const Skeleton *skeleton = TheGameData->Player(player)->GetSkeleton();
        float f13 = mRecorder.CompareSkeletonPositions(
            skeleton,
            &mPoseSkel[player],
            TheOSCMessenger.GetFloat("/fatalposeerrorweight", 1)
        );
        mSatisfaction[player] = f13 / TheOSCMessenger.GetFloat("/fatalposethresh", 0.5f);
        // NaN check
        if (mSatisfaction[player] != mSatisfaction[player]) {
            mSatisfaction[player] = 0;
        }
        ClampEq(mSatisfaction[player], 0.0f, 1.0f);
        b2 = mSatisfaction[player] >= 1 && mClipOffset[player] >= 0;
        if (mClipOffset[player] >= 0) {
            JoypadData *jData = JoypadGetPadData(0);
            if (player == 0) {
                if (jData->RT() > 0.5f || !TheGameData->Player(0)->Autoplay().Null()) {
                    b2 = true;
                }
            }
            if (player == 1) {
                if (jData->LT() > 0.5f || !TheGameData->Player(1)->Autoplay().Null()) {
                    b2 = true;
                }
            }
        }
    }
    if (b2) {
        mHoldScore[player] += f12;
    } else {
        mHoldScore[player] *=
            1 - Clamp(0.0f, 1.0f, TheOSCMessenger.GetFloat("/holddecay", 1) * f12);
    }
    float frac = Clamp(0.0f, 1.0f, mHoldScore[player] / mHoldDuration);
    WorldDir *world = TheHamDirector->GetVenueWorld();
    HamPhraseMeter *hpm =
        world->Find<HamPhraseMeter>(MakeString("phrase_meter%i", player));
    hpm->SetRatingFrac(frac, -1);
    hpm->SetShowing(true);
    hpm->Find<RndAnimatable>("perimeter_feedback_color.anim")
        ->SetFrame(mSatisfaction[player] * 4, 1);
}

void PoseFatalities::DrawDebug() {
    static SkeletonViz *sViz1 = nullptr;
    static SkeletonViz *sViz2 = nullptr;
    if (!sViz2) {
        sViz2 = Hmx::Object::New<SkeletonViz>();
        sViz2->Init();
        sViz1 = Hmx::Object::New<SkeletonViz>();
        sViz1->Init();
    }
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();

    static float sFloatd7d4 = 0.3f;
    static float sFloatd7d8 = 0.1f;
    static float sFloatd7dc = 0.3f;
    static float sFloatd7e0 = 0.2f;

    float f8 = sFloatd7dc / TheRnd.YRatio();
    if (mBeat < mFatalStartBeats[0] ? false : mFatalActive[0]) {
        if (DataVariable("fatal_debug").Int()) {
            const Skeleton *skeleton = TheGameData->Player(0)->GetSkeleton();
            static DebugMeter sDebugMeter1(0.1f, 0.1f, 0.5f, 0.1f, Hmx::Color(0, 0, 0));
            sDebugMeter1.Draw();
            sDebugMeter1.DrawBar(
                0,
                mRecorder.CompareSkeletonPositions(skeleton, &mPoseSkel[0], 1),
                Hmx::Color(0, 1, 0)
            );

            float f9 = mRecorder.CompareSkeletonPositions(
                skeleton,
                &mPoseSkel[0],
                TheOSCMessenger.GetFloat("/fatalposeerrorweight", 0)
            );
            float f17 = TheOSCMessenger.GetFloat("/fatalposethresh", 0);
            float f13 = f9 / f17;
            // NaN check
            if (f13 != f13) {
                f13 = 0;
            }
            ClampEq(f13, 0.0f, 1.0f);

            static DebugMeter sDebugMeter2(0.1f, 0.3f, 0.5f, 0.1f, Hmx::Color(0, 0, 0));
            sDebugMeter2.Draw();
            sDebugMeter2.DrawBar(0, 1, Hmx::Color(0, 0, f13 * f13));
            sDebugMeter2.DrawBar(0, f9, Hmx::Color(0, 1, 0));

            static DebugMeter sDebugMeter3(
                sFloatd7d8 + sFloatd7dc + 0.1f,
                sFloatd7d4,
                sFloatd7dc,
                0.03f,
                Hmx::Color(0, 0, 0)
            );
            sDebugMeter3.Draw();
            sDebugMeter3.DrawBar(
                0, 1, Hmx::Color(0, 0, mSatisfaction[0] * mSatisfaction[0])
            );
            sDebugMeter3.DrawBar(
                0, Clamp(0.0f, 1.0f, mHoldScore[0] / mHoldDuration), Hmx::Color(0, 1, 0)
            );
        }

        if (TheOSCMessenger.GetInt("/posefatalitiesdrawdebugskel", 0)) {
            Hmx::Rect r(sFloatd7d8 + sFloatd7dc + 0.1f, sFloatd7d4, sFloatd7dc, f8);
            TheRnd.DrawRectScreen(r, Hmx::Color(0, 0, 0, 0.4f), nullptr, nullptr, nullptr);
            sViz2->SetUsePhysicalCam(true);
            sViz2->SetPhysicalCamScreenRect(r);
            sViz2->Visualize(*handle.GetCameraInput(), mPoseSkel[0], nullptr, false);
        }
    }

    if (mBeat < mFatalStartBeats[1] ? false : mFatalActive[1]) {
        if (DataVariable("fatal_debug").Int()) {
            static DebugMeter sDebugMeter4(
                sFloatd7d8, sFloatd7d4, sFloatd7dc, 0.03f, Hmx::Color(0, 0, 0)
            );
            sDebugMeter4.Draw();
            sDebugMeter4.DrawBar(
                0, 1, Hmx::Color(0, 0, mSatisfaction[1] * mSatisfaction[1])
            );
            sDebugMeter4.DrawBar(
                0, Clamp(0.0f, 1.0f, mHoldScore[1] / mHoldDuration), Hmx::Color(0, 1, 0)
            );
        }
        if (TheOSCMessenger.GetInt("/posefatalitiesdrawdebugskel", 0)) {
            Hmx::Rect r(sFloatd7d8, sFloatd7d4, sFloatd7dc, f8);
            TheRnd.DrawRectScreen(r, Hmx::Color(0, 0, 0, 0.4f), nullptr, nullptr, nullptr);
            sViz1->SetUsePhysicalCam(true);
            sViz1->SetPhysicalCamScreenRect(r);
            sViz1->Visualize(*handle.GetCameraInput(), mPoseSkel[1], nullptr, false);
        }
    }
}
