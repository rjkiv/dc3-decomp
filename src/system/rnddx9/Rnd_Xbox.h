#pragma once

#include "os/Debug.h"
#include <types.h>

class DxRnd {
public:
    u8 pad[0x224];
    struct D3DDevice *mD3DDevice;
    u8 pad2[0xD9];
    u8 unk_0x301;

    DxRnd(void);

    struct D3DDevice *D3DDevice(void) { return mD3DDevice; }

    static const char *Error(s32);
};

#define MILO_PRINT_D3DERR(err_result, line)                                              \
    TheDebug.Fail(                                                                       \
        MakeString(                                                                      \
            "File: %s Line: %d Error: %s\n", __FILE__, line, DxRnd::Error(err_result)    \
        ),                                                                               \
        0                                                                                \
    )

extern DxRnd TheDxRnd;
