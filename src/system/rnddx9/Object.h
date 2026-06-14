#pragma once
#include "xdk/win_types.h"

class DxObject {
public:
    virtual void PreDeviceReset() {}
    virtual void PostDeviceReset() {}
};

struct DxLineVertex {
    float x, y, z; // D3DFVF_XYZ
    DWORD diffuse; // D3DFVF_DIFFUSE
};
