
#pragma once
#include "xdk/win_types.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/xbase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XSOCIAL_PREVIEWIMAGE { /* Size=0x14 */
    BYTE *pBytes;
    DWORD Pitch;
    DWORD Width;
    DWORD Height;
    D3DFORMAT Format;
} XSOCIAL_PREVIEWIMAGE;

typedef struct _XSOCIAL_LINKPOSTPARAMS { /* Size=0x30 */
    DWORD Size;
    LPCWSTR TitleText;
    LPCWSTR TitleURL;
    LPCWSTR PictureCaption;
    LPCWSTR PictureDescription;
    LPCWSTR PictureURL;
    XSOCIAL_PREVIEWIMAGE PreviewImage;
    DWORD Flags;
} XSOCIAL_LINKPOSTPARAMS;

enum XSOCIAL_POST {
    XSOCIAL_POST_USERGENERATEDCONTENT = 1,
    XSOCIAL_POST_KINECTCONTENT = 2,
    XSOCIAL_POST_GAMECONTENT = 4,
    XSOCIAL_POST_ACHIEVEMENTCONTENT = 8,
    XSOCIAL_POST_MEDIACONTENT = 0x10
};

typedef struct _XSOCIAL_IMAGEPOSTPARAMS { /* Size=0x30 */
    DWORD Size;
    LPCWSTR TitleText;
    LPCWSTR PictureCaption;
    LPCWSTR PictureDescription;
    XSOCIAL_PREVIEWIMAGE PreviewImage;
    const BYTE *pFullImage;
    DWORD FullImageByteCount;
    DWORD Flags;
} XSOCIAL_IMAGEPOSTPARAMS;

DWORD XSocialGetCapabilities(DWORD *pCapabilities, XOVERLAPPED *pOverlapped);

DWORD XShowSocialNetworkLinkPostUI(
    DWORD UserIndex, const XSOCIAL_LINKPOSTPARAMS *pParams, XOVERLAPPED *pOverlapped
);

DWORD XShowNuiSocialNetworkImagePostUI(
    DWORD TrackingID,
    DWORD UserIndex,
    const XSOCIAL_IMAGEPOSTPARAMS *pParams,
    XOVERLAPPED *pOverlapped
);

DWORD XShowNuiSocialNetworkLinkPostUI(
    DWORD TrackingID,
    DWORD UserIndex,
    const XSOCIAL_LINKPOSTPARAMS *pParams,
    XOVERLAPPED *pOverlapped
);

DWORD XShowSocialNetworkImagePostUI(
    DWORD UserIndex, const XSOCIAL_IMAGEPOSTPARAMS *pParams, XOVERLAPPED *pOverlapped
);

#ifdef __cplusplus
}
#endif
