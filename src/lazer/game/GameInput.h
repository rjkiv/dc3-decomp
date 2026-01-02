#pragma once
#include "utl/MemMgr.h"

class GameInput {
public:
    virtual float CurrentMs(bool) const = 0;
    virtual float GetSongToTaskMgrMs() const = 0;
    virtual void SetPaused(bool) = 0;
    virtual void SetTimeOffset() = 0;
    virtual void SetPostWaitJumpOffset(float) = 0;

    MEM_OVERLOAD(GameInput, 0x11);
};
