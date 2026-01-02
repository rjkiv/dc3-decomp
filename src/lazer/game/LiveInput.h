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
    float unk8; // 0x8
    Timer mTimer; // 0x10
    int unk44; // 0x44
};
