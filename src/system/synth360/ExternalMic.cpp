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
    mThread = CreateThread(0, 0, ExternalMicThreadEntry, this, 4, 0);
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

HRESULT ExternalMic::gatherGainAttribs(DWORD dw) {
    if (XMicGetGain(dw, 1, &mMinGain)) {
        return 0x80004005;
    } else if (XMicGetGain(dw, 2, &mMaxGain)) {
        return 0x80004005;
    } else {
        return 0;
    }
}

HRESULT ExternalMic::processGain(DWORD dw) {
    float gainReq = ExternalMicClientMgr::GetRequiredGain(mIndex);
    if (gainReq != mGain) {
        MILO_ASSERT((0.0f <= gainReq) && (gainReq <= 1.0f), 0x26F);
        DWORD hr = XMicSetGain(dw, (mMaxGain - mMinGain) * gainReq + mMinGain, nullptr);
        mGain = gainReq;
        if (hr != 0) {
            return 0x80004005;
        }
    }
    return 0;
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

DWORD ExternalMic::sampleProcessThread() {
    XOVERLAPPED DataRequestOverlapped1;
    XOVERLAPPED DataRequestOverlapped2;
    DWORD dwTransferLength = 0;
    DWORD dwMicrophoneIndex = mIndex | 0x4000000;

    RequestCompletionData MCD1;
    RequestCompletionData MCD2;
    XMICCAPABILITIES capabilities;

    while (!mShutdown) {
        while (!mShutdown) {
            Sleep(10);
            if (XMicGetStatus(dwMicrophoneIndex) == 1) {
                break;
            }
        }
        mIsActive = true;

        if (XMicGetCapabilities(dwMicrophoneIndex, &capabilities) == 0) {
            if (XMicStart(dwMicrophoneIndex, 0x100, &dwTransferLength, 0) == 0
                && SUCCEEDED(gatherGainAttribs(dwMicrophoneIndex))) {
                static Symbol generic_usb("generic_usb");
                Symbol micName = generic_usb;
                DataArray *sysConfig = SystemConfig("synth", "mic_types", "xbox");
                for (int i = 1; i < sysConfig->Size(); i++) {
                    DataArray *configArray = sysConfig->Array(i);
                    DataArray *capabilitiesArray = configArray->FindArray("capabilities");
                    if (capabilitiesArray->Int(1) == capabilities.dwFeatures) {
                        if (capabilitiesArray->Int(2) == capabilities.wFormatTag) {
                            if (capabilitiesArray->Int(3) == capabilities.nChannels) {
                                if (capabilitiesArray->Int(4)
                                    == capabilities.dwSampleRatesSupported) {
                                    if (capabilitiesArray->Int(5)
                                        == capabilities.wBitsPerSample) {
                                        if (capabilitiesArray->Int(6)
                                            == capabilities.dwFrameLength) {
                                            if (configArray->FindArray("min_gain")
                                                    ->Float(1)
                                                == mMinGain) {
                                                DataArray *maxGainArray =
                                                    configArray->FindArray("max_gain");
                                                if (maxGainArray->Float(1) == mMaxGain) {
                                                    micName = maxGainArray->Sym(0);
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                ExternalMicClientProxy *proxy =
                    ExternalMicClientMgr::GetMasterForIndex(mIndex);

                if (proxy) {
                    HRESULT hr = proxy->OnMicConnected(
                        0x100, capabilities.dwFeatures == 0, micName
                    );
                    if (SUCCEEDED(hr)) {
                        MCD1.pOverlap = &DataRequestOverlapped1;
                        MCD1.dwAudioFrameBufferLength = dwTransferLength;
                        MCD1.dwFrameCount = capabilities.dwFeatures + 1; // idk
                        MCD1.dwMicrophoneIndex = dwMicrophoneIndex;
                        MCD1.pAudioFramesBufferAlloc =
                            new BYTE[dwTransferLength + 0x100]; // idk
                        MCD1.pAudioFramesBuffer = MCD1.pAudioFramesBufferAlloc + 0x80;
                        MCD2.pOverlap = &DataRequestOverlapped2;
                        MCD2.dwAudioFrameBufferLength = dwTransferLength;
                        MCD1.pOwner = this;
                        MCD2.dwFrameCount = capabilities.dwFeatures + 1; // idk
                        MCD2.dwMicrophoneIndex = dwMicrophoneIndex;
                        MCD2.pAudioFramesBufferAlloc =
                            new BYTE[dwTransferLength + 0x100]; // idk
                        DataRequestOverlapped1.dwExtendedError = 0;
                        DataRequestOverlapped1.pCompletionRoutine = dataReadyEntry;
                        MCD2.pAudioFramesBuffer = MCD2.pAudioFramesBufferAlloc + 0x80;
                        // DataRequestOverlapped1.dwCompletionContext = &MCD1; what
                        DataRequestOverlapped1.hEvent = 0;
                        DataRequestOverlapped2.dwExtendedError = 0;
                        // DataRequestOverlapped2.dwCompletionContext = &MCD2; what
                        DataRequestOverlapped2.hEvent = 0;
                        DataRequestOverlapped2.pCompletionRoutine = dataReadyEntry;
                        MCD2.pOwner = this;

                        if (XMicRequestData(
                                MCD1.dwMicrophoneIndex,
                                MCD1.dwFrameCount,
                                MCD1.pAudioFramesBuffer,
                                MCD1.wFrameLengths,
                                MCD1.pOverlap
                            )
                            != ERROR_IO_PENDING) {
                            hr = -0x7fffbffb;
                        }

                        if (XMicRequestData(
                                MCD2.dwMicrophoneIndex,
                                MCD2.dwFrameCount,
                                MCD2.pAudioFramesBuffer,
                                MCD2.wFrameLengths,
                                MCD2.pOverlap
                            )
                            != ERROR_IO_PENDING) {
                            hr = -0x7fffbffb;
                        }

                        // legit no idea what all this is
                        if (SUCCEEDED(hr)) {
                            int status;
                            do {
                                if (mShutdown) {
                                    break;
                                }
                                SleepEx(10, 1);
                                hr = processGain(dwMicrophoneIndex);
                                MILO_ASSERT(SUCCEEDED(hr), 0x149);
                                status = XMicGetStatus(dwMicrophoneIndex);
                            } while (status == 2);
                        }
                    }
                }
            }
        }

        XMicStop(dwMicrophoneIndex, 0);
        mIsActive = false;
        ExternalMicClientMgr::OnMicDisconnected(mIndex);
        mGain = -1.0f;
        if (MCD1.pAudioFramesBufferAlloc) {
            delete MCD1.pAudioFramesBufferAlloc;
            MCD1.pAudioFramesBuffer = 0;
            MCD1.pAudioFramesBufferAlloc = 0;
        }
        if (MCD2.pAudioFramesBufferAlloc) {
            delete MCD2.pAudioFramesBufferAlloc;
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
        MicXbox *mic = mAssocMicXbox[proxy->GetIndex()];
        if (mic) {
            mic->AddData(data, dataSize);
        }
    }
}

bool ExternalMicClientMgr::ConnectedForClient(const MicXbox *mic) {
    for (int i = 0; i < mAssocMicXbox.size(); i++) {
        if (mAssocMicXbox[i] == mic && mMicMasters[i]->GetIndex()
            && mMicMasters[i]->IsConnected()) {
            return true;
        }
    }
    return false;
}

void ExternalMicClientMgr::OnMicDisconnected(DWORD idx) {
    ExternalMicClientProxy *proxy = GetMasterForIndex(idx);
    if (proxy) {
        proxy->SetConnected(false);
        MicXbox *mic = mAssocMicXbox[proxy->GetIndex()];
        if (mic) {
            mic->OnMicDisconnected();
        }
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
        MicXbox *mic = mAssocMicXbox[proxy->GetIndex()];
        if (mic) {
            return mic->GetGain();
        }
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
    MicXbox *mic = ExternalMicClientMgr::AssociatedMic(mIndex);
    if (mic) {
        mic->OnMicConnected(dw, b2, s);
        return 0;
    } else {
        return 0x8000FFFF;
    }
}
