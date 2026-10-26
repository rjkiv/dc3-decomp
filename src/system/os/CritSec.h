#pragma once
#include "utl/MemMgr.h"
#include "xdk/XBOXKRNL.h"

// size 0x20
class CriticalSection {
private:
    int mEntryCount; // 0x0
    RTL_CRITICAL_SECTION mCritSec; // 0x4

public:
    CriticalSection();
    ~CriticalSection();
    void Enter();
    void Exit();
    bool TryEnter();
    void Abandon();

    MEM_OVERLOAD(CriticalSection, 0x20);
};

class CritSecTracker {
public:
    CritSecTracker(CriticalSection *section) {
        mCritSec = section;
        if (section != 0) {
            section->Enter();
        }
    }

    ~CritSecTracker() {
        if (mCritSec != 0) {
            mCritSec->Exit();
        }
    }

private:
    CriticalSection *mCritSec;
};
