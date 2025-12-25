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
    int Unk3C() { return unk3c; }

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

protected:
    DirectionGestureFilterSingleUser *unk2c;
    DirectionGestureFilterSingleUser *unk30;
    bool unk34;
    int unk38;
    int unk3c;
    float unk40;
    float unk44;
    bool unk48;
    HandRaisedGestureFilter *unk4c[6];
    StandingStillGestureFilter *unk64[6];
    HighFiveGestureFilter *unk7c;
    float unk80;
    float unk84;
    float unk88;
    int unk8c;
    int unk90;
    int unk94;
    HandRaisedGestureFilter *unk98[2];
    bool unka0; // 0xa0 - multiplayer update mode
    int unka4[6];
    int unkbc;
    bool unkc0;
};
