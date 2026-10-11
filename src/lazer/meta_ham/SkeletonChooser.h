#pragma once
#include "gesture/DirectionGestureFilter.h"
#include "gesture/HandRaisedGestureFilter.h"
#include "gesture/HighFiveGestureFilter.h"
#include "gesture/StandingStillGestureFilter.h"
#include "obj/Object.h"

// size 0xc4
class SkeletonChooser : public Hmx::Object {
public:
    SkeletonChooser();
    virtual ~SkeletonChooser();
    virtual DataNode Handle(DataArray *, bool);

    bool IsHandRaised(int);
    bool IsSkeletonValid(int);
    bool IsPlayerHandRaised(int);
    bool IsLeftPlayerHandRaised();
    bool IsRightPlayerHandRaised();
    int GetAssignedPlayerSkeletonID(int);
    bool AreArmsCrossed(int);
    void DrawDebug();
    void GetJointDepthPos(int, int, Vector3 &);
    void ClearPlayerSkeletonID(int);
    SkeletonSide GetPlayerSide(int);
    void SetActivePlayer(int);
    void Poll();

    DataNode OnGetJointDepthPos(const DataArray *);
    int Unk3C() { return mActivePlayerIndex; }

private:
    void SwitchActiveToPlayerIndexImmediate(int);
    void ForceSwapPlayerSides();
    void SetPlayerSkeletonID(int, int);
    bool PotentiallyRecoverSkeletons();
    void CheckToSwitchActivePlayer();
    void UpdateTrackedSkeletonsElective();
    bool GetPlayerPresent(int);
    void SetPlayerPresent(int, bool);
    void EnterMultiPlayerUpdateMode();
    void ExitMultiPlayerUpdateMode();
    bool ShouldWaitForRecovery();
    bool IsFreestyleMode() const;
    bool IsSinglePlayerMode() const;
    int NextSkeletonIndexToTrack(int);
    int GetNumValidSkeletonChoices();
    bool IsAutoplaying() const;
    bool DoesRequireHandRaise() const;
    int RoundRobinForHandRaised(int);
    int RoundRobinForStandingStill(int);
    int RoundRobinForPlayer(int);
    bool IsCentered(int);
    void QueueActivePlayerSwitch(int);
    bool IsHandUp(int);
    bool IsAtEdge(int);
    void PollUiNavModeStatus();
    bool IsPlayerInFreestyle(int) const;
    void SwapPlayerSides();
    void ResolveFreestyle();
    void ResolveSinglePlayer();
    void SwapPlayerDataForPractice();
    void UpdatePlayerSkeletonNavData();
    void ResolveMultiPlayerUpdate();
    void SetPlayerCloseWarnings(int, int);
    bool IsBehindPlayer(int, int);
    void SetPlayerSkeletonNavData(int, int);
    void SetPlayerSkeletonWarningData(int, int);
    void ChoosePlayerSides();

    bool StandingStillRaisedEnough() {
        return mStandingStillFilter[0]->RaisedMs() > 0.0f;
    }
    bool HandRaisedRaisedEnough() { return mHandRaiseFilter[0]->RaisedMs() > 0.0f; }

private:
    DirectionGestureFilterSingleUser *mRightHandUp; // 0x2c
    DirectionGestureFilterSingleUser *mLeftHandUp; // 0x30
    bool mDrawDebug; // 0x34
    int mQueuedActivePlayerIndex; // 0x38
    int mActivePlayerIndex; // 0x3c - current player index?
    float mSwitchTimer; // 0x40
    float mSwitchDelay; // 0x44
    bool mSkeletonUpdateAutomatic; // 0x48
    HandRaisedGestureFilter *mHandRaiseFilter[6]; // 0x4c
    StandingStillGestureFilter *mStandingStillFilter[6]; // 0x64
    HighFiveGestureFilter *mHighFiveGestureFilter; // 0x7c
    float mElectiveCheckTimer; // 0x80
    float mHandRaiseMsgTimer; // 0x84
    float mHandRaiseMsgSpamTimer; // 0x88
    int mHandRaiseMsgCount; // 0x8c
    int mLastValidSkeletonCount; // 0x90
    int mLookingAtIndex; // 0x94
    HandRaisedGestureFilter *mPlayerHandRaiseFilter[2]; // 0x98
    bool mInMultiPlayerUpdateMode; // 0xa0
    int mSkeletonHandRaised[6]; // 0xa4
    int mEnrollmentLookingAtIndex; // 0xbc
    bool mEnrollmentLocked; // 0xc0
};
