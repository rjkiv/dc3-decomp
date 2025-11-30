#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _XBACKGROUND_DOWNLOAD_MODE {
    XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW = 0x0001,
    XBACKGROUND_DOWNLOAD_MODE_AUTO = 0x0002,
} XBACKGROUND_DOWNLOAD_MODE;

enum XC_LANGUAGE {
    XC_LANGUAGE_UNKNOWN = 0,
    XC_LANGUAGE_ENGLISH = 1,
    XC_LANGUAGE_JAPANESE = 2,
    XC_LANGUAGE_GERMAN = 3,
    XC_LANGUAGE_FRENCH = 4,
    XC_LANGUAGE_SPANISH = 5,
    XC_LANGUAGE_ITALIAN = 6,
    XC_LANGUAGE_KOREAN = 7,
    XC_LANGUAGE_TCHINESE = 8,
    XC_LANGUAGE_PORTUGUESE = 9,
    XC_LANGUAGE_SCHINESE = 10,
    XC_LANGUAGE_POLISH = 11,
    XC_LANGUAGE_RUSSIAN = 12
};

enum XC_LOCALE {
    XC_LOCALE_UNKNOWN = 0,
    XC_LOCALE_AUSTRALIA = 1,
    XC_LOCALE_AUSTRIA = 2,
    XC_LOCALE_BELGIUM = 3,
    XC_LOCALE_BRAZIL = 4,
    XC_LOCALE_CANADA = 5,
    XC_LOCALE_CHILE = 6,
    XC_LOCALE_CHINA = 7,
    XC_LOCALE_COLOMBIA = 8,
    XC_LOCALE_CZECH_REPUBLIC = 9,
    XC_LOCALE_DENMARK = 10,
    XC_LOCALE_FINLAND = 11,
    XC_LOCALE_FRANCE = 12,
    XC_LOCALE_GERMANY = 13,
    XC_LOCALE_GREECE = 14,
    XC_LOCALE_HONG_KONG = 15,
    XC_LOCALE_HUNGARY = 16,
    XC_LOCALE_INDIA = 17,
    XC_LOCALE_IRELAND = 18,
    XC_LOCALE_ITALY = 19,
    XC_LOCALE_JAPAN = 20,
    XC_LOCALE_KOREA = 21,
    XC_LOCALE_MEXICO = 22,
    XC_LOCALE_NETHERLANDS = 23,
    XC_LOCALE_NEW_ZEALAND = 24,
    XC_LOCALE_NORWAY = 25,
    XC_LOCALE_POLAND = 26,
    XC_LOCALE_PORTUGAL = 27,
    XC_LOCALE_SINGAPORE = 28,
    XC_LOCALE_SLOVAK_REPUBLIC = 29,
    XC_LOCALE_SOUTH_AFRICA = 30,
    XC_LOCALE_SPAIN = 31,
    XC_LOCALE_SWEDEN = 32,
    XC_LOCALE_SWITZERLAND = 33,
    XC_LOCALE_TAIWAN = 34,
    XC_LOCALE_GREAT_BRITAIN = 35,
    XC_LOCALE_UNITED_STATES = 36,
    XC_LOCALE_RUSSIAN_FEDERATION = 37
};

enum XAUDIO2_FILTER_TYPE {
    LowPassFilter = 0x0000,
    BandPassFilter = 0x0001,
    HighPassFilter = 0x0002,
    NotchFilter = 0x0003,
};

struct XUSER_ACHIEVEMENT { /* Size=0x8 */
    /* 0x0000 */ DWORD dwUserIndex;
    /* 0x0004 */ DWORD dwAchievementId;
};

typedef VOID XOVERLAPPED_COMPLETION_ROUTINE(DWORD, DWORD, struct _XOVERLAPPED *);

typedef struct _XOVERLAPPED { /* Size=0x1c */
    ULONG_PTR InternalLow;
    ULONG_PTR InternalHigh;
    ULONG_PTR InternalContext;
    HANDLE hEvent;
    XOVERLAPPED_COMPLETION_ROUTINE *pCompletionRoutine;
    DWORD_PTR dwCompletionContext;
    DWORD dwExtendedError;
} XOVERLAPPED;

