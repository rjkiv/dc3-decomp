#pragma once
#include "xdk/win_types.h"
#include "xdk/unknwn.h"
#include "xdk/XAPILIB.h"
#include <string.h>

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
    /* 0x0000 */ const tWAVEFORMATEX *pFormat;
    /* 0x0004 */ UINT MaxFrameCount;
};

struct IXAPO : public IUnknown { /* Size=0x4 */
    /* 0x0000: fields for IUnknown */

    virtual INT GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **);
    virtual INT
    IsInputFormatSupported(const tWAVEFORMATEX *, const tWAVEFORMATEX *, tWAVEFORMATEX **);
    virtual INT IsOutputFormatSupported(
        const tWAVEFORMATEX *, const tWAVEFORMATEX *, tWAVEFORMATEX **
    );
    virtual INT Initialize(const void *, UINT);
    virtual void Reset();
    virtual HRESULT LockForProcess(
        UINT InputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
        UINT OutputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters
    );
    virtual void UnlockForProcess();
    virtual void Process(
        UINT,
        const XAPO_PROCESS_BUFFER_PARAMETERS *,
        UINT,
        XAPO_PROCESS_BUFFER_PARAMETERS *,
        INT
    );
    virtual UINT CalcInputFrames(UINT);
    virtual UINT CalcOutputFrames(UINT);
    IXAPO(const IXAPO &);
    IXAPO();
    IXAPO &operator=(const IXAPO &);
};

class CXAPOBase : public IXAPO { /* Size=0x20 */
private:
    /* 0x0000: fields for IXAPO */
    /* 0x0004 */ const XAPO_REGISTRATION_PROPERTIES *m_pRegistrationProperties;
    /* 0x0008 */ void *m_pfnMatrixMixFunction;
    /* 0x000c */ float *m_pfl32MatrixCoefficients;
    /* 0x0010 */ UINT m_nSrcFormatType;
    /* 0x0014 */ BOOL m_fIsScalarMatrix;
    /* 0x0018 */ BOOL m_fIsLocked;

protected:
    /* 0x001c */ INT m_lReferenceCount;

    virtual INT ValidateFormatDefault(tWAVEFORMATEX *, INT);
    INT ValidateFormatPair(const tWAVEFORMATEX *, tWAVEFORMATEX *, INT);
    void ProcessThru(void *, float *, UINT, USHORT, USHORT, INT);
    const XAPO_REGISTRATION_PROPERTIES *GetRegistrationPropertiesInternal();
    BOOL IsLocked();

public:
    CXAPOBase(const CXAPOBase &);
    CXAPOBase(const XAPO_REGISTRATION_PROPERTIES *);
    CXAPOBase &operator=(const CXAPOBase &);
    virtual ~CXAPOBase();
    virtual HRESULT QueryInterface(const _GUID &, void **);
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual INT GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **);
    virtual BOOL
    IsInputFormatSupported(const tWAVEFORMATEX *, const tWAVEFORMATEX *, tWAVEFORMATEX **);
    virtual BOOL IsOutputFormatSupported(
        const tWAVEFORMATEX *, const tWAVEFORMATEX *, tWAVEFORMATEX **
    );
    virtual INT Initialize(const void *, UINT);
    virtual void Reset();
    virtual HRESULT LockForProcess(
        UINT InputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
        UINT OutputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters
    );
    virtual void UnlockForProcess();
    virtual UINT CalcInputFrames(UINT);
    virtual UINT CalcOutputFrames(UINT);
};

struct IXAPOParameters : public IUnknown { /* Size=0x4 */
    /* 0x0000: fields for IUnknown */

    virtual void SetParameters(const void *, UINT);
    virtual void GetParameters(void *, UINT);
    IXAPOParameters(const IXAPOParameters &);
    IXAPOParameters();
    IXAPOParameters &operator=(const IXAPOParameters &);
};

class CXAPOParametersBase : public CXAPOBase, public IXAPOParameters { /* Size=0x40 */
private:
    /* 0x0000: fields for CXAPOBase */
    /* 0x0020: fields for IXAPOParameters */
    /* 0x0024 */ unsigned char *m_pParameterBlocks;
    /* 0x0028 */ unsigned char *m_pCurrentParameters;
    /* 0x002c */ unsigned char *m_pCurrentParametersInternal;
    /* 0x0030 */ UINT m_uCurrentParametersIndex;
    /* 0x0034 */ UINT m_uParameterBlockByteSize;
    /* 0x0038 */ BOOL m_fNewerResultsReady;
    /* 0x003c */ BOOL m_fProducer;

public:
    CXAPOParametersBase(const CXAPOParametersBase &);
    CXAPOParametersBase(const XAPO_REGISTRATION_PROPERTIES *, unsigned char *, UINT, BOOL);
    CXAPOParametersBase &operator=(const CXAPOParametersBase &);
    virtual ~CXAPOParametersBase();
    virtual HRESULT QueryInterface(const _GUID &, void **);
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual void SetParameters(const void *, UINT);
    virtual void GetParameters(void *, UINT);
    virtual void OnSetParameters(const void *, UINT);
    BOOL ParametersChanged();
    unsigned char *BeginProcess();
    void EndProcess();
};

namespace ATG {
    template <class Effect, typename Params>
    class CSampleXAPOBase : public CXAPOParametersBase {
    private:
        // TODO: how am i supposed to instantiate this
        // if every Effect has a different guid?
        static XAPO_REGISTRATION_PROPERTIES m_regProps;

    public:
        virtual HRESULT LockForProcess(
            UINT InputLockedParameterCount,
            const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
            UINT OutputLockedParameterCount,
            const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters
        ) {
            HRESULT ret = CXAPOBase::LockForProcess(
                InputLockedParameterCount,
                pInputLockedParameters,
                OutputLockedParameterCount,
                pOutputLockedParameters
            );
            if (SUCCEEDED(ret)) {
                mWav = *pInputLockedParameters->pFormat;
            }
            return ret;
        }
        virtual void Process(
            UINT InputProcessParameterCount,
            const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
            UINT OutputProcessParameterCount,
            XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
            INT
        ) {
            Params *params = (Params *)BeginProcess();
            if (pInputProcessParameters->BufferFlags == XAPO_BUFFER_SILENT) {
                memset(
                    pInputProcessParameters->pBuffer,
                    0,
                    pInputProcessParameters->ValidFrameCount * mWav.nChannels * 4
                );
            } else if (pInputProcessParameters->BufferFlags != XAPO_BUFFER_VALID) {
                EndProcess();
                return;
            }
            DoProcess(
                *params,
                (float *)pInputProcessParameters->pBuffer,
                pInputProcessParameters->ValidFrameCount,
                mWav.nChannels
            );
            EndProcess();
        }

    protected:
        CSampleXAPOBase()
            : CXAPOParametersBase(
                  &m_regProps, (unsigned char *)mParams, sizeof(Params), false
              ) {
            XMemSet((VOID *)mParams, 0, sizeof(mParams));
        }
        virtual ~CSampleXAPOBase() {}
        virtual void OnSetParameters(const void *, unsigned int);
        virtual void OnSetParameters(const Params &) {}
        virtual void
        DoProcess(const Params &, float *__restrict, unsigned int, unsigned int) = 0;

        Params mParams[3]; // 0x40
        tWAVEFORMATEX mWav; // 0x58
    };
}
