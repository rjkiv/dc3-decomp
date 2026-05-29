#include "synth_xbox/Mic.h"
#include "macros.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Rnd.h"
#include "synth/FxSend.h"
#include "synth_xbox/ExternalMic.h"
#include "synth_xbox/FxSend.h"
#include "synth_xbox/Voice.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/xvh2/xvh2.h"
#include <cstring>

MicManagerXbox *sInstance;

static float gNoiseThreshold = -10;
static int gNoiseInt = 5; // rename
static float gLowCut = 800;
static float gLocalGain = -3;
static float gRemoteGain = 3;

#pragma region ChatReceiver

ChatReceiver::ChatReceiver(IXHV2Engine *engine, int i2)
    : mXHV(engine), unk4(i2), unk8(0), unk9(0), unkc(0), unk10(0), unk14(0), unk18(0),
      unk50(new MemStream(true)) {
    MILO_ASSERT(mXHV, 0x3F2);
}

ChatReceiver::~ChatReceiver() {
    ActivateProcessing(false);
    RELEASE(unk50);
}

void ChatReceiver::ActivateProcessing(bool b1) {
    if (b1 != unk9) {
        unk9 = b1;
        void *mode = _xhv_voicechat_mode;
        if (b1) {
            HRESULT hr = mXHV->RegisterLocalTalker(unk4);
            DX_ASSERT_CODE(hr, 0x40D);
            hr = mXHV->StartLocalProcessingModes(unk4, &mode, 1);
            DX_ASSERT_CODE(hr, 0x40E);
        } else {
            HRESULT hr = mXHV->StopLocalProcessingModes(unk4, &mode, 1);
            DX_ASSERT_CODE(hr, 0x412);
            hr = mXHV->UnregisterLocalTalker(unk4);
            DX_ASSERT_CODE(hr, 0x413);
        }
    }
}

#pragma endregion
#pragma region MicXbox

MicXbox::MicXbox(int, float volume)
    : mRunning(false), unk10(0), mChangeNotify(false), mVoice(0), unk301c(mVoiceBuffer),
      unk9054(1.0f), unk9058(0), unk905c(0), mSend(0), mVolume(volume), mMute(false),
      unk906c(0), mGain(1.0f), mOutputGain(1.0f), mSensitivity(1.0f), unk907c(0),
      mDroppedSamples(0), mName("generic_usb"), mClipping(false) {
    mRingBufferRecent.Init(0xc00);
    mRingBufferContinuous.Init(0x6000);
    unk3020.reserve(0x1800);
    memset(mVoiceBuffer, 0, 0x3000);
}

MicXbox::~MicXbox() {
    if (mRunning) {
        Stop();
    }
    RELEASE(mVoice);
}

void MicXbox::Start() {
    if (!mRunning) {
        unk301c = mVoiceBuffer;
        MicManagerXbox::GetInstance()->AddMic(this);
        mRunning = true;
    }
}

void MicXbox::Stop() {
    if (mRunning) {
        MicManagerXbox::GetInstance()->RemoveMic(this);
        mRunning = false;
        if (mVoice) {
            StopPlayback();
        }
    }
}

Mic::Type MicXbox::GetType() const {
    return ExternalMicClientMgr::ConnectedForClient(this) ? kUSBMic : kDisconnected;
}

void MicXbox::SetGain(float gain) { mGain = Clamp(0.0f, 1.0f, gain); }
float MicXbox::GetGain() const { return mGain; }
void MicXbox::SetMute(bool b) { mMute = b; }
bool MicXbox::GetClipping() const { return mClipping; }

void MicXbox::SetOutputGain(float f) {
    mOutputGain = f;
    MILO_ASSERT(mOutputGain >= 0.0f, 0x32c);
}

float MicXbox::GetOutputGain() const { return mOutputGain; }

void MicXbox::SetSensitivity(float f) {
    mSensitivity = f;
    MILO_ASSERT(mOutputGain >= 0.0f, 0x337);
}

float MicXbox::GetSensitivity() const { return mSensitivity; }
void MicXbox::SetVolume(float f) { mVolume = DbToRatio(f); }

void MicXbox::SetFxSend(FxSend *send) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    mSend = send;
    if (mVoice) {
        StopPlayback();
        StartPlayback();
    }
}

void MicXbox::SetChangeNotify(bool b) { mChangeNotify = b; }

void MicXbox::StartPlayback() {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    if (mVoice) {
        return;
    }
    Start();
    mMute = false;
    if (unkc) {
        unk9058 = 2700;
    } else {
        unk9058 = 1800;
    }
    unk905c = 0;
    unk9054 = 1;
    mVoice = new Voice(false, 1, false);
    mVoice->SetSampleRate(48000);
    mVoice->SetData(mVoiceBuffer, sizeof(mVoiceBuffer), 0);
    mVoice->SetLoopRegion(0, -1);
    mVoice->SetSend(dynamic_cast<FxSend360 *>(mSend));
    mVoice->Start();
    mVoice->SetVolume(0);
}