typedef enum _XPRIVILEGE_TYPE {
    XPRIVILEGE_MULTIPLAYER_SESSIONS = 0x00fe,
    XPRIVILEGE_COMMUNICATIONS = 0x00fc,
    XPRIVILEGE_COMMUNICATIONS_FRIENDS_ONLY = 0x00fb,
    XPRIVILEGE_PROFILE_VIEWING = 0x00f9,
    XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY = 0x00f8,
    XPRIVILEGE_USER_CREATED_CONTENT = 0x00f7,
    XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY = 0x00f6,
    XPRIVILEGE_PURCHASE_CONTENT = 0x00f5,
    XPRIVILEGE_PRESENCE = 0x00f4,
    XPRIVILEGE_PRESENCE_FRIENDS_ONLY = 0x00f3,
    XPRIVILEGE_TRADE_CONTENT = 0x00ee,
    XPRIVILEGE_VIDEO_COMMUNICATIONS = 0x00eb,
    XPRIVILEGE_VIDEO_COMMUNICATIONS_FRIENDS_ONLY = 0x00ea,
    XPRIVILEGE_SOCIAL_NETWORK_SHARING = 0x00dc,
    XPRIVILEGE_CONTENT_AUTHOR = 0x00de,
    XPRIVILEGE_UNSAFE_PROGRAMMING = 0x00d4,
    XPRIVILEGE_SHARE_CONTENT_OUTSIDE_LIVE = 0x00d3,
    XPRIVILEGE_INTERNET_BROWSING = 0x00d9,
} XPRIVILEGE_TYPE;

typedef enum _XUSER_SIGNIN_STATE {
    eXUserSigninState_NotSignedIn = 0x0000,
    eXUserSigninState_SignedInLocally = 0x0001,
    eXUserSigninState_SignedInToLive = 0x0002,
} XUSER_SIGNIN_STATE;

typedef unsigned long long XUID;

typedef struct _XDEVICE_DATA { /* Size=0x50 */
    /* 0x0000 */ DWORD DeviceID;
    /* 0x0004 */ DWORD DeviceType;
    /* 0x0008 */ ULONGLONG ulDeviceBytes;
    /* 0x0010 */ ULONGLONG ulDeviceFreeBytes;
    /* 0x0018 */ WCHAR wszFriendlyName[27];
} XDEVICE_DATA, *PXDEVICE_DATA;

#define XCONTENT_MAX_FILENAME_LENGTH 42
#define XCONTENT_MAX_DISPLAYNAME_LENGTH 128

typedef struct _XCONTENT_DATA { /* Size=0x134 */
    /* 0x0000 */ DWORD DeviceID;
    /* 0x0004 */ DWORD dwContentType;
    /* 0x0008 */ WCHAR szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
    /* 0x0108 */ CHAR szFileName[XCONTENT_MAX_FILENAME_LENGTH];
} XCONTENT_DATA;

typedef struct _XCONTENT_CROSS_TITLE_DATA { /* Size=0x138 */
    /* 0x0000 */ DWORD DeviceID;
    /* 0x0004 */ DWORD dwContentType;
    /* 0x0008 */ WCHAR szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
    /* 0x0108 */ CHAR szFileName[XCONTENT_MAX_FILENAME_LENGTH];
    /* 0x0134 */ DWORD dwTitleId;
} XCONTENT_CROSS_TITLE_DATA;

typedef struct _XUSER_SIGNIN_INFO { /* Size=0x28 */
    XUID xuid; // 0x0
    DWORD dwInfoFlags; // 0x8
    XUSER_SIGNIN_STATE UserSigninState; // 0xc
    DWORD dwGuestNumber; // 0x10
    DWORD dwSponsorUserIndex; // 0x14
    char szUserName[16]; // 0x18
} XUSER_SIGNIN_INFO;

typedef enum _XCONTENTDEVICEID {
    XCONTENTDEVICE_ANY = 0
    // fill in others as you find them out
} XCONTENTDEVICEID;

