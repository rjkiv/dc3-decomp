#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _NUI_ENROLLMENT_INFORMATION { /* Size=0x8 */
    /* 0x0000 */ DWORD dwUserIndex;
    /* 0x0004 */ DWORD dwEnrollmentFlags;
} NUI_ENROLLMENT_INFORMATION;

typedef enum _NUI_IDENTITY_MESSAGE_ID {
    NUI_IDENTITY_MESSAGE_ID_FRAME_PROCESSED = 0x0000,
    NUI_IDENTITY_MESSAGE_ID_COMPLETE = 0x0001,
} NUI_IDENTITY_MESSAGE_ID;

typedef enum _NUI_IDENTITY_OPERATION_ID {
    NUI_IDENTITY_OPERATION_ID_NONE = 0x0000,
    NUI_IDENTITY_OPERATION_ID_IDENTIFY = 0x0001,
    NUI_IDENTITY_OPERATION_ID_ENROLL = 0x0002,
    NUI_IDENTITY_OPERATION_ID_TUNER = 0x0003,
} NUI_IDENTITY_OPERATION_ID;

typedef struct _NUI_IDENTITY_MESSAGE_FRAME_PROCESSED { /* Size=0x4 */
    /* 0x0000 */ DWORD dwQualityFlags;
} NUI_IDENTITY_MESSAGE_FRAME_PROCESSED;

typedef struct _NUI_IDENTITY_MESSAGE_COMPLETE { /* Size=0xc */
    /* 0x0000 */ HRESULT hrResult;
    /* 0x0004 */ DWORD dwEnrollmentIndex;
    /* 0x0008 */ BOOL bProfileMatched;
} NUI_IDENTITY_MESSAGE_COMPLETE;

typedef struct _NUI_IDENTITY_MESSAGE { /* Size=0x1c */
    /* 0x0000 */ NUI_IDENTITY_MESSAGE_ID MessageId;
    /* 0x0004 */ NUI_IDENTITY_OPERATION_ID OperationId;
    /* 0x0008 */ DWORD dwTrackingID;
    /* 0x000c */ DWORD dwSkeletonFrameNumber;
    /* 0x0010 */
    union {
        NUI_IDENTITY_MESSAGE_FRAME_PROCESSED FrameProcessed;
        NUI_IDENTITY_MESSAGE_COMPLETE Complete;
    } Data;
} NUI_IDENTITY_MESSAGE;

typedef BOOL NUI_IDENTITY_CALLBACK(VOID *, NUI_IDENTITY_MESSAGE *);

HRESULT NuiIdentityAbort();
HRESULT NuiIdentityGetEnrollmentInformation(
    DWORD dwEnrollmentIndex, NUI_ENROLLMENT_INFORMATION *pEnrollmentInformation
);
HRESULT NuiIdentityIdentify(
    DWORD dwTrackingID,
    DWORD dwFlags,
    NUI_IDENTITY_CALLBACK *pfnIdentityCallback,
    VOID *pvContext
);

#ifdef __cplusplus
}
#endif
