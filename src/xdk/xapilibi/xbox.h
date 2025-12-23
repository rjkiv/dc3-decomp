#pragma once
#include "../win_types.h"
#include "minwinbase.h"
#include "wtypesbase.h"
#include "winsockx.h"
#include "xinput.h"
#include "xbase.h"

// This is where you declare any Xbox 360 specific functions you may need.
// They're in no particular order, maybe we can reorganize them later, idk.
// If you need to add a new struct/enum, put it in xbase.h.

#ifdef __cplusplus
extern "C" {
#endif

DWORD XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE);

DWORD XEnableScreenSaver(BOOL);

DWORD XGetLocale();

DWORD XTLGetLanguage();

VOID XLaunchNewImage(LPCSTR, DWORD);

LPVOID XPhysicalAlloc(SIZE_T, ULONG_PTR, ULONG_PTR, DWORD);
VOID XPhysicalFree(LPVOID);

DWORD XShowFriendsUI(DWORD);
DWORD XShowPartyUI(DWORD);

DWORD XShowNuiFriendsUI(DWORD, DWORD);
DWORD XShowNuiPartyUI(DWORD, DWORD);
DWORD XShowNuiGuideUI(DWORD);

DWORD XUserCheckPrivilege(DWORD, XPRIVILEGE_TYPE, BOOL *);
XUSER_SIGNIN_STATE XUserGetSigninState(DWORD);
DWORD XUserGetXUID(DWORD, XUID *);
void XUserSetContext(DWORD, DWORD, DWORD);
DWORD XUserWriteAchievements(DWORD, XUSER_ACHIEVEMENT *, XOVERLAPPED *);

DWORD XContentClose(const char *, XOVERLAPPED *);
DWORD XContentGetDeviceData(DWORD, XDEVICE_DATA *);

DWORD XCancelOverlapped(XOVERLAPPED *);
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
    DWORD,
    LPCSTR,
    XCONTENT_CROSS_TITLE_DATA *,
    DWORD,
    DWORD *,
    DWORD *,
    int,
    ULONGLONG,
    XOVERLAPPED *
);
DWORD XContentCrossTitleDelete(DWORD, const XCONTENT_CROSS_TITLE_DATA *, XOVERLAPPED *);

DWORD XGetOverlappedExtendedError(XOVERLAPPED *);
DWORD XGetOverlappedResult(XOVERLAPPED *, DWORD *, BOOL);

DWORD XUserGetSigninInfo(DWORD, DWORD, XUSER_SIGNIN_INFO *);

DWORD XSetThreadProcessor(HANDLE, DWORD);

DWORD XContentCreateEx(
    DWORD,
    LPCSTR,
    CONST XCONTENT_DATA *,
    DWORD,
    DWORD *,
    DWORD *,
    DWORD,
    ULARGE_INTEGER,
    XOVERLAPPED *
);
DWORD XContentGetCreator(DWORD, CONST XCONTENT_DATA *, BOOL *, XUID *, XOVERLAPPED *);
DWORD XContentGetDeviceState(DWORD, XOVERLAPPED *);
DWORD XContentDelete(DWORD, CONST XCONTENT_DATA *, XOVERLAPPED *);
DWORD XContentCreateEnumerator(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD *, HANDLE *);
DWORD XEnumerate(HANDLE, VOID *, DWORD, DWORD *, XOVERLAPPED *);
DWORD XContentFlush(LPCSTR, XOVERLAPPED *);
ULONGLONG XContentCalculateSize(ULONGLONG, DWORD);

void XAudioGetSpeakerConfig();
void XGetVideoMode(XVIDEO_MODE *);

void *XMemSet(VOID *dest, INT c, SIZE_T count);
void *XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes);

DWORD XUserAwardGamerPicture(
    DWORD dwUserIndex, DWORD dwPictureId, DWORD dwReserved, XOVERLAPPED *pXOverlapped
);
DWORD XUserAwardAvatarAssets(
    DWORD dwNumAssets, const XUSER_AVATARASSET *pAssets, XOVERLAPPED *pOverlapped
);

DWORD XTitleServerCreateEnumerator(
    LPCSTR pszServerInfo, DWORD cItem, DWORD *pcbBuffer, HANDLE *hEnum
);

DWORD XNetGetConnectStatus(const in_addr ina);
INT XNetUnregisterInAddr(const in_addr ina);
INT XNetConnect(const in_addr ina);

DWORD XSessionStart(HANDLE hSession, DWORD dwFlags, XOVERLAPPED *pXOverlapped);
DWORD XSessionEnd(HANDLE hSession, XOVERLAPPED *pXOverlapped);
DWORD XSessionWriteStats(
    HANDLE hSession,
    XUID xuid,
    DWORD dwNumViews,
    XSESSION_VIEW_PROPERTIES *pViews,
    XOVERLAPPED *pXOverlapped
);
DWORD XSessionCreate(
    DWORD dwFlags,
    DWORD dwUserIndex,
    DWORD dwMaxPublicSlots,
    DWORD dwMaxPrivateSlots,
    ULONGLONG *pqwSessionNonce,
    XSESSION_INFO *pSessionInfo,
    XOVERLAPPED *pXOverlapped,
    HANDLE *ph
);
DWORD XSessionDelete(HANDLE hSession, XOVERLAPPED *pXOverlapped);
DWORD XSessionJoinLocal(
    HANDLE hSession,
    DWORD dwUserCount,
    const DWORD *pdwUserIndexes,
    const BOOL *pfPrivateSlots,
    XOVERLAPPED *pXOverlapped
);
DWORD XSessionLeaveLocal(
    HANDLE hSession,
    DWORD dwUserCount,
    const DWORD *pdwUserIndexes,
    XOVERLAPPED *pXOverlapped
);

DWORD XUserGetName(DWORD dwUserIndex, LPSTR szUserName, DWORD cchUserName);

#ifdef __cplusplus
}
#endif
