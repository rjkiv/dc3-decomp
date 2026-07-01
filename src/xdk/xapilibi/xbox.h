#pragma once
#include "../win_types.h"
#include "minwinbase.h"
#include "wtypesbase.h"
#include "xinput.h"
#include "xbase.h"

// This is where you declare any Xbox 360 specific functions you may need.
// They're in no particular order, maybe we can reorganize them later, idk.
// If you need to add a new struct/enum, put it in xbase.h.

#ifdef __cplusplus
extern "C" {
#endif

DWORD XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE Mode);
DWORD XEnableScreenSaver(BOOL fEnable);
DWORD XGetLocale();
DWORD XTLGetLanguage();
__declspec(noreturn) VOID XLaunchNewImage(LPCSTR szImagePath, DWORD dwFlags);
LPVOID XPhysicalAlloc(
    SIZE_T dwSize, ULONG_PTR ulPhysicalAddress, ULONG_PTR ulAlignment, DWORD flProtect
);
VOID XPhysicalFree(LPVOID lpAddress);
DWORD XShowFriendsUI(DWORD dwUserIndex);
DWORD XShowPartyUI(DWORD dwUserIndex);
DWORD XShowNuiFriendsUI(DWORD dwTrackingID, DWORD dwUserIndex);
DWORD XShowNuiPartyUI(DWORD dwTrackingID, DWORD dwUserIndex);
DWORD XShowNuiGuideUI(DWORD dwTrackingID);
DWORD XShowNuiSigninUI(DWORD dwTrackingID, DWORD dwFlags);
HRESULT XNuiDelayUI(ULONG ulMilliSeconds);
DWORD XShowNuiGamerCardUI(DWORD dwTrackingID, DWORD dwUserIndex, XUID XuidPlayer);
DWORD XShowNuiControllerRequiredUI(DWORD dwTrackingID, DWORD dwUserIndex);
DWORD XUserCheckPrivilege(
    DWORD dwUserIndex, XPRIVILEGE_TYPE PrivilegeType, BOOL *pfResult
);
XUSER_SIGNIN_STATE XUserGetSigninState(DWORD dwUserIndex);
DWORD XUserGetXUID(DWORD dwUserIndex, XUID *pxuid);
VOID XUserSetContext(DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue);
DWORD XUserWriteAchievements(
    DWORD dwNumAchievements, XUSER_ACHIEVEMENT *pAchievements, XOVERLAPPED *pOverlapped
);
DWORD XContentClose(LPCSTR szRootName, XOVERLAPPED *pOverlapped);
DWORD XContentGetDeviceData(DWORD DeviceID, XDEVICE_DATA *pDeviceData);
DWORD XCancelOverlapped(XOVERLAPPED *pOverlapped);
DWORD XGetOverlappedExtendedError(XOVERLAPPED *pOverlapped);
DWORD XShowKeyboardUI(
    DWORD dwUserIndex,
    DWORD dwFlags,
    LPCWSTR wseDefaultText,
    LPCWSTR wszTitleText,
    LPCWSTR wszDescriptionText,
    LPWSTR wszResultText,
    DWORD cchResultText,
    XOVERLAPPED *pOverlapped
);
DWORD XContentCrossTitleCreate(
    DWORD dwUserIndex,
    LPCSTR szRootName,
    XCONTENT_CROSS_TITLE_DATA *pContentData,
    DWORD dwContentFlags,
    DWORD *pdwDisposition,
    DWORD *pdwLicenseMask,
    SIZE_T dwFileCacheSize,
    ULARGE_INTEGER uliContentSize,
    XOVERLAPPED *pOverlapped
);
DWORD XContentCrossTitleDelete(
    DWORD dwUserIndex,
    CONST XCONTENT_CROSS_TITLE_DATA *pContentData,
    XOVERLAPPED *pOverlapped
);
DWORD XGetOverlappedExtendedError(XOVERLAPPED *pOverlapped);
DWORD XGetOverlappedResult(XOVERLAPPED *pOverlapped, DWORD *pdwResult, BOOL bWait);
DWORD XUserGetSigninInfo(DWORD dwUserIndex, DWORD dwFlags, XUSER_SIGNIN_INFO *pSigninInfo);
DWORD XSetThreadProcessor(HANDLE hThread, DWORD dwHardwareThread);
DWORD XContentCreateEx(
    DWORD dwUserIndex,
    LPCSTR szRootName,
    CONST XCONTENT_DATA *pContentData,
    DWORD dwContentFlags,
    DWORD *pdwDisposition,
    DWORD *pdwLicenseMask,
    SIZE_T dwFileCacheSize,
    ULARGE_INTEGER uliContentSize,
    XOVERLAPPED *pOverlapped
);
DWORD XContentGetCreator(
    DWORD dwUserIndex,
    CONST XCONTENT_DATA *pContentData,
    BOOL *pfUserIsCreator,
    XUID *pxuid,
    XOVERLAPPED *pOverlapped
);
DWORD XContentGetDeviceState(DWORD DeviceID, XOVERLAPPED *pOverlapped);
DWORD XContentDelete(
    DWORD dwUserIndex, CONST XCONTENT_DATA *pContentData, XOVERLAPPED *pOverlapped
);
DWORD XContentCreateEnumerator(
    DWORD dwUserIndex,
    DWORD DeviceID,
    DWORD dwContentType,
    DWORD dwContentFlags,
    DWORD cItem,
    DWORD *pcbBuffer,
    HANDLE *phEnum
);

