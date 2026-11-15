#include "synth360/ExternalMic.h"
#include "macros.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth360/Mic.h"
#include "utl/Symbol.h"
#include "xdk/XAPILIB.h"
#include "xdk/XMIC.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xmic/xmic.h"
#include <cstring>

namespace {
    DWORD ExternalMicThreadEntry(void *v) {
        ExternalMic *mic = (ExternalMic *)v;
        return mic->sampleProcessThread();
    }

    void dataReadyEntry(
        DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, XOVERLAPPED *pOverlapped
    ) {
        ((RequestCompletionData *)pOverlapped->dwCompletionContext)
            ->pOwner->dataReady(dwErrorCode, dwNumberOfBytesTransferred, pOverlapped);
    }

    std::vector<ExternalMic *> gMics;
}

std::vector<ExternalMicClientProxy *> ExternalMicClientMgr::mMicMasters;
std::vector<DWORD> ExternalMicClientMgr::mDevToMicMaster;
std::vector<DWORD> ExternalMicClientMgr::mMicMasterToDev;
std::vector<MicXbox *> ExternalMicClientMgr::mAssocMicXbox;

#pragma region ExternalMic

ExternalMic::ExternalMic(DWORD micId)
    : mIndex(micId), mShutdown(false), mIsActive(false), mGain(-1) {
    mThread = CreateThread(nullptr, 0, ExternalMicThreadEntry, this, 4, 0);
    MILO_ASSERT(mThread, 0x6a);
    SetThreadPriority(mThread, 15);
    XSetThreadProcessor(mThread, 3);
    ResumeThread(mThread);
}

ExternalMic::~ExternalMic() {
    mShutdown = true;
    WaitForSingleObject(mThread, -1);
    CloseHandle(mThread);
}

HRESULT ExternalMic::gatherGainAttribs(DWORD dwMicrophoneIndex) {
    if (XMicGetGain(dwMicrophoneIndex, 1, &mMinGain) != ERROR_SUCCESS) {
        return E_FAIL;
    } else {
        HRESULT ret = XMicGetGain(dwMicrophoneIndex, 2, &mMaxGain);
        return ERROR_SUCCESS != ret ? E_FAIL : ERROR_SUCCESS;
    }
}

HRESULT ExternalMic::processGain(DWORD dwMicrophoneIndex) {
    float gainReq = ExternalMicClientMgr::GetRequiredGain(mIndex);
    if (gainReq != mGain) {
        MILO_ASSERT((0.0f <= gainReq) && (gainReq <= 1.0f), 0x26F);
        DWORD hr = XMicSetGain(
            dwMicrophoneIndex, (mMaxGain - mMinGain) * gainReq + mMinGain, nullptr
        );
        mGain = gainReq;
        if (hr != ERROR_SUCCESS) {
            return E_FAIL;
        }
    }
    return ERROR_SUCCESS;
}

void ExternalMic::Init() {
    MILO_ASSERT(gMics.empty(), 0x3B);
    ExternalMicClientMgr::Init();
    for (DWORD i = 0; i < 4; i++) {
        gMics.push_back(new ExternalMic(i));
    }
}

int ExternalMic::NumConnectedMics() {
    int numConnected = 0;
    for (int i = 0; i < gMics.size(); i++) {
        if (gMics[i]->mIsActive) {
            numConnected++;
        }
    }
    return numConnected;
}

void ExternalMic::Terminate() {
    for (DWORD i = 0; i < gMics.size(); i++) {
        delete gMics[i];
    }
    gMics.clear();
    ExternalMicClientMgr::Terminate();
}

void ExternalMic::dataReady(
    DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, XOVERLAPPED *pOverlapped
) {
    RequestCompletionData *ctx =
        (RequestCompletionData *)pOverlapped->dwCompletionContext;

    if (ctx) {
        int i = 0;
        unsigned int numBytes = 0;
        BYTE AudioSamples[2048] = { 0 };
        PBYTE p = ctx->pAudioFramesBuffer;
        for (; i < ctx->dwFrameCount; i++) {
            if (ctx->wFrameLengths[i]) {
                WORD wFrameLength = ctx->wFrameLengths[i];
                if (wFrameLength & 1) {
                    MILO_LOG(
                        "Mic data frame length was odd: %x bytes; truncating last byte\n",
                        wFrameLength
                    );
                    wFrameLength--;
                }
                memcpy(AudioSamples + numBytes, p, wFrameLength);
                numBytes += wFrameLength;
            }
            p += ctx->dwAudioFrameBufferLength;
        }
        if (numBytes != 0) {
            ExternalMicClientMgr::AddAudio(mIndex, AudioSamples, numBytes);
        }
        if (XMicGetStatus(ctx->dwMicrophoneIndex) == 2) {
            XMicRequestData(
                ctx->dwMicrophoneIndex,
                ctx->dwFrameCount,
                ctx->pAudioFramesBuffer,
                ctx->wFrameLengths,
                ctx->pOverlap
            );
        }
    }
}

