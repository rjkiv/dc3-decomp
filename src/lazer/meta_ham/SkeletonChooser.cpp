#include "meta_ham/SkeletonChooser.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DirectionGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/HandRaisedGestureFilter.h"
#include "gesture/HighFiveGestureFilter.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonRecoverer.h"
#include "gesture/StandingStillGestureFilter.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Vec.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/PassiveMessenger.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

SkeletonChooser::SkeletonChooser()
    : mDrawDebug(false), unk3c(0), unk44(1), unk48(true), unk80(0), unk84(0), unk88(0),
      unk8c(0), unk90(0), mNextSkelIdxToTrack(-1), mInMultiPlayerUpdateMode(false),
      unkbc(-1), mEnrollmentLocked(false) {
    SetName("skeleton_chooser", ObjectDir::Main());
    unk2c = new DirectionGestureFilterSingleUser(kSkeletonRight, kSkeletonLeft, 0, 0);
    unk30 = new DirectionGestureFilterSingleUser(kSkeletonLeft, kSkeletonRight, 0, 0);
    unk7c = Hmx::Object::New<HighFiveGestureFilter>();
    for (int i = 0; i < 6; i++) {
        unk4c[i] = Hmx::Object::New<HandRaisedGestureFilter>();
        unk4c[i]->SetRequiredMs(750);
        unk4c[i]->Clear();
        unk64[i] = new StandingStillGestureFilter();
        unk64[i]->SetRequiredMs(750);
        unka4[i] = 0;
    }
    for (int i = 0; i < 2; i++) {
        mHandRaisedFilters[i] = Hmx::Object::New<HandRaisedGestureFilter>();
        mHandRaisedFilters[i]->SetRequiredMs(750);
        mHandRaisedFilters[i]->Clear();
    }
}

SkeletonChooser::~SkeletonChooser() {
    delete unk2c;
    delete unk30;
    for (int i = 0; i < 6; i++) {
        delete unk4c[i];
        delete unk64[i];
    }
    delete unk7c;
}

BEGIN_HANDLERS(SkeletonChooser)
    HANDLE_EXPR(toggle_draw_debug, mDrawDebug = !mDrawDebug)
    HANDLE_ACTION(
        switch_active_to_player_index_immediate,
        SwitchActiveToPlayerIndexImmediate(_msg->Int(2))
    )
    HANDLE_EXPR(check_hand_raised, IsHandRaised(_msg->Int(2)))
    HANDLE(get_joint_depth_pos, OnGetJointDepthPos)
    HANDLE_EXPR(is_skeleton_valid, IsSkeletonValid(_msg->Int(2)))
    HANDLE_ACTION(lock_enrollment, mEnrollmentLocked = _msg->Int(2))
    HANDLE_EXPR(is_enrollment_locked, mEnrollmentLocked)
    HANDLE_EXPR(is_left_player_hand_raised, IsLeftPlayerHandRaised())
    HANDLE_EXPR(is_right_player_hand_raised, IsRightPlayerHandRaised())
    HANDLE_ACTION(force_swap_player_sides, ForceSwapPlayerSides())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

int SkeletonChooser::GetAssignedPlayerSkeletonID(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x7F);
    return TheGameData->Player(player)->GetSkeletonTrackingID();
}

bool SkeletonChooser::AreArmsCrossed(int id) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(id);
    MILO_ASSERT(pSkeleton, 0x55F);
    Vector2 leftHandPos, rightHandPos;
    pSkeleton->ScreenPos(kJointHandLeft, leftHandPos);
    pSkeleton->ScreenPos(kJointHandRight, rightHandPos);
    if (leftHandPos.x > rightHandPos.x) {
        return true;
    } else {
        return false;
    }
}

void SkeletonChooser::GetJointDepthPos(int i1, int i2, Vector3 &v) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(
        TheGestureMgr->GetSkeleton(i1).TrackingID()
    );
    if (pSkeleton) {
        Vector2 jointPos;
        pSkeleton->ScreenPos((SkeletonJoint)i2, jointPos);
        Vector3 jointV3 = pSkeleton->TrackedJoints()[i2].unk60;
        v.z = jointV3.z;
        v.x = jointPos.x;
        v.y = jointPos.y;
    } else {
        v.Zero();
    }
}

bool SkeletonChooser::IsSkeletonValid(int idx) {
    return TheGestureMgr->GetSkeletonByTrackingID(
        TheGestureMgr->GetSkeleton(idx).TrackingID()
    );
}

bool SkeletonChooser::IsHandRaised(int idx) {
    if (IsSkeletonValid(idx)) {
        return unka4[idx];
    } else {
        return false;
    }
}

