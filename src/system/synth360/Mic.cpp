#include "synth360/Mic.h"
#include "Synth.h"
#include "macros.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "rnddx9/Rnd.h"
#include "synth/FxSend.h"
#include "synth/MicClientMapper.h"
#include "synth/MicManagerInterface.h"
#include "synth/Synth.h"
#include "synth360/ExternalMic.h"
#include "synth360/FxSend.h"
#include "synth360/GainEffect.h"
#include "synth360/HeadsetPlaybackEffect.h"
#include "synth360/Voice.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/XHV2.h"
#include "xdk/xapilibi/xbox.h"
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/xhv2/xhv2.h"
#include <cmath>
#include <cstring>

MicManagerXbox *sInstance;

static float gNoiseThreshold = -10;
static int gNoiseInt = 5; // rename
static float gLowCut = 800;
static float gLocalGain = -3;
static float gRemoteGain = 3;

const static float sFloat1 = 2700.0f;
const static u64 bignumber = 0;
const static float sFloat2 = 1800.0f;

#pragma region ChatReceiver

ChatReceiver::ChatReceiver(IXHV2Engine *engine, int port)
    : mXHV(engine), mPad(port), mEnabled(0), mProcessingActivated(0), mDelay(0), mFB(0),
      mHoldFrames(0), mFramesTalking(0), mNetBuffer(new MemStream(true)) {
    MILO_ASSERT(mXHV, 0x3F2);
}

ChatReceiver::~ChatReceiver() {
    ActivateProcessing(false);
    RELEASE(mNetBuffer);
}

void ChatReceiver::ActivateProcessing(bool b1) {
    if (b1 != mProcessingActivated) {
        mProcessingActivated = b1;
        void *mode = _xhv_voicechat_mode;
        if (b1) {
            HRESULT hr = mXHV->RegisterLocalTalker(mPad);
            DX_ASSERT(hr, 0x40D);
            hr = mXHV->StartLocalProcessingModes(mPad, &mode, 1);
            DX_ASSERT(hr, 0x40E);
        } else {
            HRESULT hr = mXHV->StopLocalProcessingModes(mPad, &mode, 1);
            DX_ASSERT(hr, 0x412);
            hr = mXHV->UnregisterLocalTalker(mPad);
            DX_ASSERT(hr, 0x413);
        }
    }
}

#pragma endregion
#pragma region MicXbox

MicXbox::MicXbox(int, float volume)
    : mRunning(false), mLastType(kDisconnected), mChangeNotify(false), mVoice(0),
      mPlaybackWritePtr(mPlaybackBuffer), mVoiceSpeed(1.0f), mAvgLag(0), mLastLag(0),
      mFxSend(0), mVolume(volume), mMute(false), mRMS(0), mGain(1.0f), mOutputGain(1.0f),
      mSensitivity(1.0f), mLastSamp(0), mDroppedSamples(0), mName("generic_usb"),
      mClipping(false) {
    mRingBufferRecent.Init(0xc00);
    mRingBufferContinuous.Init(0x6000);
    mChatBuffer.reserve(0x1800);
    memset(mPlaybackBuffer, 0, 0x3000);
}

MicXbox::~MicXbox() {
    if (mRunning) {
        Stop();
    }
    RELEASE(mVoice);
}

void MicXbox::Start() {
    if (!mRunning) {
        mPlaybackWritePtr = mPlaybackBuffer;
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
    mFxSend = send;
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
    mAvgLag = mIsWireless ? sFloat2 : sFloat1;
    mLastLag = 0;
    mVoiceSpeed = 1.0f;
    mVoice = new Voice(false, 1, false);
    mVoice->SetSampleRate(48000.0f);
    mVoice->SetData(mPlaybackBuffer, sizeof(mPlaybackBuffer), 0);
    mVoice->SetLoopRegion(0, -1);
    mVoice->SetSend(dynamic_cast<FxSend360 *>(mFxSend));
    mVoice->Start();
    mVoice->SetVolume(0);
}

void MicXbox::StopPlayback() {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    RELEASE(mVoice);
    memset(mPlaybackBuffer, 0, sizeof(mPlaybackBuffer));
}

bool MicXbox::IsPlaying() { return mVoice; }

void MicXbox::ClearBuffers() {
    mRingBufferRecent.Reset();
    mRingBufferContinuous.Reset();
}

short *MicXbox::GetRecentBuf(int &iref) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    mRingBufferRecent.Peek(mOutputBuffer, 0xC00);
    iref = 0x600;
    return mOutputBuffer;
}