DWORD ExternalMic::sampleProcessThread() {
    DWORD dwTransferLength = 0;
    DWORD dwMicrophoneIndex = mIndex | 0x4000000;
    RequestCompletionData MCD1;
    RequestCompletionData MCD2;

    while (!mShutdown) {
        memset(&MCD1, 0, sizeof(RequestCompletionData));
        memset(&MCD2, 0, sizeof(RequestCompletionData));

        do {
            Sleep(10);
        } while (XMicGetStatus(dwMicrophoneIndex) != 1 && !mShutdown);
        mIsActive = true;

        XMICCAPABILITIES capabilities;
        if (XMicGetCapabilities(dwMicrophoneIndex, &capabilities) == ERROR_SUCCESS) {
            DWORD b1 = capabilities.dwFeatures & 1;
            if (XMicStart(dwMicrophoneIndex, 256, &dwTransferLength, nullptr)
                    == ERROR_SUCCESS
                && SUCCEEDED(gatherGainAttribs(dwMicrophoneIndex))) {
                static Symbol generic_usb("generic_usb");
                Symbol micName = generic_usb;
                DataArray *sysConfig = SystemConfig("synth", "mic_types", "xbox");
                for (int i = 1; i < sysConfig->Size(); i++) {
                    DataArray *configArray = sysConfig->Array(i);
                    DataArray *capabilitiesArray = configArray->FindArray("capabilities");
                    if (capabilitiesArray->Int(1) == capabilities.dwFeatures
                        && capabilitiesArray->Int(2) == capabilities.wFormatTag
                        && capabilitiesArray->Int(3) == capabilities.nChannels
                        && capabilitiesArray->Int(4)
                            == capabilities.dwSampleRatesSupported
                        && capabilitiesArray->Int(5) == capabilities.wBitsPerSample
                        && capabilitiesArray->Int(6) == capabilities.dwFrameLength
                        && configArray->FindFloat("min_gain") == mMinGain
                        && configArray->FindFloat("max_gain") == mMaxGain) {
                        micName = configArray->Sym(0);
                        break;
                    }
                }

                ExternalMicClientProxy *proxy =
                    ExternalMicClientMgr::GetMasterForIndex(mIndex);
                if (proxy) {
                    HRESULT hr = proxy->OnMicConnected(256, b1, micName);
                    if (SUCCEEDED(hr)) {
                        XOVERLAPPED DataRequestOverlapped1;
                        XOVERLAPPED DataRequestOverlapped2;
                        DWORD u2 = b1 ? 1 : 2;

                        MCD1.dwMicrophoneIndex = dwMicrophoneIndex;
                        MCD1.dwFrameCount = u2;
                        MCD1.pOverlap = &DataRequestOverlapped1;
                        MCD1.dwAudioFrameBufferLength = dwTransferLength;
                        MCD1.pAudioFramesBufferAlloc =
                            new BYTE[dwTransferLength * u2 + 256];
                        MCD1.pAudioFramesBuffer = MCD1.pAudioFramesBufferAlloc + 128;
                        MCD1.pOwner = this;

                        MCD2.dwMicrophoneIndex = dwMicrophoneIndex;
                        MCD2.dwFrameCount = u2;
                        MCD2.pOverlap = &DataRequestOverlapped2;
                        MCD2.dwAudioFrameBufferLength = dwTransferLength;
                        MCD2.pAudioFramesBufferAlloc =
                            new BYTE[dwTransferLength * u2 + 256];
                        MCD2.pAudioFramesBuffer = MCD2.pAudioFramesBufferAlloc + 128;
                        MCD2.pOwner = this;

                        DataRequestOverlapped1.dwExtendedError = 0;
                        DataRequestOverlapped1.hEvent = nullptr;
                        DataRequestOverlapped1.pCompletionRoutine = dataReadyEntry;
                        DataRequestOverlapped1.dwCompletionContext = (DWORD_PTR)&MCD1;

                        DataRequestOverlapped2.dwExtendedError = 0;
                        DataRequestOverlapped2.hEvent = nullptr;
                        DataRequestOverlapped2.pCompletionRoutine = dataReadyEntry;
                        DataRequestOverlapped2.dwCompletionContext = (DWORD_PTR)&MCD2;

                        if (XMicRequestData(
                                MCD1.dwMicrophoneIndex,
                                MCD1.dwFrameCount,
                                MCD1.pAudioFramesBuffer,
                                MCD1.wFrameLengths,
                                MCD1.pOverlap
                            )
                            != ERROR_IO_PENDING) {
                            hr = E_FAIL;
                        }
                        if (SUCCEEDED(hr)) {
                            if (XMicRequestData(
                                    MCD2.dwMicrophoneIndex,
                                    MCD2.dwFrameCount,
                                    MCD2.pAudioFramesBuffer,
                                    MCD2.wFrameLengths,
                                    MCD2.pOverlap
                                )
                                != ERROR_IO_PENDING) {
                                hr = E_FAIL;
                            }
                            if (SUCCEEDED(hr)) {
                                while (!mShutdown) {
                                    SleepEx(10, true);
                                    hr = processGain(dwMicrophoneIndex);
                                    MILO_ASSERT(SUCCEEDED(hr), 0x149);
                                    if (XMicGetStatus(dwMicrophoneIndex) != 2) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        XMicStop(dwMicrophoneIndex, nullptr);
        mIsActive = false;
        ExternalMicClientMgr::OnMicDisconnected(mIndex);
        mGain = -1;
        if (MCD1.pAudioFramesBufferAlloc) {
            delete[] MCD1.pAudioFramesBufferAlloc;
            MCD1.pAudioFramesBuffer = nullptr;
            MCD1.pAudioFramesBufferAlloc = nullptr;
        }
        if (MCD2.pAudioFramesBufferAlloc) {
            delete[] MCD2.pAudioFramesBufferAlloc;
            MCD2.pAudioFramesBuffer = nullptr;
            MCD2.pAudioFramesBufferAlloc = nullptr;
        }
    }

    return 0;
}

#pragma endregion
#pragma region ExternalMicClientMgr

void ExternalMicClientMgr::Init() {
    mAssocMicXbox.reserve(4);
    mMicMasters.reserve(4);
    mDevToMicMaster.reserve(4);
    mMicMasterToDev.reserve(4);
    for (int i = 0; i < 4; i++) {
        mDevToMicMaster.push_back(-1);
        mMicMasterToDev.push_back(-1);
        mMicMasters.push_back(nullptr);
        mAssocMicXbox.push_back(nullptr);
    }
}

void ExternalMicClientMgr::Terminate() {
    for (int i = 0; i < mMicMasters.size(); i++) {
        if (mMicMasters[i]) {
            delete mMicMasters[i];
        }
    }
    mMicMasters.clear();
}

void ExternalMicClientMgr::Associate(int idx, MicXbox *mic) { mAssocMicXbox[idx] = mic; }

void ExternalMicClientMgr::AddAudio(DWORD idx, BYTE *data, DWORD dataSize) {
    ExternalMicClientProxy *proxy = GetMasterForIndex(idx);
    if (proxy) {
        proxy->AddAudio(data, dataSize);
    }
}

bool ExternalMicClientMgr::ConnectedForClient(const MicXbox *mic) {
    for (int i = 0; i < mAssocMicXbox.size(); i++) {
        if (mAssocMicXbox[i] == mic && mMicMasters[i] && mMicMasters[i]->IsConnected()) {
            return true;
        }
    }
    return false;
}

void ExternalMicClientMgr::OnMicDisconnected(DWORD idx) {
    ExternalMicClientProxy *proxy = GetMasterForIndex(idx);
    if (proxy) {
        proxy->OnMicDisconnected();
        DWORD dmm = mDevToMicMaster[idx];
        if (dmm != -1) {
            mMicMasterToDev[dmm] = -1;
            mDevToMicMaster[idx] = -1;
        }
    }
}

float ExternalMicClientMgr::GetRequiredGain(DWORD idx) {
    ExternalMicClientProxy *proxy = GetMasterForIndex(idx);
    if (proxy) {
        return proxy->GetRequiredGain();
    }
    return 1;
}

ExternalMicClientProxy *ExternalMicClientMgr::GetMasterForIndex(DWORD idx) {
    DWORD dmm = mDevToMicMaster[idx];
    if (dmm != -1) {
        return mMicMasters[dmm];
    } else {
        for (int i = 0; i < mMicMasters.size(); i++) {
            if (!mMicMasters[i]) {
                mMicMasters[i] = new ExternalMicClientProxy(i);
            }
            if (mMicMasterToDev[i] == -1) {
                mDevToMicMaster[idx] = i;
                mMicMasterToDev[i] = idx;
                return mMicMasters[i];
            }
        }
        return nullptr;
    }
}

#pragma endregion

HRESULT ExternalMicClientProxy::OnMicConnected(DWORD dw, bool b2, const Symbol &s) {
    mConnected = true;
    MicXbox *mic = ExternalMicClientMgr::GetClient(mIndex);
    if (mic) {
        mic->OnMicConnected(dw, b2, s);
        return ERROR_SUCCESS;
    } else {
        return E_UNEXPECTED;
    }
}

void ExternalMicClientProxy::OnMicDisconnected() {
    mConnected = false;
    MicXbox *mic = ExternalMicClientMgr::GetClient(mIndex);
    if (mic) {
        mic->OnMicDisconnected();
    }
}

void ExternalMicClientProxy::AddAudio(BYTE *pSamples, DWORD numBytes) {
    MicXbox *mic = ExternalMicClientMgr::GetClient(mIndex);
    if (mic) {
        mic->AddData(pSamples, numBytes);
    }
}

float ExternalMicClientProxy::GetRequiredGain() {
    MicXbox *mic = ExternalMicClientMgr::GetClient(mIndex);
    if (mic) {
        return mic->GetGain();
    } else {
        return 1;
    }
}