bool SkeletonChooser::IsPlayerHandRaised(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x6DC);
    return mHandRaisedFilters[player]->HandRaised();
}

void SkeletonChooser::ClearPlayerSkeletonID(int id) {
    SetPlayerSkeletonID(id, -1);
    TheGestureMgr->SetPlayerSkeletonID(id, -1);
}

SkeletonSide SkeletonChooser::GetPlayerSide(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x86);
    HamPlayerData *pPlayer = TheGameData->Player(player);
    MILO_ASSERT(pPlayer, 0x89);
    Hmx::Object *pPlayerProvider = pPlayer->Provider();
    MILO_ASSERT(pPlayerProvider, 0x8B);
    static Symbol side("side");
    const DataNode *pPlayerSide = pPlayerProvider->Property(side);
    MILO_ASSERT(pPlayerSide, 0x8F);
    return (SkeletonSide)pPlayerSide->Int();
}

void SkeletonChooser::SetActivePlayer(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x635);
    unk3c = playerIndex;
    int skeletonTrackingID = TheGameData->Player(playerIndex)->GetSkeletonTrackingID();
    TheGestureMgr->SetActiveSkeletonTrackingID(skeletonTrackingID);
}

bool SkeletonChooser::IsLeftPlayerHandRaised() {
    for (int i = 0; i < 2; i++) {
        if (GetPlayerSide(i) == kSkeletonLeft) {
            return IsPlayerHandRaised(i);
        }
    }
    return false;
}

bool SkeletonChooser::IsRightPlayerHandRaised() {
    for (int i = 0; i < 2; i++) {
        if (GetPlayerSide(i) == kSkeletonRight) {
            return IsPlayerHandRaised(i);
        }
    }
    return false;
}

void SkeletonChooser::Poll() {
    if (!PotentiallyRecoverSkeletons()) {
        CheckToSwitchActivePlayer();
        if (unk38 >= 0) {
            unk40 += TheTaskMgr.DeltaUISeconds();
            if (unk40 > unk44) {
                Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(
                    TheGestureMgr->GetPlayerSkeletonID(unk38)
                );
                if (pSkeleton && pSkeleton->IsTracked()) {
                    SetActivePlayer(unk38);
                }
                unk38 = -1;
                unk40 = 0;
            }
        }
        if (unk38 < 0) {
            UpdateTrackedSkeletonsElective();
            unk7c->Update(
                TheGestureMgr->GetSkeletonByTrackingID(
                    TheGameData->Player(0)->GetSkeletonTrackingID()
                ),
                TheGestureMgr->GetSkeletonByTrackingID(
                    TheGameData->Player(1)->GetSkeletonTrackingID()
                )
            );
            if (unk7c->CheckHighFive()) {
                static Message highFiveMsg("high_five");
                TheHamProvider->Handle(highFiveMsg, false);
            }
        }
    }
}

DataNode SkeletonChooser::OnGetJointDepthPos(const DataArray *a) {
    MILO_ASSERT(a->Size() == 7, 0x6B0);
    Vector3 v;
    GetJointDepthPos(a->Int(2), a->Int(3), v);
    *a->Var(4) = v.x;
    *a->Var(5) = v.y;
    *a->Var(6) = v.z;
    return 0;
}

bool SkeletonChooser::GetPlayerPresent(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0xAB);
    static Symbol player_present("player_present");
    HamPlayerData *pPlayer = TheGameData->Player(player);
    MILO_ASSERT(pPlayer, 0xB0);
    Hmx::Object *pPlayerProvider = pPlayer->Provider();
    MILO_ASSERT(pPlayerProvider, 0xB2);
    const DataNode *pPlayerPresent = pPlayerProvider->Property(player_present);
    MILO_ASSERT(pPlayerPresent, 0xB4);
    return pPlayerPresent->Int();
}

void SkeletonChooser::SetPlayerPresent(int player, bool present) {
    if (TheGestureMgr->IsTrackingAllSkeletons())
        return;
    MILO_ASSERT_RANGE(player, 0, 2, 0x98);
    static Symbol player_present("player_present");
    HamPlayerData *pPlayer = TheGameData->Player(player);
    MILO_ASSERT(pPlayer, 0x9D);
    Hmx::Object *pPlayerProvider = pPlayer->Provider();
    MILO_ASSERT(pPlayerProvider, 0x9F);
    bool pPlayerPresent = GetPlayerPresent(player);
    if (pPlayerPresent != present) {
        pPlayerProvider->SetProperty(player_present, present);
    }
}

