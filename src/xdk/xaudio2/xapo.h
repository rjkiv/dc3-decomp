#pragma once
#include "xdk/win_types.h"
#include "xdk/unknwn.h"
#include "xdk/XAPILIB.h"

// https://learn.microsoft.com/en-us/windows/win32/api/xapo/

struct XAPO_REGISTRATION_PROPERTIES { /* Size=0x42c */
    /* 0x0000 */ _GUID clsid;
    /* 0x0010 */ WCHAR FriendlyName[256];
    /* 0x0210 */ WCHAR CopyrightInfo[256];
    /* 0x0410 */ UINT MajorVersion;
    /* 0x0414 */ UINT MinorVersion;
    /* 0x0418 */ UINT Flags;
    /* 0x041c */ UINT MinInputBufferCount;
    /* 0x0420 */ UINT MaxInputBufferCount;
    /* 0x0424 */ UINT MinOutputBufferCount;
    /* 0x0428 */ UINT MaxOutputBufferCount;
};

#pragma pack(push, 1)
// https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex
typedef struct tWAVEFORMATEX { /* Size=0x12 */
    /* 0x0000 */ WORD wFormatTag;
    /* 0x0002 */ WORD nChannels;
    /* 0x0004 */ DWORD nSamplesPerSec;
    /* 0x0008 */ DWORD nAvgBytesPerSec;
    /* 0x000c */ WORD nBlockAlign;
    /* 0x000e */ WORD wBitsPerSample;
    /* 0x0010 */ WORD cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *NPWAVEFORMATEX, *LPWAVEFORMATEX;
#pragma pack(pop)

enum XAPO_BUFFER_FLAGS {
    XAPO_BUFFER_SILENT = 0x0000,
    XAPO_BUFFER_VALID = 0x0001,
};

struct XAPO_PROCESS_BUFFER_PARAMETERS { /* Size=0xc */
    /* 0x0000 */ void *pBuffer;
    /* 0x0004 */ XAPO_BUFFER_FLAGS BufferFlags;
    /* 0x0008 */ UINT ValidFrameCount;
};

struct XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS { /* Size=0x8 */
    /* 0x0000 */ const WAVEFORMATEX *pFormat;
    /* 0x0004 */ UINT MaxFrameCount;
};

DEFINE_IID(IXAPO, A90BC001, E897, E897, 55, E4, 9E, 47, 00, 00, 00, 00);

struct IXAPO : public IUnknown { /* Size=0x4 */
    /* 0x0000: fields for IUnknown */

    virtual HRESULT
    GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **ppRegistrationProperties);
    virtual HRESULT IsInputFormatSupported(
        const WAVEFORMATEX *pOutputFormat,
        const WAVEFORMATEX *pRequestedInputFormat,
        WAVEFORMATEX **ppSupportedInputFormat
    );
    virtual HRESULT IsOutputFormatSupported(
        const WAVEFORMATEX *pInputFormat,
        const WAVEFORMATEX *pRequestedOutputFormat,
        WAVEFORMATEX **ppSupportedOutputFormat
    );
    virtual HRESULT Initialize(const void *pData, UINT32 DataByteSize);
    virtual void Reset();
    virtual HRESULT LockForProcess(
        UINT32 InputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
        UINT32 OutputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters
    );
    virtual void UnlockForProcess();
    virtual void Process(
        UINT32 InputProcessParameterCount,
        const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
        UINT32 OutputProcessParameterCount,
        XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
        BOOL IsEnabled
    );
    virtual UINT32 CalcInputFrames(UINT32 OutputFrameCount);
    virtual UINT32 CalcOutputFrames(UINT32 InputFrameCount);
    IXAPO(const IXAPO &);
    IXAPO();
    IXAPO &operator=(const IXAPO &);
};

DEFINE_IID(IXAPOParameters, A90BC001, E897, E897, 55, E4, 9E, 47, 00, 00, 00, 01);

struct IXAPOParameters : public IUnknown { /* Size=0x4 */
    /* 0x0000: fields for IUnknown */

    virtual void SetParameters(const void *pParameters, UINT32 ParameterByteSize);
    virtual void GetParameters(void *pParameters, UINT32 ParameterByteSize);
    IXAPOParameters(const IXAPOParameters &);
    IXAPOParameters();
    IXAPOParameters &operator=(const IXAPOParameters &);
};
