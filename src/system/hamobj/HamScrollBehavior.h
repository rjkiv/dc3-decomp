#pragma once
#include "math/DoubleExponentialSmoother.h"
#include "ui/UIListState.h"

enum ListScrollDirection {
    kScrollDir_None = 0,
    kScrollDir_Up = 1,
    kScrollDir_Down = 2,
};

class HamNavList;

class HamScrollBehavior {
public:
    enum ScrollState {
        kScrollState_FastUp = 0,
        kScrollState_Up = 1,
        kScrollState_None = 2,
        kScrollState_Down = 3,
        kScrollState_FastDown = 4,
    };
    HamScrollBehavior(HamNavList *, UIListState *);
    bool ScrollUp(bool);
    bool ScrollDown(bool);
    bool IsScrolling() const;
    bool AtTop() const;
    bool AtBottom() const;
    void Enter();
    void Reset();
    void Exit();
    void Update(float);
    void PlayScrollSound();

    float GetFirstVal() const {
        return mScrollingSettledTimer;
    } // change once context found
    ListScrollDirection GetHandScrollPos() const { return mHandScrollPos; }
    void SetHandScrollPos(ListScrollDirection dir) { mHandScrollPos = dir; }
    float GetScrollFrame() const { return mScrollFrame; }

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

    float mScrollingSettledTimer; // 0x0
    bool mAtTop; // 0x4
    bool mAtBottom; // 0x5
    int mScrollAmount; // 0x8
    float mScrollTimer; // 0xc
    float mScrollDelayBefore; // 0x10
    float mScrollDelayBetween; // 0x14
    float mScrollProgress; // 0x18
    bool mOnScrollButton; // 0x1c
    bool mJustOnScrollButton; // 0x1d
    float mScrollFrame; // 0x20
    ListScrollDirection mScrollDir; // 0x24
    ListScrollDirection mBufferedScrollDir; // 0x28
    ListScrollDirection mLastScrollDir; // 0x2c
    ListScrollDirection mHandScrollPos; // 0x30
    DoubleExponentialSmoother mScrollSpeedFrame; // 0x34
    ScrollState mScrollState; // 0x48
    UIListState *mListState; // 0x4c - owner liststate?
    HamNavList *mOwner; // 0x50 - owner navlist?
};