void SkeletonChooser::EnterMultiPlayerUpdateMode() {
    if (!mInMultiPlayerUpdateMode) {
        TheGestureMgr->StartTrackAllSkeletons();
        mInMultiPlayerUpdateMode = true;
        for (int i = 0; i < 6; i++) {
            unka4[i] = 0;
            unk4c[i]->SetRequiredMs(500);
            unk4c[i]->SetForwardFacingCutoff(0.82f);
            unk4c[i]->Clear();
        }
    }
}

void SkeletonChooser::ExitMultiPlayerUpdateMode() {
    if (mInMultiPlayerUpdateMode) {
        TheGestureMgr->CancelTrackAllSkeletons();
        mInMultiPlayerUpdateMode = false;
        for (int i = 0; i < 6; i++) {
            unka4[i] = 0;
            unk4c[i]->SetRequiredMs(750);
            unk4c[i]->RestoreDefaultForwardFacingCutoff();
            unk4c[i]->Clear();
        }
    }
}

void SkeletonChooser::SetPlayerSkeletonID(int player, int id) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x78);
    TheGameData->AssignSkeleton(player, id);
}

bool SkeletonChooser::IsFreestyleMode() const {
    bool ret = false;
    static Symbol ui_nav_mode("ui_nav_mode");
    const DataNode *pNavPlayerNode = TheHamProvider->Property(ui_nav_mode);
    MILO_ASSERT(pNavPlayerNode, 0x272);
    Symbol navModeSym = pNavPlayerNode->Sym();
    static Symbol game("game");
    if (navModeSym == game) {
        static Symbol freestyle("freestyle");
        static Symbol game_stage("game_stage");
        const DataNode *pGameStageNode = TheHamProvider->Property(game_stage);
        MILO_ASSERT(pGameStageNode, 0x27C);
        Symbol gameStageSym = pGameStageNode->Sym();
        if (gameStageSym == freestyle) {
            ret = true;
        }
    }
    return ret;
}

bool SkeletonChooser::IsSinglePlayerMode() const {
    bool ret = false;
    static Symbol ui_nav_mode("ui_nav_mode");
    const DataNode *pNavPlayerNode = TheHamProvider->Property(ui_nav_mode);
    MILO_ASSERT(pNavPlayerNode, 0x2AF);
    Symbol navModeSym = pNavPlayerNode->Sym();
    static Symbol practice("practice");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol game("game");
    static Symbol results("results");
    static Symbol loading("loading");
    static Symbol pause("pause");
    static Symbol practice_shell("practice_shell");
    static Symbol store("store");
    if (TheGameMode->Property(gameplay_mode)->Sym() == practice
        && (navModeSym == game || navModeSym == results || navModeSym == loading
            || navModeSym == pause || navModeSym == practice_shell
            || navModeSym == store)) {
        ret = true;
    }
    return ret;
}

bool SkeletonChooser::IsAutoplaying() const {
    HamPlayerData *pPlayer1 = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1, 0x3EE);
    HamPlayerData *pPlayer2 = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2, 0x3F0);
    return pPlayer1->IsAutoplaying() || pPlayer2->IsAutoplaying();
}

bool SkeletonChooser::DoesRequireHandRaise() const {
    static Symbol ui_nav_mode("ui_nav_mode");
    static Symbol game("game");
    static Symbol loading("loading");
    const DataNode *uiNavModeProp = TheHamProvider->Property(ui_nav_mode);
    Symbol uiNavModeSym = uiNavModeProp->Sym();
    static Symbol raise_hand_to_join("raise_hand_to_join");
    bool shouldRaiseHand = TheGameMode->Property(raise_hand_to_join)->Int();
    static Symbol is_in_campaign_work_it("is_in_campaign_work_it");
    bool b1 = TheHamProvider->Property(is_in_campaign_work_it)->Int()
        && uiNavModeSym == loading;
    static Symbol doing_stupid_kinect_trick("doing_stupid_kinect_trick");
    bool doingStupidTrickLmao =
        TheHamProvider->Property(doing_stupid_kinect_trick)->Int();
    if (!IsFreestyleMode() && shouldRaiseHand && !b1 && !doingStupidTrickLmao
        && (uiNavModeSym == game || uiNavModeSym == loading)) {
        return true;
    }
    return false;
}

int SkeletonChooser::RoundRobinForPlayer(int x) {
    if (DoesRequireHandRaise()) {
        return RoundRobinForHandRaised(x);
    } else {
        return RoundRobinForStandingStill(x);
    }
}

