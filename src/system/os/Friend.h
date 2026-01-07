#pragma once
#include "utl/MemMgr.h"
#include "utl/Str.h"

class Friend {
public:
    Friend();
    void SetName(String name) { mName = name; }

    MEM_OVERLOAD(Friend, 0x1b)

    String mName; // 0x0
    String unkc;
};
