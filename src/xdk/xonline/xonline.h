#pragma once
#include "../win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xnet/winsockx.h"

#ifdef __cplusplus
extern "C" {
#endif

enum XONLINE_NAT_TYPE {
    XONLINE_NAT_OPEN = 0x0001,
    XONLINE_NAT_MODERATE = 0x0002,
    XONLINE_NAT_STRICT = 0x0003,
};

typedef struct _XSESSION_VIEW_PROPERTIES { /* Size=0xc */
    /* 0x0000 */ DWORD dwViewId;
    /* 0x0004 */ DWORD dwNumProperties;
    /* 0x0008 */ _XUSER_PROPERTY *pProperties;
} XSESSION_VIEW_PROPERTIES;

typedef struct _XSTORAGE_FILE_INFO { /* Size=0x41 */
    /* 0x0000 */ DWORD dwTitleID;
    /* 0x0004 */ DWORD dwTitleVersion;
    /* 0x0008 */ QWORD qwOwnerPUID;
    /* 0x0010 */ BYTE bCountryID;
    /* 0x0011 */ QWORD qwReserved;
    /* 0x0019 */ DWORD dwContentType;
    /* 0x001d */ DWORD dwStorageSize;
    /* 0x0021 */ DWORD dwInstalledSize;
    /* 0x0025 */ FILETIME ftCreated;
    /* 0x002d */ FILETIME ftLastModified;
    /* 0x0035 */ WORD wAttributesSize;
    /* 0x0037 */ WORD cchPathName;
    /* 0x0039 */ WCHAR *pwszPathName;
    /* 0x003d */ BYTE *pbAttributes;
} XSTORAGE_FILE_INFO;

typedef struct _XSTORAGE_ENUMERATE_RESULTS { /* Size=0xc */
    /* 0x0000 */ DWORD dwTotalNumItems;
    /* 0x0004 */ DWORD dwNumItemsReturned;
    /* 0x0008 */ XSTORAGE_FILE_INFO *pItems;
} XSTORAGE_ENUMERATE_RESULTS;

DWORD XTitleServerCreateEnumerator(
    LPCSTR pszServerInfo, DWORD cItem, DWORD *pcbBuffer, HANDLE *hEnum
);

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
DWORD XOnlineStartup();
DWORD XOnlineCleanup();
DWORD XMarketplaceCreateOfferEnumeratorByOffering(
    DWORD dwUserIndex,
    DWORD cItem,
    const QWORD *pqwOfferIds,
    WORD cOfferIds,
    DWORD *pcbBuffer,
    HANDLE *phEnum
);

#ifdef __cplusplus
}
#endif