bool SkeletonChooser::IsCentered(int id) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(id);
    MILO_ASSERT(pSkeleton, 0x5A8);
    if (pSkeleton->IsTracked()) {
        Vector2 spinePos;
        pSkeleton->ScreenPos(kJointSpine, spinePos);
        if (0.35f < spinePos.x && spinePos.x < 0.65f)
            return true;
    }
    return false;
}

void SkeletonChooser::QueueActivePlayerSwitch(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x589);
    if (unk38 != playerIndex) {
        unk38 = playerIndex;
        unk40 = 0;
    }
}

bool SkeletonChooser::IsHandUp(int id) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(id);
    if (!pSkeleton)
        return false;
    return !AreArmsCrossed(id)
        && (unk2c->IsHandValid(*pSkeleton) || unk30->IsHandValid(*pSkeleton));
}

bool SkeletonChooser::IsAtEdge(int id) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(id);
    MILO_ASSERT(pSkeleton, 0x5BA);
    if (!pSkeleton->IsTracked()) {
        return false;
    } else {
        Vector2 spinePos;
        pSkeleton->ScreenPos(kJointSpine, spinePos);
        if (0.85f < spinePos.x || spinePos.x < 0.15f) {
            return true;
        } else {
            return false;
        }
    }
}

void SkeletonChooser::PollUiNavModeStatus() {
    static Symbol ui_nav_mode("ui_nav_mode");
    static Symbol shell_4p("shell_4p");
    bool inShell = TheHamProvider->Property(ui_nav_mode)->Sym() == shell_4p;
    if (inShell != mInMultiPlayerUpdateMode) {
        if (inShell) {
            EnterMultiPlayerUpdateMode();
        } else {
            ExitMultiPlayerUpdateMode();
        }
    }
}

bool SkeletonChooser::IsPlayerInFreestyle(int player) const {
    HamPlayerData *pPlayer = TheGameData->Player(player);
    MILO_ASSERT(pPlayer, 0x28B);
    PropertyEventProvider *pProvider = pPlayer->Provider();
    MILO_ASSERT(pProvider, 0x28E);
    static Symbol in_freestyle("in_freestyle");
    const DataNode *pNode = pProvider->Property(in_freestyle);
    MILO_ASSERT(pNode, 0x292);
    return pNode->Int();
}

void SkeletonChooser::SwapPlayerSides() {
    static Symbol is_in_party_mode("is_in_party_mode");
    if (!IsFreestyleMode() && TheHamProvider->Property(is_in_party_mode)->Int() != 1) {
        TheGameData->SwapPlayerSides();
        static Message cSwapPlayersMsg("swap_players");
        TheUI->Handle(cSwapPlayersMsg, false);
        static Message post_sides_switched("post_sides_switched");
        TheHamProvider->Export(post_sides_switched, true);
    } else {
        TheGameData->SwapPlayerSidesByIDOnly();
    }
}

void SkeletonChooser::ForceSwapPlayerSides() {
    TheGameData->SwapPlayerSides();
    TheGameData->SwapPlayerSidesByIDOnly();
    static Message cSwapPlayersMsg("swap_players");
    TheUI->Handle(cSwapPlayersMsg, false);
    static Message post_sides_switched("post_sides_switched");
    TheHamProvider->Export(post_sides_switched, true);
}

void SkeletonChooser::SwitchActiveToPlayerIndexImmediate(int playerIndex) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x581);
    unk38 = -1;
    SetActivePlayer(playerIndex);
}

bool SkeletonChooser::ShouldWaitForRecovery() {
    int id0 = TheGameData->Player(0)->GetSkeletonTrackingID();
    int id1 = TheGameData->Player(1)->GetSkeletonTrackingID();
    bool skel0 = true;
    if (id0 > 0) {
        skel0 = TheGestureMgr->GetSkeletonByTrackingID(id0);
    }
    bool skel1 = true;
    if (id1 > 0) {
        skel1 = TheGestureMgr->GetSkeletonByTrackingID(id1);
    }
    if ((!skel0 || !skel1) && TheGestureMgr->Recoverer().WaitingToRecover()) {
        return true;
    } else {
        return false;
    }
}

bool SkeletonChooser::PotentiallyRecoverSkeletons() {
    SkeletonRecoverer &recoverer = TheGestureMgr->Recoverer();
    int id0 = TheGameData->Player(0)->GetSkeletonTrackingID();
    int id1 = TheGameData->Player(1)->GetSkeletonTrackingID();
    bool ret = false;
    if (id0 > 0) {
        int recoveryID = recoverer.GetTrackingIDWithRecovery(id0, id1);
        if (recoveryID > 0 && recoveryID != id0) {
            TheGameData->AssignSkeleton(0, recoveryID);
            id0 = recoveryID;
            ret = true;
        }
    }
    if (id1 > 0) {
        int recoveryID = recoverer.GetTrackingIDWithRecovery(id1, id0);
        if (recoveryID > 0 && recoveryID != id1) {
            TheGameData->AssignSkeleton(1, recoveryID);
            id1 = recoveryID;
            ret = true;
        }
    }
    if (ret) {
        mNextSkelIdxToTrack = -1;
        TheGestureMgr->SetTrackedSkeletons(id0, id1);
    }
    return ret;
}

