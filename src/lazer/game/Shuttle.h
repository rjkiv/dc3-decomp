#pragma once
#include "utl/MemMgr.h"

class Shuttle {
public:
    Shuttle();
    void SetActive(bool);
    void Poll();

    MEM_OVERLOAD(Shuttle, 0x12);

    float unk0;
    float unk4;
    bool unk8;
    int unkc;
};
