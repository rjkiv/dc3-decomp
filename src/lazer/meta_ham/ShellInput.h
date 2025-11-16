#pragma once

#include "types.h"

class ShellInput {
public:
    ShellInput();
    virtual ~ShellInput();

    void Init();

    int CycleDrawCursor();
    u8 pad[0xE4];
};
