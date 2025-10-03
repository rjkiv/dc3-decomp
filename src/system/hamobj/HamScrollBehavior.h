#pragma once
#include "math/DoubleExponentialSmoother.h"
#include "ui/UIListState.h"

class HamNavList;

class HamScrollBehavior {
public:
    HamScrollBehavior(HamNavList *, UIListState *);
    bool ScrollDown(bool);
    bool AtTop() const;
    bool AtBottom() const;
    void Enter();
    void Reset();
    void Exit();
    void Update(float);

    static void Init();
    static float mNeutralToSlowDownDelay;
    static float mSlowDownFirstTickDelay;
    static float mSlowDownTickDelay;
    static float mFastDownTickDelay;
    static float mNeutralToSlowUpDelay;
    static float mSlowUpFirstTickDelay;
    static float mSlowUpTickDelay;
    static float mFastUpTickDelay;
    static float mSlowScrollSpeed;
    static float mNormalScrollSpeed;
    static float mFastScrollSpeedBase;
    static float mFastScrollSpeedScalar;
    static float mScrollUpCap;
    static float mScrollDownCap;
    static float mSlowFastThreshold;

private:
    static float sScrollSettleTime;

    float unk0;
    bool unk4;
    bool unk5;
    int unk8;
    float unkc;
    float unk10;
    float unk14;
    float unk18;
    bool unk1c;
    bool unk1d;
    float unk20;
    int unk24;
    int unk28;
    int unk2c;
    int unk30;
    DoubleExponentialSmoother unk34;
    int unk48;
    UIListState *unk4c;
    HamNavList *unk50;
};
