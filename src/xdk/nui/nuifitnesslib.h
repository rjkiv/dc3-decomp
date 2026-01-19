#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _NUI_FITNESS_TRACKING_MODE {
    NUI_FITNESS_TRACKING_AUTO = 0,
    NUI_FITNESS_TRACKING_MANUAL = 1,
} NUI_FITNESS_TRACKING_MODE;

typedef struct _NUI_FITNESS_DATA { /* Size=0xc */
    /* 0x0000 */ DWORD DurationInMS;
    /* 0x0004 */ FLOAT AverageMET;
    /* 0x0008 */ DWORD Joules;
} NUI_FITNESS_DATA;

HRESULT XShowNuiFitnessBodyProfileUI(DWORD TrackingID, DWORD UserIndex);
HRESULT XShowFitnessBodyProfileUI(DWORD UserIndex);
HRESULT NuiFitnessStartTracking(
    NUI_FITNESS_TRACKING_MODE TrackingMode, DWORD TrackingID, DWORD UserIndex
);
HRESULT NuiFitnessPauseTracking(DWORD TrackingID);
HRESULT NuiFitnessResumeTracking(DWORD OldTrackingID, DWORD NewTrackingID);
HRESULT NuiFitnessStopTracking(DWORD TrackingID);
HRESULT NuiFitnessGetCurrentFitnessData(DWORD TrackingID, NUI_FITNESS_DATA *pFitnessData);

#ifdef __cplusplus
}
#endif