int SkeletonChooser::NextSkeletonIndexToTrack(int i1) {
    int curSkelIdx = (i1 + 1) % 6;
    int idxToTrack = -1;
    SkeletonRecoverer &recoverer = TheGestureMgr->Recoverer();
    for (int i = 0; i < 6; i++) {
        int skelID = TheGestureMgr->GetSkeleton(curSkelIdx).TrackingID();
        if (idxToTrack >= 0) {
            return idxToTrack;
        }
        if (skelID > 0) {
            idxToTrack = curSkelIdx;
            for (int j = 0; j < 2; j++) {
                int trackingID = TheGameData->Player(j)->GetSkeletonTrackingID();
                int trackingIDRecovery =
                    recoverer.GetTrackingIDWithRecovery(trackingID, -1);
                if (skelID == trackingID || skelID == trackingIDRecovery) {
                    idxToTrack = -1;
                    break;
                }
            }
        }
        if (idxToTrack >= 0) {
            return idxToTrack;
        }
        curSkelIdx = (curSkelIdx + 1) % 6;
    }
    return idxToTrack;
}

void SkeletonChooser::ResolveFreestyle() {
    int id0 = TheGameData->Player(0)->GetSkeletonTrackingID();
    int id1 = TheGameData->Player(1)->GetSkeletonTrackingID();
    if (id0 <= 0 && IsPlayerInFreestyle(0)) {
        id0 = RoundRobinForPlayer(0);
    } else if (id1 <= 0 && IsPlayerInFreestyle(1)) {
        id1 = RoundRobinForPlayer(1);
    }
    TheGestureMgr->SetTrackedSkeletons(id0, id1);
}

void SkeletonChooser::ResolveSinglePlayer() {
    int player = unk3c;
    int otherPlayer = !player;
    int skel = GetAssignedPlayerSkeletonID(player); // goes unused
    int otherSkel = GetAssignedPlayerSkeletonID(otherPlayer);
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol practice("practice");
    if (player != 0) {
        SwapPlayerDataForPractice();
        TheGameData->SwapPlayerSidesByIDOnly();
        player = 0;
        TheHamProvider->SetProperty("ui_nav_player", 0);
        unk3c = 0;
        HamPlayerData *hpd = TheGameData->Player(0);
        otherPlayer = 1;
        TheGestureMgr->SetActiveSkeletonTrackingID(hpd->GetSkeletonTrackingID());
    }
    if (GetPlayerSide(player) != kSkeletonRight) {
        SwapPlayerSides();
    }
    if (otherSkel > 0) {
        SetPlayerSkeletonID(otherPlayer, -1);
    }
    int assignedID = GetAssignedPlayerSkeletonID(player);
    if (assignedID <= 0) {
        assignedID = RoundRobinForPlayer(player);
    }
    if (player == 0) {
        TheGestureMgr->SetTrackedSkeletons(assignedID, -1);
    } else {
        TheGestureMgr->SetTrackedSkeletons(-1, assignedID);
    }
}

void SkeletonChooser::UpdateTrackedSkeletonsElective() {
    if (!ShouldWaitForRecovery()) {
        PollUiNavModeStatus();
        UpdatePlayerSkeletonNavData();
        if (IsSinglePlayerMode()) {
            ResolveSinglePlayer();
        } else if (IsFreestyleMode()) {
            ResolveFreestyle();
        } else if (mInMultiPlayerUpdateMode) {
            if (!mEnrollmentLocked) {
                ResolveMultiPlayerUpdate();
            }
        } else {
            int id0 = TheGameData->Player(0)->GetSkeletonTrackingID();
            int id1 = TheGameData->Player(1)->GetSkeletonTrackingID();
            if (id0 <= 0) {
                id0 = RoundRobinForPlayer(0);
            } else if (id1 <= 0) {
                id1 = RoundRobinForPlayer(1);
            }
            TheGestureMgr->SetTrackedSkeletons(id0, id1);
        }
    }
}

