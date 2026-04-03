#include "hamobj/PoseFatalities.h"
#include "PoseFatalities.h"
#include "char/CharClip.h"
#include "char/CharDriver.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "hamobj/CharCameraInput.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMaster.h"
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
#include "rndobj/PropKeys.h"
#include "synth/FxSendDelay.h"
#include "synth/Synth.h"
#include "utl/BeatMap.h"
#include "utl/OSCMessenger.h"
#include "utl/Symbol.h"

PoseFatalities::PoseFatalities()
    : unk15fc(0.5f), mHudPanel(0), mJumpStart(0), mJumpEnd(0), unk1754(0), unk1764(0),
      unk1768(0), unk176c(0) {
    for (int i = 0; i < 2; i++) {
        mInFatality[i] = 0;
        unk1710[i] = 0;
        unk1720[i] = 0;
        mGotFullCombo[i] = 0;
        unk3c[i] = 0;
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
        if (mInFatality[i]) {
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
    int max = unk1754;
    if (player == -1) {
        bool b1 = false;
        for (int i = 0; i < 2; i++) {
            if (max >= mFatalStartBeats[i]) {
                b1 = true;
            }
        }
        if (b1) {
            return FatalActive();
        }
    } else {
        MILO_ASSERT_RANGE(player, 0, 2, 0x391);
        if (max >= mFatalStartBeats[player]) {
            return mInFatality[player];
        }
    }
    return false;
}

bool PoseFatalities::InStrikeAPose() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol strike_a_pose("strike_a_pose");
    return TheHamProvider->Property(gameplay_mode, true)->Sym() == strike_a_pose;
}

void PoseFatalities::SetCombo(int player, int combo) {
    mCurrentCombo[player] = combo;
    if (combo != 0) {
        mPoseComboLabels[TheGameData->Player(player)->Side()]->SetInt(combo, false);
    }
}

void PoseFatalities::SetFatalitiesStart(int player) {
    mInFatality[player] = true;
    if (InStrikeAPose()) {
        mFatalStartBeats[player] = unk1754 + 3;
        for (int i = 0; i < 4; i++) {
            if (TheBeatMap->IsDownbeat(mFatalStartBeats[player])) {
                return;
            }
            mFatalStartBeats[player]++;
        }
    } else {
        mFatalStartBeats[player] = unk1754 + 7;
    }
}

void PoseFatalities::Reset() {
    for (int i = 0; i < 2; i++) {
        mFatalStartBeats[i] = -1;
        mInFatality[i] = false;
        unk15f4[i] = 0;
    }
    TheHamProvider->SetProperty("in_fatalities", 0);
    RndAnimatable *anim = TheSynth->Find<RndAnimatable>("beat_repeat.anim", true);
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
    unk1764 = 5;
}

String PoseFatalities::GetCelebrationClip(int player) {
    Symbol outfit =
        GetOutfitCharacter(TheHamDirector->GetCharacter(player)->Outfit(), true);
    static Symbol strikeapose_celebrations("strikeapose_celebrations");
    DataArray *cfg = SystemConfig(strikeapose_celebrations);
    DataArray *a = cfg->FindArray(outfit, true);
    return a->Node(RandomInt(1, a->Size())).Str();
}

void PoseFatalities::EndFatal(int player) {
    bool b10 = true;
    mInFatality[player] = false;
    unk1720[player] = true;
    static Message endFatalityMsg("fatality_end", 0);
    endFatalityMsg[0] = player;
    TheHamProvider->Handle(endFatalityMsg, false);
    if (!InStrikeAPose()) {
        ObjectDir *poseDisplay = TheHamDirector->GetVenueWorld()->Find<ObjectDir>(
            MakeString("final_pose_display%d", player), true
        );
        HamLabel *label = poseDisplay->Find<HamLabel>("pose_combo.lbl", true);
        int num;
        if (mGotFullCombo[player]) {
            num = 8;
        } else {
            num = mCurrentCombo[player] - 1;
        }
        if (mCurrentCombo[player] > 1) {
            label->SetTokenFmt("pose_fatality_3d_combo", num);
        } else {
            label->SetTextToken(gNullStr);
        }
        poseDisplay->Find<RndAnimatable>("pose_combo.anim", true)
            ->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
    }
    mCurrentCombo[player] = 0;
    if (!DataVariable("restart_fatals").Int() && !InStrikeAPose()) {
        for (int i = 0; i < 2; i++) {
            b10 &= mInFatality[i] != 0;
        }
        if (b10) {
            unk1760 = 4;
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
        unk1748 = true;
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
    if (fabsf(beat - unk1754) > 2.0f) {
        beat = (mJumpEnd - mJumpStart) + beat;
    }
    int num = 4;
    if (unk44[player] == 1 && !InStrikeAPose()) {
        num = 8;
    }
    return (unk3c[player] + num) - beat;
}

void PoseFatalities::BeginFatal(int player) {
    if (mInFatality[player]) {
        unk1720[player] = false;
        if (!InStrikeAPose()) {
            unk44[player] = 0;
        }
        mCurrentCombo[player] = 0;
        mGotFullCombo[player] = false;
        AddFatal(player);
        static Message beginFatalityMsg("fatality_begin", 0);
        beginFatalityMsg[0] = player;
        TheHamProvider->Handle(beginFatalityMsg, false);
        TheHamDirector->GetCharacter(player)->BlendInFaceOverrides(100);
    }
}

bool PoseFatalities::CheckMatchingPose(int player) {
    return InFatality(player) && unk15f4[player] >= unk15fc;
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
        mPoseComboLabels[kSkeletonLeft] = mHudPanel->Find<ObjectDir>("hud_left", true)
                                              ->Find<HamLabel>("pose_combo.lbl", true);
        mPoseComboLabels[kSkeletonRight] = mHudPanel->Find<ObjectDir>("hud_right", true)
                                               ->Find<HamLabel>("pose_combo.lbl", true);
        mPoseBeatAnims[kSkeletonLeft] = mHudPanel->Find<ObjectDir>("hud_left", true)
                                            ->Find<RndAnimatable>("pose_beat.anim", true);
        mPoseBeatAnims[kSkeletonRight] =
            mHudPanel->Find<ObjectDir>("hud_right", true)
                ->Find<RndAnimatable>("pose_beat.anim", true);
        unk1748 = InStrikeAPose();
        FxSendDelay *delay = TheSynth->Find<FxSendDelay>("BeatRepeat.send", true);
        if (delay) {
            float bpm = TheMaster->SongData()->GetTempoMap()->GetTempoBPM(0);
            delay->SetProperty("tempo", bpm);
        }
        unk1754 = 0;
        unk1760 = -1;
    }
}

void PoseFatalities::PollVO() {
    if (InStrikeAPose()) {
        bool flag1 = unk1768 & 1;
        bool flag2 = (unk1768 >> 1) & 1;
        if (flag1) {
            if (flag2) {
                PlayVO("nar_sap_both_fc");
            } else {
                PlayVO("nar_sap_left_fc");
            }
        } else if (flag2) {
            PlayVO("nar_sap_right_fc");
        }
        if (unk1764 <= 0) {
            if (unk1768 & 4) {
                PlayVO("nar_sap_gen_pos");
            } else if (unk176c > 8.0f) {
                PlayVO("nar_sap_time_limit");
                unk176c = 0;
            }
        } else {
            unk1764 -= TheTaskMgr.DeltaSeconds();
        }
        unk1768 = 0;
        if (InFatality(-1)) {
            unk176c += Clamp(0.0f, 1.0f, TheTaskMgr.DeltaSeconds());
        }
    }
}

void PoseFatalities::Poll() {
    if (InStrikeAPose()) {
        TheHamDirector->GetVenueWorld()
            ->Find<HamCharacter>("backup0", true)
            ->SetShowing(false);
        TheHamDirector->GetVenueWorld()
            ->Find<HamCharacter>("backup1", true)
            ->SetShowing(false);
    }
    if (unk1760 > 0) {
        unk1760 -= TheTaskMgr.DeltaBeat();
        if (unk1760 <= 0) {
            static Message msg("fatals_over");
            TheHamDirector->HandleType(msg);
            TheHamProvider->SetProperty("in_fatalities", 0);
        }
    }
    int deltaBeat = TheTaskMgr.Beat() + 0.2f;
    if (deltaBeat > unk1754) {
        unk1754 = deltaBeat;
        if (unk1754 == mJumpStart) {
            unk1754 = mJumpEnd;
        }
        if (unk1754 == mJumpEnd) {
            MILO_LOG("Jump detected! %d to %d\n", mJumpStart, mJumpEnd);
            int diff = mJumpEnd - mJumpStart;
            for (int i = 0; i < 2; i++) {
                if (mFatalStartBeats[i] != -1) {
                    mFatalStartBeats[i] += diff;
                    unk3c[i] += diff;
                }
            }
        }
        OnBeat(unk1754);
    }
    if (unk1748) {
        for (int i = 0; i < 2; i++) {
            if (InFatality(i) || unk1720[i]) {
                UpdateClipDriver(i);
            }
            if (InFatality(i)) {
                CharCameraInput input(TheHamDirector->GetCharacter(i));
                input.SetUnk2430(true);
                input.PollTracking();
                const SkeletonFrame *frame = input.NewFrame();
                if (frame) {
                    mPlayerSkeletons[i].Poll(0, *frame);
                }
            }
            UpdateMatchingPose(i);
            mPoseBeatAnims[TheGameData->Player(i)->Side()]->SetFrame(
                4.0f - BeatsLeftToMatch(i), 1
            );
        }

        // probably an inline
        bool p12 = unk1754 < mFatalStartBeats[0] ? false : mInFatality[0];

        bool b9 = p12 || mGotFullCombo[0]
            || !(InStrikeAPose() || unk1754 >= mFatalStartBeats[0]);

        // also probably an inline
        p12 = unk1754 < mFatalStartBeats[1] ? false : mInFatality[1];
        bool b10 = p12 || mGotFullCombo[1]
            || !(InStrikeAPose() || unk1754 >= mFatalStartBeats[1]);

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
        unk15fc = TheOSCMessenger.GetFloat("/holdduration", 0.5f);
        PollVO();
    }
}

void PoseFatalities::AddFatal(int player) {
    unk3c[player] = unk1754;
    if (InStrikeAPose() && !DataVarExists("debug_pose_char")) {
        int cur = unk44[player];
        int rand;
        do {
            rand = RandomInt(1, 9);
        } while (rand == cur);
        unk44[player] = rand;
    } else {
        unk44[player]++;
    }
    unk15f4[player] = 0;
    unk1718[player] = -0.5f;
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
                        MakeString("pose_fatality_%i", unk44[player] - 1)
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
        const char *str =
            MakeString("pose_fatalities_%s", GetOutfitCharacter(hChar->Outfit()));
        randClip = driver->FindClip(str, true);
    }
lab5548:
    if (randClip) {
        driver->Play(randClip, 0x402, -1, kHugeFloat, 0);
        TheHamDirector->CurShot()->Reteleport(
            Vector3::GetZero(), false, MakeString("player%d", player)
        );
    }
lab55b4:
    SetCombo(player, mCurrentCombo[player] + 1);
    static Message addedFatalityMsg("fatality_added", 0, 0);
    addedFatalityMsg[0] = player;
    addedFatalityMsg[1] = mCurrentCombo[player];
    TheHamProvider->Handle(addedFatalityMsg, false);
    hChar->BlendInFaceOverrideClip(GetFatalityFace(), 1, 1);
}

void PoseFatalities::OnFatalResult(int player, bool hit) {
    if (mInFatality[player]) {
        if (hit) {
            static Symbol score("score");
            int scoreProp =
                TheGameData->Player(player)->Provider()->Property(score, true)->Int();
            int newScoreProp;
            if (mCurrentCombo[player] < 8) {
                newScoreProp = mCurrentCombo[player] * 2000 + scoreProp;
            } else {
                newScoreProp = scoreProp + 40000;
            }
            TheGameData->Player(player)->Provider()->SetProperty(score, newScoreProp);
            static Message poseMatchedMsg("fatality_matched", 0, 0, 0);
            poseMatchedMsg[0] = player;
            poseMatchedMsg[1] = mCurrentCombo[player];
            poseMatchedMsg[2] = TheGameData->Player(player)->Side();
            TheHamProvider->Handle(poseMatchedMsg, false);
            unk176c = 0;
            unk1768 |= 4;
            if (InStrikeAPose()) {
                TheSynth->Find<RndAnimatable>("beat_repeat.anim", true)
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
            if (mCurrentCombo[player] == 8) {
                static Message poseAllMatchedMsg("fatality_all_matched", 0, 0);
                poseAllMatchedMsg[0] = player;
                poseAllMatchedMsg[1] = TheGameData->Player(player)->Side();
                TheHamProvider->Handle(poseAllMatchedMsg, false);
                mGotFullCombo[player] = true;
                unk15f4[player] = 0;
                if (TheGameData->Player(player)->Side() == kSkeletonLeft) {
                    unk176c = 0;
                    unk1768 |= 1;
                }
                if (TheGameData->Player(player)->Side() == kSkeletonRight) {
                    unk176c = 0;
                    unk1768 |= 2;
                }
                CharDriver *driver = TheHamDirector->GetCharacter(player)->Driver();
                CharClip *celebrationClip =
                    driver->FindClip(GetCelebrationClip(player), true);
                if (celebrationClip) {
                    driver->Play(celebrationClip, 2, -1, kHugeFloat, 0);
                }
            } else if (unk1754 < mFatalEndBeat) {
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
                mInFatality[i] = false;
                TheHamDirector->GetCharacter(i)->BlendOutFaceOverrides(100);
            }
            return;
        }
    } else {
        if (beat + 4 >= mFatalStartBeats[0] && (mInFatality[0] || mInFatality[1])) {
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
    bool p9 = unk1754 < mFatalStartBeats[0] ? false : mInFatality[0];
    if (p9) {
        JoypadData *jData = JoypadGetPadData(0);
        if (jData->RX() > 0.5f) {
            AddFatal(0);
        }
        if (jData->RX() < -0.5f) {
            unk44[0] -= 2;
            AddFatal(0);
        }
        if (jData->LY() > 0.5f) {
            OnFatalResult(0, false);
        }
    }
}
