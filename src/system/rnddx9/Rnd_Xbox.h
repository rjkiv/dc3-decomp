#pragma once

#include <types.h>

class DxRnd {
    u8 pad[0x224];
    struct D3DDevice *mD3DDevice;

public:
    DxRnd(void);

    struct D3DDevice *D3DDevice(void) { return mD3DDevice; }
};

extern DxRnd TheDxRnd;
