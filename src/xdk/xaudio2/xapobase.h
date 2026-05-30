#pragma once
#include "xapo.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include <string.h>

// https://learn.microsoft.com/en-us/windows/win32/api/xapobase/

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

    virtual HRESULT ValidateFormatDefault(WAVEFORMATEX *pFormat, BOOL fOverwrite);

    HRESULT ValidateFormatPair(
        const WAVEFORMATEX *pSupportedFormat,
        WAVEFORMATEX *pRequestedFormat,
        BOOL fOverwrite
    );
    void ProcessThru(
        const void *pInputBuffer,
        float *pOutputBuffer,
        UINT32 FrameCount,
        UINT32 InputChannelCount,
        UINT32 OutputChannelCount,
        BOOL MixWithOutput
    );
    const XAPO_REGISTRATION_PROPERTIES *GetRegistrationPropertiesInternal();
    BOOL IsLocked();

public:
    CXAPOBase(const CXAPOBase &);
    CXAPOBase(const XAPO_REGISTRATION_PROPERTIES *pRegistrationProperties);
    CXAPOBase &operator=(const CXAPOBase &);
    virtual ~CXAPOBase();
    virtual HRESULT QueryInterface(const _GUID &, void **);
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual HRESULT GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **);
    virtual HRESULT
    IsInputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **);
    virtual HRESULT
    IsOutputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **);
    virtual HRESULT Initialize(const void *, UINT32);
    virtual void Reset();
    virtual HRESULT LockForProcess(
        UINT32 InputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
        UINT32 OutputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters
    );
    virtual void UnlockForProcess();
    virtual UINT32 CalcInputFrames(UINT32);
    virtual UINT32 CalcOutputFrames(UINT32);
};

class CXAPOParametersBase : public CXAPOBase, public IXAPOParameters { /* Size=0x40 */
private:
    /* 0x0000: fields for CXAPOBase */
    /* 0x0020: fields for IXAPOParameters */
    /* 0x0024 */ BYTE *m_pParameterBlocks;
    /* 0x0028 */ BYTE *m_pCurrentParameters;
    /* 0x002c */ BYTE *m_pCurrentParametersInternal;
    /* 0x0030 */ UINT m_uCurrentParametersIndex;
    /* 0x0034 */ UINT m_uParameterBlockByteSize;
    /* 0x0038 */ BOOL m_fNewerResultsReady;
    /* 0x003c */ BOOL m_fProducer;

public:
    CXAPOParametersBase(const CXAPOParametersBase &);
    CXAPOParametersBase(
        const XAPO_REGISTRATION_PROPERTIES *pRegistrationProperties,
        BYTE *pParameterBlocks,
        UINT32 uParameterBlockByteSize,
        BOOL fProducer
    );
    CXAPOParametersBase &operator=(const CXAPOParametersBase &);
    virtual ~CXAPOParametersBase();
    virtual HRESULT QueryInterface(const _GUID &, void **);
    virtual ULONG AddRef();
    virtual ULONG Release();
    virtual void SetParameters(const void *pParameters, UINT32 ParameterByteSize);
    virtual void GetParameters(void *pParameters, UINT32 ParameterByteSize);
    virtual void OnSetParameters(const void *pParameters, UINT32 ParameterByteSize);
    BOOL ParametersChanged();
    BYTE *BeginProcess();
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
            : CXAPOParametersBase(&m_regProps, (BYTE *)mParams, sizeof(Params), false) {
            XMemSet((VOID *)mParams, 0, sizeof(mParams));
        }
        virtual ~CSampleXAPOBase() {}
        virtual void OnSetParameters(const void *, unsigned int);
        virtual void OnSetParameters(const Params &) {}
        virtual void
        DoProcess(const Params &, float *__restrict, unsigned int, unsigned int) = 0;

        Params mParams[3]; // 0x40
        WAVEFORMATEX mWav; // 0x58
    };
}