void SkeletonChooser::SetPlayerCloseWarnings(int player, int mask) {
    MILO_ASSERT_RANGE(player, 0, 2, 0xBB);
    static Symbol player_tooclose("player_tooclose");
    static Symbol player_close_top("player_close_top");
    static Symbol player_close_bottom("player_close_bottom");
    static Symbol player_close_left("player_close_left");
    static Symbol player_close_right("player_close_right");
    HamPlayerData *pPlayer = TheGameData->Player(player);
    MILO_ASSERT(pPlayer, 0xC4);
    Hmx::Object *pPlayerProvider = pPlayer->Provider();
    MILO_ASSERT(pPlayerProvider, 0xC6);
    pPlayerProvider->SetProperty(player_tooclose, (mask & 4 && mask & 8));
    pPlayerProvider->SetProperty(player_close_top, (mask & 4) > 0);
    pPlayerProvider->SetProperty(player_close_bottom, (mask & 8) > 0);
    pPlayerProvider->SetProperty(player_close_left, (mask & 2) > 0);
    pPlayerProvider->SetProperty(player_close_right, (mask & 1) > 0);
}

bool SkeletonChooser::IsBehindPlayer(int skelID, int refSkelID) {
    Skeleton *pSkeleton = TheGestureMgr->GetSkeletonByTrackingID(skelID);
    MILO_ASSERT(pSkeleton, 0x56e);
    Skeleton *pRefSkeleton = TheGestureMgr->GetSkeletonByTrackingID(refSkelID);
    MILO_ASSERT(pRefSkeleton, 0x570);
    if (pSkeleton->IsTracked() && pRefSkeleton->IsTracked()) {
        if (pSkeleton->TrackedJoints()[kJointSpine].mJointPos[kCoordCamera].z
            > pRefSkeleton->TrackedJoints()[kJointSpine].mJointPos[kCoordCamera].z
                + 0.3f) {
            return true;
        }
    }
    return false;
}

void SkeletonChooser::SetPlayerSkeletonWarningData(int p1ID, int p2ID) {
    static Symbol player_stepback("player_stepback");
    HamPlayerData *player1 = TheGameData->Player(0);
    HamPlayerData *player2 = TheGameData->Player(1);
    int trackingID = player2->GetSkeletonTrackingID();
    int p1flags = 0;
    int p2flags = 0;
    if (0 < p1ID) {
        Skeleton *pPlayer1Skeleton = TheGestureMgr->GetSkeletonByTrackingID(p1ID);
        MILO_ASSERT(pPlayer1Skeleton, 0x183);
        p1flags = pPlayer1Skeleton->QualityFlags();
    }
    if (0 < p2ID) {
        Skeleton *pPlayer2Skeleton = TheGestureMgr->GetSkeletonByTrackingID(p2ID);
        MILO_ASSERT(pPlayer2Skeleton, 0x18a);
        p2flags = pPlayer2Skeleton->QualityFlags();
    }

    int p2warnings = 0;
    int p1warnings = 0;
    if (0 < p1ID && p1ID == trackingID) {
        p1warnings = p1flags;
        p1warnings = 0;
        p2warnings = p1flags;
    }

    if (0 < p2ID && p2ID == trackingID) {
        p1flags = p2flags;
        p2warnings = p2flags;
        p1flags = p1warnings;
    }
    SetPlayerCloseWarnings(0, p1flags);
    SetPlayerCloseWarnings(1, p2warnings);
}

void SkeletonChooser::SwapPlayerDataForPractice() {
    bool player1PropVal =
        TheGameData->Player(0)->Provider()->Property("using_fitness")->Int();
    bool player2PropVal =
        TheGameData->Player(1)->Provider()->Property("using_fitness")->Int();
    TheGameData->Player(0)->SetUsingFitness(player2PropVal);
    TheGameData->Player(1)->SetUsingFitness(player1PropVal);

    Symbol player1Crew = TheGameData->Player(0)->Provider()->Property("crew")->Sym();
    TheGameData->Player(0)->SetCrew(
        TheGameData->Player(1)->Provider()->Property("crew")->Sym()
    );
    TheGameData->Player(1)->SetCrew(player1Crew);

    player1Crew = TheGameData->Player(0)->Crew();
    TheGameData->Player(0)->SetCrew(TheGameData->Player(1)->Crew());
    TheGameData->Player(1)->SetCrew(player1Crew);

    Symbol player1Char = TheGameData->Player(0)->Char();
    TheGameData->Player(0)->SetCharacter(TheGameData->Player(1)->Char());
    TheGameData->Player(1)->SetCharacter(player1Char);

    Symbol player1Symbol = TheGameData->Player(0)->Unk48();
    Symbol player2Symbol = TheGameData->Player(1)->Unk48();
    TheGameData->Player(0)->SetUnk48(player2Symbol);
    TheGameData->Player(1)->SetUnk48(player1Symbol);

    Symbol player1Outfit = TheGameData->Player(0)->Outfit();
    TheGameData->Player(0)->SetOutfit(TheGameData->Player(1)->Outfit());
    TheGameData->Player(1)->SetOutfit(player1Outfit);

    Symbol player1PreferredOutfit = TheGameData->Player(0)->GetPreferredOutfit();
    TheGameData->Player(0)->SetPreferredOutfit(
        TheGameData->Player(1)->GetPreferredOutfit()
    );
    TheGameData->Player(1)->SetPreferredOutfit(player1PreferredOutfit);

    int player1PadNum = TheGameData->Player(0)->PadNum();
    TheGameData->SetAssociatedPadNum(0, TheGameData->Player(1)->PadNum());
    TheGameData->SetAssociatedPadNum(1, player1PadNum);
}

