#include "synth360/ExternalMic.h"
#include "os/Debug.h"
#include "synth360/Mic.h"
#include "xdk/XAPILIB.h"
#include "xdk/XMIC.h"
#include "xdk/xmic/xmic.h"

namespace {
    DWORD ExternalMicThreadEntry(void *v) {
        ExternalMic *mic = (ExternalMic *)v;
        return mic->sampleProcessThread();
    }

    void dataReadyEntry(DWORD, DWORD, XOVERLAPPED *);

    std::vector<ExternalMic *> gMics;
}

std::vector<ExternalMicClientProxy *> ExternalMicClientMgr::mMicMasters;
std::vector<DWORD> ExternalMicClientMgr::mDevToMicMaster;
std::vector<DWORD> ExternalMicClientMgr::mMicMasterToDev;
std::vector<MicXbox *> ExternalMicClientMgr::mAssocMicXbox;

#pragma region ExternalMic

ExternalMic::ExternalMic(DWORD micId)
    : mMicIndex(micId), unk8(false), mConnected(false), mGain(-1) {
    mThread = CreateThread(0, 0, ExternalMicThreadEntry, this, 4, 0);
    MILO_ASSERT(mThread, 0x6a);
    SetThreadPriority(mThread, 15);
    XSetThreadProcessor(mThread, 3);
    ResumeThread(mThread);
}

ExternalMic::~ExternalMic() {
    unk8 = true;
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
    float gainReq = ExternalMicClientMgr::GetRequiredGain(mMicIndex);
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
        if (gMics[i]->mConnected) {
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
        MicXbox *mic = mAssocMicXbox[proxy->unk0];
        if (mic) {
            mic->AddData(data, dataSize);
        }
    }
}

bool ExternalMicClientMgr::ConnectedForClient(const MicXbox *mic) {
    for (int i = 0; i < mAssocMicXbox.size(); i++) {
        if (mAssocMicXbox[i] == mic && mMicMasters[i]->unk0
            && mMicMasters[i]->mConnected) {
            return true;
        }
    }
    return false;
}

void ExternalMicClientMgr::OnMicDisconnected(DWORD idx) {
    ExternalMicClientProxy *proxy = GetMasterForIndex(idx);
    if (proxy) {
        proxy->mConnected = false;
        MicXbox *mic = mAssocMicXbox[proxy->unk0];
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
        MicXbox *mic = mAssocMicXbox[proxy->unk0];
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
    MicXbox *mic = ExternalMicClientMgr::AssociatedMic(unk0);
    if (mic) {
        mic->OnMicConnected(dw, b2, s);
        return 0;
    } else {
        return 0x8000FFFF;
    }
}