void MicXbox::StopPlayback() {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    RELEASE(mVoice);
    memset(mVoiceBuffer, 0, sizeof(mVoiceBuffer));
}

bool MicXbox::IsPlaying() { return mVoice; }

void MicXbox::ClearBuffers() {
    mRingBufferRecent.Reset();
    mRingBufferContinuous.Reset();
}

short *MicXbox::GetRecentBuf(int &iref) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    mRingBufferRecent.Peek(unk3054, 0xC00);
    iref = 0x600;
    return unk3054;
}

short *MicXbox::GetContinuousBuf(int &iref) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    iref = mRingBufferContinuous.Read(unk3054, 0x6000) / sizeof(short);
    return unk3054;
}

int MicXbox::GetDroppedSamples() { return mDroppedSamples; }

void MicXbox::OnMicConnected(unsigned long ul, bool b, Symbol const &s) {
    unkc = b;
    mName = s;
    MicManagerXbox::GetInstance()->SetMicsChanged();
}

void MicXbox::OnMicDisconnected() { MicManagerXbox::GetInstance()->SetMicsChanged(); }

#pragma endregion MicXbox
#pragma region MicManagerXbox

static DataNode SetNoiseGate(DataArray *a) {
    gNoiseThreshold = a->Float(1);
    if (a->Size() >= 3) {
        gNoiseInt = a->Int(2);
    }
    return 0;
}

static DataNode SetLowCut(DataArray *a) {
    gLowCut = a->Float(1);
    return 0;
}

static DataNode SetLocalGain(DataArray *a) {
    gLocalGain = a->Float(1);
    return 0;
}

static DataNode SetRemoteGain(DataArray *a) {
    gRemoteGain = a->Float(1);
    return 0;
}

MicManagerXbox::MicManagerXbox()
    : unk18(-1), mXHVEngine(0), mXHVWorkerThread(0), mMicsChanged(false), mPad(-1) {
    for (int i = 4; i != 0; i--) {
        mChatReceivers.push_back(nullptr);
    }
    mChatBuffers.reserve(4);
    DataRegisterFunc("set_noise_gate", SetNoiseGate);
    DataRegisterFunc("set_low_cut", SetLowCut);
    DataRegisterFunc("set_local_gain", SetLocalGain);
    DataRegisterFunc("set_remote_gain", SetRemoteGain);
    DataArray *synthConfig = SystemConfig("synth", "xbox_headset");
    synthConfig->FindData("noise_threshold", gNoiseThreshold);
    synthConfig->FindData("low_cut", gLowCut);
    synthConfig->FindData("local_gain", gLocalGain);
    synthConfig->FindData("remote_gain", gRemoteGain);
    //  GainEffect::sGain = DbToRatio(gRemoteGain);
}

MicManagerXbox::~MicManagerXbox() {}

void MicManagerXbox::Init() {
    MILO_ASSERT(this == sInstance, 0xB8);

    XHV_INIT_PARAMS params;
    params.dwMaxLocalTalkers = 4;
    params.localTalkerEnabledModes = &_xhv_voicechat_mode;
    params.remoteTalkerEnabledModes = &_xhv_loopback_mode;
    params.dwNumRemoteTalkerEnabledModes = 1;
    params.dwMaxRemoteTalkers = 5;
    params.dwNumLocalTalkerEnabledModes = 2;
    params.pfnMicrophoneRawDataReady = DataReadyCallback;
    params.bCustomVADProvided = true;
    params.bRelaxPrivileges = true;
    HRESULT hr = XHV2CreateEngine(&params, &mXHVWorkerThread, &mXHVEngine);
    DX_ASSERT_CODE(hr, 0xCD);
}

void MicManagerXbox::RequirePushToTalk(bool req, int pad) {
    CritSecTracker t(&mMicArrayLock);
    if (req) {
        MILO_ASSERT(pad >=0, 0x2c7);
        mPad = pad;
    } else {
        mPad = -1;
    }
}

void MicManagerXbox::AddMic(MicXbox *mic) {
    FOREACH (it, mMics) {
        if (*it == mic) {
            return;
        }
    }
    mMics.push_back(mic);
    mic->SetChangeNotify(true);
}

void MicManagerXbox::RemoveMic(MicXbox *mic) {
    FOREACH (it, mMics) {
        if (*it == mic) {
            mMics.erase(it);
            mic->SetChangeNotify(false);
            return;
        }
    }
}

void MicManagerXbox::Shutdown() {
    MILO_ASSERT(this == sInstance, 0xF0);
    for (int i = 0; i < 4; i++) {
        RELEASE(mChatReceivers[i]);
    }
    if (mXHVEngine) {
        mXHVEngine->Release();
        mXHVEngine = nullptr;
    }
    sInstance = nullptr;
    delete this;
}

MicManagerXbox *MicManagerXbox::GetInstance() {
    if (!sInstance) {
        sInstance = new MicManagerXbox();
    }
    return sInstance;
}

#pragma endregion MicManagerXbox