void SkeletonChooser::UpdatePlayerSkeletonNavData() {
    static Symbol ui_nav_mode("ui_nav_mode");
    const DataNode *pNavPlayerNode = TheHamProvider->Property(ui_nav_mode, true);
    MILO_ASSERT(pNavPlayerNode, 0x328);
    Symbol nodeSym = pNavPlayerNode->Sym();
    static Symbol game("game");
    int trackingID = -1;
    bool notGame = nodeSym != game;
    if (0 <= mNextSkelIdxToTrack) {
        trackingID = TheGestureMgr->GetSkeleton(mNextSkelIdxToTrack).TrackingID();
    }

    int player1ID = TheGestureMgr->GetPlayerFilteredSkeletonID(0, notGame);
    if (player1ID == trackingID) {
        player1ID = -1;
    }
    int player2ID = TheGestureMgr->GetPlayerFilteredSkeletonID(1, notGame);
    if (player2ID == trackingID) {
        player2ID = -1;
    }

    SetPlayerSkeletonNavData(player1ID, player2ID);
    SetPlayerSkeletonWarningData(player1ID, player2ID);
    TheGameData->SetPlayerSidesLocked(false);
    if (TheGameData->GetPlayerSidesLocked() == false) {
        ChoosePlayerSides();
    }

    for (int i = 0; i < 2; i++) {
        int assignedID = GetAssignedPlayerSkeletonID(i);
        mHandRaisedFilters[i]->Update(assignedID, TheTaskMgr.DeltaUISeconds() * 1000.0f);
    }
}

void SkeletonChooser::ResolveMultiPlayerUpdate() {
    MILO_ASSERT(TheGestureMgr->IsTrackingAllSkeletons(), 0x680);

    for (int i = 0; i < 6; i++) {
        unka4[i] = 0;
        TheGestureMgr->SetUnk30AtPos(i, 0);
    }

    for (int i = 0; i < 6; i++) {
        Skeleton &skel = TheGestureMgr->GetSkeleton(i);
        if ((0 <= skel.SkeletonIndex()) && (0 < skel.TrackingID())) {
            unk4c[i]->Update(skel, TheTaskMgr.DeltaUISeconds() * 1000.0f);
            if (unk4c[i]->HandRaised()) {
                unka4[i] = 1;
            }
        }
    }
}

int SkeletonChooser::GetNumValidSkeletonChoices() {
    int numValidSkeletonChoices = 0;
    SkeletonRecoverer &recoverer = TheGestureMgr->Recoverer();
    for (int i = 0; i < 6; i++) {
        Skeleton &skel = TheGestureMgr->GetSkeleton(i);
        int trackingID = skel.TrackingID();
        bool b = false;
        for (int j = 0; j < 2; j++) {
            HamPlayerData *pPlayer = TheGameData->Player(j);
            int playerSkeletonTrackingID = pPlayer->GetSkeletonTrackingID();
            int trackingIDWithRecovery =
                recoverer.GetTrackingIDWithRecovery(playerSkeletonTrackingID, -1);
            if (trackingID == playerSkeletonTrackingID
                || trackingID == trackingIDWithRecovery) {
                b = true;
                break;
            }
        }
        if (!b) {
            numValidSkeletonChoices++;
        }
    }
    return numValidSkeletonChoices;
}

