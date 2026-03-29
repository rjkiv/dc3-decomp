#pragma once
#include "../win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xonline/xonline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XPARTY_CUSTOM_DATA { /* Size=0x10 */
    /* 0x0000 */ QWORD qwFirst;
    /* 0x0008 */ QWORD qwSecond;
} XPARTY_CUSTOM_DATA;

typedef struct _XPARTY_USER_INFO { /* Size=0x78 */
    /* 0x0000 */ XUID Xuid;
    /* 0x0008 */ char GamerTag[16];
    /* 0x0018 */ DWORD dwUserIndex;
    /* 0x001c */ XONLINE_NAT_TYPE NatType;
    /* 0x0020 */ DWORD dwTitleId;
    /* 0x0024 */ DWORD dwFlags;
    /* 0x0028 */ _XSESSION_INFO SessionInfo;
    /* 0x0068 */ _XPARTY_CUSTOM_DATA CustomData;
} XPARTY_USER_INFO;

typedef struct _XPARTY_USER_LIST { /* Size=0xf08 */
    /* 0x0000 */ DWORD dwUserCount;
    /* 0x0008 */ XPARTY_USER_INFO Users[32];
} XPARTY_USER_LIST;

HRESULT XPartyGetUserList(XPARTY_USER_LIST *pUserList);
DWORD XPartySendGameInvites(DWORD dwUserIndex, XOVERLAPPED *pOverlapped);

#ifdef __cplusplus
}
#endif
