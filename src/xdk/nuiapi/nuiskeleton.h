#pragma once
#include "../win_types.h"
#include "vectorintrinsics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum _NUI_SKELETON_POSITION_TRACKING_STATE {
    NUI_SKELETON_POSITION_NOT_TRACKED = 0x0000,
    NUI_SKELETON_POSITION_INFERRED = 0x0001,
    NUI_SKELETON_POSITION_TRACKED = 0x0002,
};
typedef _NUI_SKELETON_POSITION_TRACKING_STATE NUI_SKELETON_POSITION_TRACKING_STATE;

enum _NUI_SKELETON_TRACKING_STATE {
    NUI_SKELETON_NOT_TRACKED = 0x0000,
    NUI_SKELETON_POSITION_ONLY = 0x0001,
    NUI_SKELETON_TRACKED = 0x0002,
};
typedef _NUI_SKELETON_TRACKING_STATE NUI_SKELETON_TRACKING_STATE;

struct _NUI_SKELETON_DATA { /* Size=0x1c0 */
    /* 0x0000 */ _NUI_SKELETON_TRACKING_STATE eTrackingState;
    /* 0x0004 */ DWORD dwTrackingID;
    /* 0x0008 */ DWORD dwEnrollmentIndex;
    /* 0x000c */ DWORD dwUserIndex;
    /* 0x0010 */ XMVECTOR Position;
    /* 0x0020 */ XMVECTOR SkeletonPositions[20];
    /* 0x0160 */ _NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[20];
    /* 0x01b0 */ DWORD dwQualityFlags;
};
typedef _NUI_SKELETON_DATA NUI_SKELETON_DATA;

struct _NUI_SKELETON_FRAME { /* Size=0xab0 */
    /* 0x0000 */ u64 liTimeStamp;
    /* 0x0008 */ DWORD dwFrameNumber;
    /* 0x000c */ DWORD dwFlags;
    /* 0x0010 */ XMVECTOR vFloorClipPlane;
    /* 0x0020 */ XMVECTOR vNormalToGravity;
    /* 0x0030 */ _NUI_SKELETON_DATA SkeletonData[6];
};
typedef _NUI_SKELETON_FRAME NUI_SKELETON_FRAME;

#ifdef __cplusplus
}
#endif

// has a C++ definition
XMMATRIX NuiTransformMatrixLevel(XMVECTOR);