struct _GUID { /* Size=0x10 */
    /* 0x0000 */ DWORD Data1;
    /* 0x0004 */ WORD Data2;
    /* 0x0006 */ WORD Data3;
    /* 0x0008 */ BYTE Data4[8];
};

struct IUnknown { /* Size=0x4 */

    virtual DWORD QueryInterface(const _GUID &, void **);
    virtual ULONG AddRef();
    virtual ULONG Release();
    IUnknown(const IUnknown &);
    IUnknown();
    IUnknown &operator=(const IUnknown &);
};

struct XAUDIO2_BUFFER { /* Size=0x24 */
    /* 0x0000 */ UINT32 Flags;
    /* 0x0004 */ UINT32 AudioBytes;
    /* 0x0008 */ const BYTE *pAudioData;
    /* 0x000c */ UINT32 PlayBegin;
    /* 0x0010 */ UINT32 PlayLength;
    /* 0x0014 */ UINT32 LoopBegin;
    /* 0x0018 */ UINT32 LoopLength;
    /* 0x001c */ UINT32 LoopCount;
    /* 0x0020 */ void *pContext;
};

struct tWAVEFORMATEX { /* Size=0x12 */
    /* 0x0000 */ WORD wFormatTag;
    /* 0x0002 */ WORD nChannels;
    /* 0x0004 */ DWORD nSamplesPerSec;
    /* 0x0008 */ DWORD nAvgBytesPerSec;
    /* 0x000c */ WORD nBlockAlign;
    /* 0x000e */ WORD wBitsPerSample;
    /* 0x0010 */ WORD cbSize;
};

struct XMA2WAVEFORMATEX { /* Size=0x34 */
    /* 0x0000 */ tWAVEFORMATEX wfx;
    /* 0x0012 */ WORD NumStreams;
    /* 0x0014 */ DWORD ChannelMask;
    /* 0x0018 */ DWORD SamplesEncoded;
    /* 0x001c */ DWORD BytesPerBlock;
    /* 0x0020 */ DWORD PlayBegin;
    /* 0x0024 */ DWORD PlayLength;
    /* 0x0028 */ DWORD LoopBegin;
    /* 0x002c */ DWORD LoopLength;
    /* 0x0030 */ BYTE LoopCount;
    /* 0x0031 */ BYTE EncoderVersion;
    /* 0x0032 */ WORD BlockCount;
};

struct XAUDIO2_VOICE_DETAILS { /* Size=0xc */
    /* 0x0000 */ UINT32 CreationFlags;
    /* 0x0004 */ UINT32 InputChannels;
    /* 0x0008 */ UINT32 InputSampleRate;
};

struct XAUDIO2_EFFECT_DESCRIPTOR { /* Size=0xc */
    /* 0x0000 */ IUnknown *pEffect;
    /* 0x0004 */ BOOL InitialState;
    /* 0x0008 */ UINT32 OutputChannels;
};

struct XAUDIO2_EFFECT_CHAIN { /* Size=0x8 */
    /* 0x0000 */ UINT32 EffectCount;
    /* 0x0004 */ XAUDIO2_EFFECT_DESCRIPTOR *pEffectDescriptors;
};

struct XAUDIO2_FILTER_PARAMETERS { /* Size=0xc */
    /* 0x0000 */ XAUDIO2_FILTER_TYPE Type;
    /* 0x0004 */ float Frequency;
    /* 0x0008 */ float OneOverQ;
};

struct XAUDIO2_SEND_DESCRIPTOR { /* Size=0x8 */
    /* 0x0000 */ UINT32 Flags;
    ///* 0x0004 */  IXAudio2Voice *pOutputVoice;
};

struct XAUDIO2_VOICE_SENDS { /* Size=0x8 */
    /* 0x0000 */ UINT32 SendCount;
/* 0x0004 */ XAUDIO2_SEND_DESCRIPTOR
*pSends;
}
;

struct NUI_TALKER_POSITION { /* Size=0x8 */
    /* 0x0000 */ float fDirection;
    /* 0x0004 */ float fConfidence;
};

#ifdef __cplusplus
}
#endif
