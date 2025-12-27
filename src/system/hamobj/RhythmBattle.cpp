#include "hamobj/RhythmBattle.h"
#include "RhythmBattlePlayer.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/ArchiveSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
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
