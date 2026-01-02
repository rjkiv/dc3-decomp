#pragma once
#include "../win_types.h"
#include "xdk/XAPILIB.h"
#include "vectorintrinsics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int NUI_TILT_FLAGS;

// this struct seems to be 0x10-aligned
// but i can't get the pragmas for it to work
// #pragma pack(16)
struct _NUI_TILT_OBJECTS { /* Size=0xd0 */
    /* 0x0000 */ float CameraHeightMeters;
    /* 0x0004 */ DWORD Count;
    int filler[2];
    /* 0x0010 */ struct { /* Size=0x20 */
        /* 0x0000 */ __vector4 Location;
        /* 0x0010 */ NUI_TILT_FLAGS Flags;
        int filler[3];
    } Objects[6];
};
// #pragma pack(pop)

typedef _NUI_TILT_OBJECTS NUI_TILT_OBJECTS;

DWORD NuiCameraAdjustTilt(
    NUI_TILT_FLAGS, float, float, float, NUI_TILT_OBJECTS *, XOVERLAPPED *
);
HRESULT NuiCameraElevationSetAngle(long);

#ifdef __cplusplus
}
#endif
