#pragma once
#include "utl/MemMgr.h"

class Shuttle {
public:
    Shuttle();
    ~Shuttle();
    void SetActive(bool);
    void Poll();
    bool IsActive() const { return mActive; }
    float Ms() const { return mMs; }
    void SetController(int cnt) { mController = cnt; }
    void SetMs(float ms) { mMs = ms; }
    void SetEndMs(float ms) { mEndMs = ms; }

    MEM_OVERLOAD(Shuttle, 0x12);

private:
    float mMs; // 0x0
    float mEndMs; // 0x4
    bool mActive; // 0x8
    int mController; // 0xc
};