short *MicXbox::GetContinuousBuf(int &iref) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    iref = mRingBufferContinuous.Read(mOutputBuffer, 0x6000) / sizeof(short);
    return mOutputBuffer;
}

int MicXbox::GetDroppedSamples() { return mDroppedSamples; }

void MicXbox::OnMicConnected(unsigned long ul, bool b, Symbol const &s) {
    mIsWireless = b;
    mName = s;
    MicManagerXbox::GetInstance()->SetMicsChanged();
}

void MicXbox::OnMicDisconnected() { MicManagerXbox::GetInstance()->SetMicsChanged(); }

bool MicXbox::AddToBuffer(std::vector<short> &buf, void *v, int i1, int *i2) {
    int samps = i1 / 2;
    bool b = false;
    MILO_ASSERT(samps <= buf.capacity(), 0x3ac);

    if (samps + buf.size() > buf.capacity()) {
        if (i2 != nullptr) {
            *i2 = *i2 + buf.size();
        }
        b = true;
        buf.clear();
    }

    int bufSize = buf.size();
    samps = bufSize + samps;
    buf.resize(samps);
    XMemCpy(&buf[bufSize], v, i1);
    return b;
}

void MicXbox::AddData(void *v, int bytes) {
    CritSecTracker t(MicManagerXbox::GetInstance()->CritSec());
    MILO_ASSERT((bytes&1) == 0, 0x344);

    if (mOutputGain != 1.0f) {
        int i = bytes / 2;
        if (0 < i) {
            short *s = (short *)v - 2;
            while (i != 0) {
                float f = floor(*s * mOutputGain + 0.5f);
                i--;

                s++;
            }
        }
        // stuff
    }

    if (mVoice) {
        if (mPlaybackWritePtr < mPlaybackWritePtr + bytes) {
            // is it 0???
            XMemCpy(mPlaybackWritePtr, v, 0);
            XMemCpy(mPlaybackBuffer, v, 0);
        } else {
            XMemCpy(mPlaybackWritePtr, v, bytes);
        }
        // hmmmm
        if (!mVoice->IsPlaying()) {
            mVoice->SetVolume(mVolume);
        }
    }

    AddToBuffer(mChatBuffer, v, bytes, 0);
    mRingBufferRecent.Write(v, bytes);
    mDroppedSamples = mRingBufferContinuous.Write(v, bytes);
}

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
    GainEffect::SetGain(DbToRatio(gRemoteGain));
    return 0;
}

MicManagerXbox::MicManagerXbox()
    : mPlayerUsingUSBChat(-1), mXHV(0), mWorkerThread(0), mMicsChanged(false),
      mPushToTalkPad(-1) {
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
    GainEffect::SetGain(DbToRatio(gRemoteGain));
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
    params.pXAudio2 = TheXboxSynth->GetXAudio2();
    HRESULT hr = XHV2CreateEngine(&params, &mWorkerThread, &mXHV);
    DX_ASSERT(hr, 0xCD);

    auto &headsetSubmixes = TheXboxSynth->GetHeadsetSubmixes();
    if (!headsetSubmixes.empty()) {
        for (int i = 0; i < 4; i++) {
            // idk about the params here...
            HRESULT hr =
                TheXboxSynth->GetHeadsetSubmix(i)->GetEffectParameters(0, &params, 4);
            MILO_ASSERT(SUCCEEDED(hr), 0xd9);

            XAUDIO2_EFFECT_CHAIN chain;
            chain.EffectCount = 0;
            XAUDIO2_EFFECT_DESCRIPTOR descriptor;
            descriptor.InitialState = false;
            descriptor.OutputChannels = 1;
            chain.pEffectDescriptors = &descriptor;

            AddRemoteMic(bignumber, &chain);
        }
    }

    for (int i = 0; i < 16; i++) {
        ChatReceiver *receiver = new ChatReceiver(mXHV, i);
        mChatReceivers[i] = receiver;
    }
}

