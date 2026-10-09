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

#pragma pack(push, 1)
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

typedef struct _XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS { /* Size=0x14 */
    /* 0x0000 */ DWORD dwBytesTotal;
    /* 0x0004 */ XUID xuidOwner;
    /* 0x000c */ FILETIME ftCreated;
} XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS;
#pragma pack(pop)

typedef enum _XSTORAGE_FACILITY {
    XSTORAGE_FACILITY_GAME_CLIP = 1,
    XSTORAGE_FACILITY_PER_TITLE = 2,
    XSTORAGE_FACILITY_PER_USER_TITLE = 3,
} XSTORAGE_FACILITY;

#define XONLINE_FRIENDSTATE_FLAG_REQUEST 0x40000000
#define XONLINE_FRIENDSTATE_FLAG_PENDING 0x80000000

#pragma pack(push, 4)
typedef struct _XONLINE_FRIEND { /* Size=0xc4 */
    /* 0x0000 */ XUID xuid;
    /* 0x0008 */ char szGamertag[16];
    /* 0x0018 */ DWORD dwFriendState;
    /* 0x001c */ XNKID sessionID;
    /* 0x0024 */ DWORD dwTitleID;
    /* 0x0028 */ FILETIME ftUserTime;
    /* 0x0030 */ XNKID xnkidInvite;
    /* 0x0038 */ FILETIME gameinviteTime;
    /* 0x0040 */ DWORD cchRichPresence;
    /* 0x0044 */ WCHAR wszRichPresence[64];
} XONLINE_FRIEND;
#pragma pack(pop)

struct XTITLE_SERVER_INFO { /* Size=0xd0 */
    /* 0x0000 */ IN_ADDR inaServer;
    /* 0x0004 */ DWORD dwFlags;
    /* 0x0008 */ char szServerInfo[200];
};

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
DWORD XFriendsCreateEnumerator(
    DWORD dwUserIndex,
    DWORD dwStartingIndex,
    DWORD dwFriendsToReturn,
    DWORD *pcbBuffer,
    HANDLE *ph
);
DWORD XStorageBuildServerPath(
    DWORD dwUserIndex,
    XSTORAGE_FACILITY StorageFacility,
    const void *pvStorageFacilityInfo,
    DWORD dwStorageFacilityInfoSize,
    LPCWSTR pwszItemName,
    WCHAR *pwszServerPath,
    DWORD *pdwServerPathLength
);
DWORD XStorageEnumerate(
    DWORD dwUserIndex,
    LPCWSTR pwszServerPath,
    DWORD dwStartingIndex,
    DWORD dwMaxResultsToReturn,
    DWORD cbResultsBuffer,
    XSTORAGE_ENUMERATE_RESULTS *pResults,
    XOVERLAPPED *pXOverlapped
);
DWORD XStorageDownloadToMemory(
    DWORD dwUserIndex,
    LPCWSTR pwszServerPath,
    DWORD dwBufferSize,
    const BYTE *pbBuffer,
    DWORD cbResults,
    XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS *pResults,
    XOVERLAPPED *pXOverlapped
);
DWORD XOnlineStartup();
DWORD XOnlineCleanup();
DWORD XMarketplaceCreateOfferEnumerator(
    DWORD dwUserIndex,
    DWORD dwOfferType,
    DWORD dwContentCategories,
    WORD cItem,
    DWORD *pcbBuffer,
    HANDLE *phEnum
);
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