DWORD XContentCreateCrossTitleEnumerator(
    DWORD dwUserIndex,
    DWORD DeviceID,
    DWORD dwContentType,
    DWORD dwContentFlags,
    DWORD cItem,
    DWORD *pcbBuffer,
    HANDLE *phEnum
);

DWORD XEnumerate(
    HANDLE hEnum,
    VOID *pvBuffer,
    DWORD cbBuffer,
    DWORD *pcItemsReturned,
    XOVERLAPPED *pOverlapped
);

DWORD XEnumerateCrossTitle(
    HANDLE hEnum,
    VOID *pvBuffer,
    DWORD cbBuffer,
    DWORD *pcItemsReturned,
    XOVERLAPPED *pOverlapped
);

DWORD XContentFlush(LPCSTR szRootName, XOVERLAPPED *pOverlapped);
ULONGLONG XContentCalculateSize(ULONGLONG cbData, DWORD cDirectories);
VOID XGetVideoMode(XVIDEO_MODE *pVideoMode);
VOID *XMemCpy(VOID *__restrict dest, const VOID *__restrict src, SIZE_T count);
VOID *XMemCpyStreaming_WriteCombined(
    VOID *__restrict dest, const VOID *__restrict src, SIZE_T count
);
VOID *XMemSet(VOID *dest, INT c, SIZE_T count);
VOID *XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes);
VOID *XMemAllocDefault(SIZE_T dwSize, DWORD dwAllocAttributes);
VOID XMemFree(LPVOID lpHandle, DWORD dwFreeAttributes);
VOID XMemFreeDefault(LPVOID lpHandle, DWORD dwFreeAttributes);
INT XMemSize(LPVOID lpHandle, DWORD dwSizeAttributes);
INT XMemSizeDefault(LPVOID lpHandle, DWORD dwSizeAttributes);
DWORD XPhysicalSize(LPVOID lpAddress);
DWORD XUserAwardGamerPicture(
    DWORD dwUserIndex, DWORD dwPictureId, DWORD dwReserved, XOVERLAPPED *pXOverlapped
);
DWORD XUserAwardAvatarAssets(
    DWORD dwNumAssets, CONST XUSER_AVATARASSET *pAssets, XOVERLAPPED *pOverlapped
);
DWORD XUserGetName(DWORD dwUserIndex, LPSTR szUserName, DWORD cchUserName);
DWORD XShowTokenRedemptionUI(DWORD dwUserIndex);
DWORD XUserAreUsersFriends(
    DWORD dxUserIndex,
    XUID *pXuids,
    DWORD dwXuidCount,
    BOOL *pfResult,
    XOVERLAPPED *pOverlapped
);
DWORD XShowNuiMarketplaceUI(
    DWORD dwTrackingID,
    DWORD dwUserIndex,
    DWORD dwEntryPoint,
    QWORD qwOfferID,
    DWORD dwContentCategories
);
DWORD XShowMarketplaceUI(
    DWORD dwUserIndex, DWORD dwEntryPoint, QWORD qwOfferID, DWORD dwContentCategories
);
DWORD XShowMarketplaceDownloadItemsUI(
    DWORD dwUserIndex,
    DWORD dwEntryPoint,
    const XUID *pOfferIDs,
    DWORD dwOfferIdCount,
    HRESULT *phrResult,
    _XOVERLAPPED *pOverlapped
);
DWORD XShowNuiDeviceSelectorUI(
    DWORD dwTrackingID,
    DWORD dwUserIndex,
    DWORD dwContentType,
    DWORD dwContentFlags,
    ULARGE_INTEGER uliBytesRequested,
    DWORD *pDeviceID,
    XOVERLAPPED *pOverlapped
);
DWORD XShowDeviceSelectorUI(
    DWORD dwUserIndex,
    DWORD dwContentType,
    DWORD dwContentFlags,
    ULARGE_INTEGER uliBytesRequested,
    DWORD *pDeviceID,
    XOVERLAPPED *pOverlapped
);
DWORD XGetGameRegion();
DWORD XShowSigninUI(DWORD cPanes, DWORD dwFlags);
__declspec(noreturn) DWORD XShowNuiDirtyDiscErrorUI(DWORD dwTrackingID, DWORD dwUserIndex);
__declspec(noreturn) DWORD XShowDirtyDiscErrorUI(DWORD dwUserIndex);

HANDLE XNotifyCreateListener(QWORD qwAreas);
void XNotifyPositionUI(DWORD dwPosition);
BOOL XNotifyGetNext(HANDLE hNotification, DWORD dwMsgFilter, DWORD *pdwId, ULONG *pParam);
DWORD XUserSetPropertyEx(
    DWORD dwUserIndex,
    DWORD dwPropertyId,
    DWORD cbValue,
    const void *pvValue,
    XOVERLAPPED *pOverlapped
);
DWORD XShowGamerCardUI(DWORD dwUserIndex, XUID XuidPlayer);

#ifdef __cplusplus
}
#endif