void MicManagerXbox::RequirePushToTalk(bool req, int pad) {
    CritSecTracker t(&mCritSec);
    if (req) {
        MILO_ASSERT(pad >=0, 0x2c7);
        mPushToTalkPad = pad;
    } else {
        mPushToTalkPad = -1;
    }
}

void MicManagerXbox::AddMic(MicXbox *mic) {
    FOREACH (it, mInterfaces) {
        if (*it == mic) {
            return;
        }
    }
    mInterfaces.push_back(mic);
    mic->SetChangeNotify(true);
}

void MicManagerXbox::RemoveMic(MicXbox *mic) {
    FOREACH (it, mInterfaces) {
        if (*it == mic) {
            mInterfaces.erase(it);
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
    if (mXHV) {
        mXHV->Release();
        mXHV = nullptr;
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

void MicManagerXbox::OnDataReady(
    unsigned long userIndex, void *v, unsigned long ul, int *i
) {
    MILO_ASSERT(userIndex >= 0 && userIndex < 4, 0x18a);
    CritSecTracker tracker(&mCritSec);
    ChatReceiver *receiver = mChatReceivers[userIndex];
    MILO_ASSERT(receiver, 0x191);
    if (receiver->GetEnabled()) {
        if (receiver->GetIXHV2Engine()->IsHeadsetPresent(receiver->GetPad())) {
            if (mPlayerUsingUSBChat == userIndex) {
                mPlayerUsingUSBChat = -1;
            }
        } else {
            if (mPlayerUsingUSBChat == -1) {
                mPlayerUsingUSBChat = userIndex;
            }
            if (mPlayerUsingUSBChat == userIndex
                && (mPushToTalkPad == -1
                    || JoypadGetPadData(mPushToTalkPad)->mButtons % 2 != 0
                    || (bool)(JoypadGetPadData(mPushToTalkPad)->mButtons & 2))) {
                MicClientMapper *mapper = TheSynth->GetMicClientMapper();
                for (int i = 0; i < 4; i++) {
                    MicClientID micClientID(i);
                    int micID = mapper->GetMicIDForClientID(micClientID);
                    if (micID != -1) {
                        MicXbox *mic = (MicXbox *)TheSynth->GetMic(micID);
                        if (mic) {
                            mic->ReadChatBuffer(v, ul);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!receiver->GetIXHV2Engine()->IsSharedMicPresent(receiver->GetPad())) {
        receiver->ProcessChatData(v, ul, i);
    } else {
        MicXbox *mic = (MicXbox *)TheSynth->GetMic(0);
        if (mic) {
            mic->AddData(v, ul);
        }
        *i = 1;
    }
}

void MicManagerXbox::DataReadyCallback(
    unsigned long userIndex, void *v, unsigned long ul, int *i
) {
    MILO_ASSERT(sInstance, 0x183);
    sInstance->OnDataReady(userIndex, v, ul, i);
}

void MicManagerXbox::AddRemoteMic(
    unsigned long long const &xuid, XAUDIO2_EFFECT_CHAIN *PairFX
) {
    GainEffect *gainEffect = new GainEffect();
    // param 2 is not right
    HRESULT registerTalker = mXHV->RegisterRemoteTalker(xuid, 0, PairFX, 0);
    if (registerTalker != 0) {
        MILO_FAIL(
            "File: %s Line: %d Error: %s\n", "Mic.cpp", 0x150, DxRnd::Error(registerTalker)
        );
    }

    // param 2 not right
    HRESULT startRemoteProcessing = mXHV->StartRemoteProcessingModes(xuid, 0, 1);
    if (startRemoteProcessing != 0) {
        MILO_FAIL(
            "File: %s Line: %d Error: %s\n",
            "Mic.cpp",
            0x155,
            DxRnd::Error(startRemoteProcessing)
        );
    }
}

#pragma endregion MicManagerXbox
