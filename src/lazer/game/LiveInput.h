#pragma once
#include "game/GameInput.h"
#include "hamobj/HamAudio.h"

class LiveInput : public GameInput {
public:
    LiveInput(HamAudio &);

    virtual float CurrentMs(bool) const;
    virtual float GetSongToTaskMgrMs() const;
    virtual void SetPaused(bool);
    virtual void SetTimeOffset();
    virtual void SetPostWaitJumpOffset(float);
    virtual ~LiveInput() {}

protected:
    HamAudio &mAudio; // 0x4
    float mTimeOffset; // 0x8
    Timer mRealTimer; // 0x10
    int mJumpWaitMs; // 0x44
};
