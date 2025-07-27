#pragma once

#include "types.h"

class Licenses {
public:
    enum Requirement {
    };
    CDECL Licenses(const char *, Requirement);

    static void CDECL PrintAll(void);

protected:
    static Licenses *sFront;
    static uint sInited;

private:
    Requirement unk_0x0;
    const char *unk_0x4;
    Licenses *mNext; // 0x8
};