int SkeletonChooser::RoundRobinForStandingStill(int i) {
    int id = -1;
    if (mNextSkelIdxToTrack < 0 || unk80 <= 0.0f) {
        mNextSkelIdxToTrack = NextSkeletonIndexToTrack(mNextSkelIdxToTrack);
        unk80 = 0.08f;
        unk64[0]->Clear();
    }

    if (mNextSkelIdxToTrack >= 0) {
        Skeleton &skel = TheGestureMgr->GetSkeleton(mNextSkelIdxToTrack);
        id = skel.TrackingID();
        unk64[0]->Update(id, TheTaskMgr.DeltaUISeconds() * 1000.0f);
        if (unk64[0]->StandingStill()) {
            mNextSkelIdxToTrack = -1;
        } else {
            if (unk64[0]->RaisedMs() <= 0.0f) {
                unk80 = 0.08f;
            } else {
                unk80 -= TheTaskMgr.DeltaUISeconds();
            }
        }
    }
    return id;
}

int SkeletonChooser::RoundRobinForHandRaised(int i) {
    int id = -1;
    int numValidSkeletonChoices = GetNumValidSkeletonChoices();
    if (numValidSkeletonChoices > unk90) {
        unk84 = 2.0f;
        unk88 = 0.0f;
        unk8c = 0;
    }
    unk90 = numValidSkeletonChoices;
    if (mNextSkelIdxToTrack < 0 || unk80 <= 0.0f) {
        mNextSkelIdxToTrack = NextSkeletonIndexToTrack(mNextSkelIdxToTrack);
        unk80 = 0.08f;
        unk4c[0]->Clear();
    }

    if (mNextSkelIdxToTrack >= 0) {
        Skeleton &skel = TheGestureMgr->GetSkeleton(mNextSkelIdxToTrack);
        id = skel.TrackingID();
        unk4c[0]->Update(id, TheTaskMgr.DeltaUISeconds() * 1000.0f);
        if (unk4c[0]->HandRaised()) {
            static Symbol join_in_progress_complete("join_in_progress_complete");
            static Symbol none("none");
            ThePassiveMessenger->TriggerGenericMsg(
                join_in_progress_complete, none, kPassiveMessageGeneral, gNullStr, -1
            );
            mNextSkelIdxToTrack = -1;
        } else {
            if (unk4c[0]->RaisedMs() <= 0.0f) {
                unk80 = 0.08f;
            } else {
                unk80 -= TheTaskMgr.DeltaUISeconds();
                if (0 <= id && unk8c < 2) {
                    unk84 -= TheTaskMgr.DeltaUISeconds();
                    unk88 -= TheTaskMgr.DeltaUISeconds();
                    if (unk84 <= 0.0f && unk88 <= 0.0f
                        && unk4c[0]->StandingStillFilter().StandingStill()) {
                        unk80 = 0.08f;
                        unk8c++;
                        unk88 = 13.0f;
                        static Symbol doing_stupid_kinect_trick(
                            "doing_stupid_kinect_trick"
                        );
                        if (!IsAutoplaying()) {
                            const DataNode *prop =
                                TheHamProvider->Property(doing_stupid_kinect_trick, true);
                            if (prop->Int() == 0) {
                                static Symbol join_in_progress("join_in_progress");
                                static Symbol none("none");
                                ThePassiveMessenger->TriggerGenericMsg(
                                    join_in_progress,
                                    none,
                                    kPassiveMessageGeneral,
                                    gNullStr,
                                    -1
                                );
                            }
                        }
                    }
                }
            }
        }
    } else {
        unk84 = 2.0f;
        unk88 = 0.0f;
    }
    return id;
}

void SkeletonChooser::CheckToSwitchActivePlayer() {
    int altPlayerID = unk3c == 0;
    int player1ID = GetAssignedPlayerSkeletonID(unk3c);
    int player2ID = GetAssignedPlayerSkeletonID(altPlayerID);
    if (0 <= player2ID) {
        if (player1ID >= 0) {
            Skeleton *skel1 = TheGestureMgr->GetSkeletonByTrackingID(player1ID);
            if (skel1) {
                Skeleton *skel2 = TheGestureMgr->GetSkeletonByTrackingID(player2ID);
                if (skel2 && skel2->IsValid()) {
                    HamPanel *pPanel = dynamic_cast<HamPanel *>(TheHamUI.FocusPanel());
                    if (!pPanel || !pPanel->HasNavList()) {
                        return;
                    }
                    bool p2HandUp = IsHandUp(player2ID);
                    bool p1HandUp = IsHandUp(player1ID);
                    if (!skel2->IsValid() && p2HandUp) {
                        SwitchActiveToPlayerIndexImmediate(altPlayerID);
                        return;
                    }
                    if (!p1HandUp && p2HandUp) {
                        QueueActivePlayerSwitch(altPlayerID);
                        return;
                    }
                }
            }
        } else {
            SwitchActiveToPlayerIndexImmediate(altPlayerID);
            return;
        }
    }
    unk38 = -1;
}
