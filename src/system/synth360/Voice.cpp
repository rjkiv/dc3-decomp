#include "synth360/Voice.h"
#include "math/Utl.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth360/EnvelopeGenerator.h"
#include "synth360/Synth.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/sysinfoapi.h"
#include "xdk/xapilibi/winbase.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xaudio2/xapobase.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cmath>
#include <list>
#include <deque>

HANDLE gEvent = INVALID_HANDLE_VALUE;
HANDLE gVoiceThread = INVALID_HANDLE_VALUE;
float s_MaxVoiceFreqRatio = 4;

bool gShutdownVoiceThread = false;
bool gHasPendingStopCommits = false;
bool gCommitSyncVoices = false;
bool gWasCommitSyncVoices = false;
static int s_TotalInUseVoices = 0;
static int s_TotalGCVoices = 0;
int gCommitTag = 0;
int gWasCommitTag = 0;
int rolling = 0;

CriticalSection gLockPendingLists;
CriticalSection gVoiceGC;
std::list<Voice *> gPendingVoices;
std::list<Voice *> gPendingSyncVoices;
std::list<Voice *> gInProgressVoices;
std::list<Voice *> gInProgressSyncVoices;
std::deque<PoolVoice> s_voiceGC;
std::deque<PoolVoice> s_voiceGCInProgress;

int Voice::sHeadsetTarget = -1;

DWORD StartVoiceThreadEntry(void *);

void StartSynchronizedVoices() {
    if (!gShutdownVoiceThread) {
        CritSecTracker t(&gLockPendingLists);
        gCommitSyncVoices = true;
        gCommitTag = 1;
        if (gEvent != INVALID_HANDLE_VALUE) {
            SetEvent(gEvent);
        }
    }
}

void StopSynchronizedVoices() {
    if (!gShutdownVoiceThread && gHasPendingStopCommits) {
        CritSecTracker t(&gLockPendingLists);
        gHasPendingStopCommits = false;
        gCommitSyncVoices = true;
        gCommitTag = 2;
        if (gEvent != INVALID_HANDLE_VALUE) {
            SetEvent(gEvent);
        }
    }
}

void TerminateVoiceThread() {
    gShutdownVoiceThread = true;
    if (gEvent != INVALID_HANDLE_VALUE) {
        SetEvent(gEvent);
    }
    if (gVoiceThread != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(gVoiceThread, 500);
        CloseHandle(gVoiceThread);
    }
}

Voice::Voice(bool xma, int chans, bool b2)
    : mState(kUninitialized), mBuffer(0), mBufferBytes(0), mNumSamples(0), mSampleRate(0),
      mStartSamp(0), mLoopStart(-1), mLoopEnd(-1), mVolume(1.0f), mPan(0), mSpeed(1.0f),
      mAttackRate(0.001f), mReleaseRate(0.001f), mXMA(xma), mSend(0),
      mReverbEnable(false), mReverbMixDb(-96.0f), mReverbVoiceConnected(false),
      mCoherentSet(b2), mChannels(chans), mTag(0), mHasOutputVoices(false) {
    mPoolVoice.eg = 0;
    mPoolVoice.egParams = 0;
    mPoolVoice.xVoice = 0;
    if (gEvent == INVALID_HANDLE_VALUE) {
        gEvent = CreateEventA(0, 0, 0, 0);
        MILO_ASSERT(gEvent, 0xfa);
        gVoiceThread = CreateThread(0, 0x10000, StartVoiceThreadEntry, 0, 4, 0);
        MILO_ASSERT(gVoiceThread, 0xff);
        SetThreadPriority(gVoiceThread, 0xf);
        DWORD ret = XSetThreadProcessor(gVoiceThread, 2);
        MILO_ASSERT(ret != -1, 0x107);
        ret = ResumeThread(gVoiceThread);
        MILO_ASSERT(ret != -1, 0x10c);
    }
}

Voice::~Voice() {
    while (mState == kPendingStart) {
        if (mCoherentSet) {
            StartSynchronizedVoices();
        }
        Sleep(0);
    }
    if (mSend) {
        mSend->RemoveOwnerVoice(this);
    }
    if (GetVoice()) {
        GetVoice()->Stop(0, 0);
        dispose(&mPoolVoice, mFmtHash);
    }
}

IXAudio2Voice *Voice::OutputVoice() {
    if (mSend) {
        return mSend->InputVoice();
    } else {
        return TheXboxSynth->GetMasteringVoice();
    }
}

void Voice::Init(bool b1) {
    if (TheXboxSynth->GetMasteringVoice()) {
        if (!b1) {
            mState = kStopped;
        }
        MILO_ASSERT(0 < mSampleRate && mSampleRate <= 48000, 0x160);
        MILO_ASSERT(mBuffer, 0x161);
        if (mSend && !mSend->Initialized()) {
            FxSend *send = dynamic_cast<FxSend *>(mSend);
            send->RebuildChain();
        }
        XAUDIO2_SEND_DESCRIPTOR desc;
        desc.Flags = 0;
        desc.pOutputVoice = OutputVoice();
        std::vector<XAUDIO2_SEND_DESCRIPTOR> descs;
        if (desc.pOutputVoice) {
            descs.push_back(desc);
        }
        if (mReverbEnable) {
            XAUDIO2_SEND_DESCRIPTOR desc2;
            desc2.Flags = 0;
            desc2.pOutputVoice = TheXboxSynth->GetReverbVoice();
            descs.push_back(desc2);
            mReverbVoiceConnected = true;
        }
        XAUDIO2_SEND_DESCRIPTOR desc3;
        desc3.Flags = 0;
        desc3.pOutputVoice = TheXboxSynth->GetHeadsetSubmix(sHeadsetTarget);
        if (desc3.pOutputVoice) {
            descs.push_back(desc3);
        }
        XAUDIO2_VOICE_SENDS sends;
        sends.SendCount = descs.size();
        sends.pSends = sends.SendCount ? &descs.front() : nullptr;
        XAUDIO2_BUFFER xbuffer;
        InitSourceBuffer(xbuffer);
        XMA2WAVEFORMATEX fmtex;
        InitVoiceParameters(fmtex, xbuffer);
        MILO_ASSERT(!GetVoice(), 0x194);

        XAUDIO2_VOICE_SENDS *sendsPtr = nullptr;
        if (sends.SendCount) {
            sendsPtr = &sends;
        }
        HRESULT hr = createOrReuse(&mPoolVoice, mFmtHash, fmtex.wfx, sendsPtr);

        MILO_ASSERT(SUCCEEDED(hr), 0x19D);
        MILO_ASSERT(GetVoice(), 0x19E);
        hr = GetVoice()->SubmitSourceBuffer(&xbuffer, nullptr);
        MILO_ASSERT(SUCCEEDED(hr), 0x1A3);
        UpdateMix();
        if (GetVoice()) {
            GetVoice()->SetFrequencyRatio(mSpeed, 0);
        }
        mPoolVoice.egParams->AttackRate = mAttackRate;
        mPoolVoice.egParams->ReleaseRate = mReleaseRate;
        mPoolVoice.egParams->StartRelease = 0;
        mPoolVoice.egParams->IsDone = 0;
        hr = GetVoice()->SetEffectParameters(
            0, mPoolVoice.egParams, sizeof(EnvelopeGeneratorParams), 0
        );
        MILO_ASSERT(SUCCEEDED(hr), 0x1B0);
    }
}

void Voice::blockingStart(bool b1) {
    if (!gShutdownVoiceThread && TheXboxSynth->GetMasteringVoice()) {
        CritSecTracker t(&gLockPendingLists);
        Init(b1);
        HRESULT hr = GetVoice()->Start(0, mCoherentSet != false);
        MILO_ASSERT(SUCCEEDED(hr), 0x29B);
        mState = kPlaying;
    }
}

void Voice::Pause(bool b1) {
    if (b1 != (mState == kPaused) && IsPlaying()) {
        if (mCoherentSet && mState == kPendingStart && b1) {
            StartSynchronizedVoices();
        }
        while (mState == kPendingStart) {
            Sleep(0);
        }
        MILO_ASSERT(GetVoice(), 0x2B4);
        if (b1) {
            gHasPendingStopCommits = true;
            HRESULT hr = GetVoice()->Stop(0, mCoherentSet ? 2 : 0);
            MILO_ASSERT(SUCCEEDED(hr), 700);
            mState = kPaused;
        } else {
            SafeRestart();
        }
    }
}

void Voice::SafeRestart() {
    MILO_ASSERT(GetVoice(), 0x471);
    GetVoice()->Start(0, mCoherentSet != false);
    mState = kPlaying;
}

void Voice::Stop(bool b1) {
    if (GetVoice()) {
        if (b1) {
            GetVoice()->Stop(0, 0);
        } else {
            MILO_ASSERT(mPoolVoice.egParams, 0x14D);
            mPoolVoice.egParams->StartRelease = 1;
            HRESULT hr = GetVoice()->SetEffectParameters(
                0, mPoolVoice.egParams, sizeof(EnvelopeGeneratorParams), 0
            );
            MILO_ASSERT(SUCCEEDED(hr), 0x150);
        }
    }
    mState = kStopped;
}

void Voice::SetSampleRate(int i) {
    mSampleRate = i;
    MILO_ASSERT(0 < mSampleRate && mSampleRate <= 48000, 0x2c9);
}

void Voice::SetLoopRegion(int loopStart, int loopEnd) {
    MILO_ASSERT_RANGE(loopStart, 0, mNumSamples, 0x2cf);
    MILO_ASSERT(loopEnd == -1 || loopEnd > loopStart, 0x2d0);
    mLoopStart = loopStart;
    mLoopEnd = loopEnd;
}

void Voice::SetReverbEnable(bool b) {
    if (mReverbEnable != b) {
        mReverbEnable = b;
        UpdateSends();
    }
}

void Voice::SetVolume(float f) {
    if (f != mVolume) {
        mVolume = f;
        if (4.0f < f) {
            MILO_NOTIFY("A gain of %f is rather loud", mVolume);
            mVolume = 4.0f;
        }
        UpdateMix();
    }
}

void Voice::SetPan(float f) {
    float mod = Mod(f - -4.0f, 8.0f);
    if (mod - 4.0f != mPan) {
        mPan = mod - 4.0f;
        UpdateMix();
    }
}

void Voice::SetStartSamp(int samp) {
    MILO_ASSERT(samp >= 0, 0x31e);
    MILO_ASSERT(samp < mNumSamples, 799);
    mStartSamp = samp;
}

void Voice::SetReverbMixDb(float f) {
    mReverbMixDb = f;
    UpdateMix();
}

void Voice::EndLoop() {
    HRESULT hr = GetVoice()->ExitLoop(0);
    MILO_ASSERT(SUCCEEDED(hr), 0x2da);
}

void Voice::Start() { blockingStart(false); }

void Voice::SetData(const void *buffer, int bytes, int i) {
    MILO_ASSERT(buffer, 299);
    MILO_ASSERT(bytes >= 0, 300);
    mBuffer = buffer;
    mBufferBytes = bytes;
    if (i != 0) {
        mNumSamples = i;
    } else {
        MILO_ASSERT(!mXMA, 0x136);
        mNumSamples = bytes / 2;
        if (1 < mChannels) {
            MILO_ASSERT((mNumSamples & (mChannels)) == 0, 0x13a);
            mNumSamples = mNumSamples / mChannels;
        }
    }
}

void Voice::SetSpeed(float f1) {
    float speed = std::max(0.01f, f1);
    if (speed > 2 && mXMA) {
        MILO_NOTIFY_ONCE("can't pitch an XMA sound up more than one octave");
        speed = 2;
    }
    mSpeed = speed;
    if (GetVoice()) {
        GetVoice()->SetFrequencyRatio(mSpeed, 0);
    }
}

void Voice::InitSourceBuffer(XAUDIO2_BUFFER &audio_buffer) {
    audio_buffer.pAudioData = (BYTE *)mBuffer;
    audio_buffer.AudioBytes = mBufferBytes;
    audio_buffer.pContext = 0;
    audio_buffer.PlayBegin = mStartSamp;
    audio_buffer.PlayLength = 0;
    if (mLoopStart >= 0) {
        if (mLoopEnd < 0) {
            mLoopEnd = mNumSamples;
        }
        if (mXMA) {
            mLoopStart -= (mLoopStart % 128);
            mLoopEnd -= (mLoopEnd % 128);
        }
        audio_buffer.LoopCount = 0xff;
        audio_buffer.LoopBegin = mLoopStart;
        audio_buffer.LoopLength = mLoopEnd - mLoopStart;
    } else {
        audio_buffer.LoopBegin = 0;
        audio_buffer.LoopCount = 0;
        audio_buffer.LoopLength = 0;
    }
    audio_buffer.Flags = 0x40;
}

void Voice::SetSend(FxSend360 *send) {
    if (mSend != send) {
        SetSendImpl(send);
    }
}

void Voice::SetSendImpl(FxSend360 *send) {
    if (mSend) {
        mSend->RemoveOwnerVoice(this);
    }
    if (send) {
        send->AddOwnerVoice(this);
    }
    mSend = send;
    UpdateSends();
}

bool Voice::HasPendingVoices() {
    if (gShutdownVoiceThread) {
        return false;
    } else {
        CritSecTracker t(&gLockPendingLists);
        return gPendingVoices.size() + gPendingSyncVoices.size() != 0;
    }
}

bool Voice::IsPlaying() {
    START_AUTO_TIMER("voice_is_playing");
    if (mState == kPendingStart) {
        return true;
    } else if (!GetVoice()) {
        return false;
    } else if (mState == kStopped) {
        return false;
    } else if (mState == kPaused) {
        return true;
    } else {
        XAUDIO2_VOICE_STATE voiceState;
        GetVoice()->GetState(&voiceState, 0);
        if (voiceState.BuffersQueued == 0 && voiceState.SamplesPlayed == 0) {
            return false;
        } else {
            EnvelopeGeneratorParams params;
            params.IsDone = 0;
            if (TheXboxSynth->GetVoiceDestructionCS()->TryEnter()) {
                HRESULT hr = GetVoice()->GetEffectParameters(0, &params, sizeof(params));
                TheXboxSynth->GetVoiceDestructionCS()->Exit();
                MILO_ASSERT(SUCCEEDED(hr), 0x2ff);
            }
            return params.IsDone == 0;
        }
    }
}

void Voice::dispose(PoolVoice *pPoolVoice, unsigned int fmtHash) {
    pPoolVoice->xVoice->FlushSourceBuffers();
    if (mHasOutputVoices) {
        XAUDIO2_VOICE_SENDS sends = { 0, nullptr };
        pPoolVoice->xVoice->SetOutputVoices(&sends);
        mHasOutputVoices = false;
    }
    pPoolVoice->tickCount = GetTickCount() - 500000;
    {
        CritSecTracker cst(&gVoiceGC);
        s_voiceGC.push_back(*pPoolVoice);
        s_TotalGCVoices++;
        s_TotalInUseVoices--;
    }
    pPoolVoice->eg = nullptr;
    pPoolVoice->egParams = nullptr;
    pPoolVoice->xVoice = nullptr;
}

HRESULT Voice::createOrReuse(
    PoolVoice *pPoolVoice,
    unsigned int &fmtHash,
    WAVEFORMATEX &format,
    XAUDIO2_VOICE_SENDS *pSends
) {
    if (!TheXboxSynth->GetMasteringVoice()) {
        return ERROR_SUCCESS;
    } else {
        MILO_ASSERT(pPoolVoice->eg == 0, 0x1C1);
        pPoolVoice->eg = new EnvelopeGenerator();
        MILO_ASSERT(mPoolVoice.egParams == 0, 0x1C3);
        pPoolVoice->egParams = new EnvelopeGeneratorParams;

        XAUDIO2_EFFECT_DESCRIPTOR fxi;
        fxi.pEffect = static_cast<CXAPOBase *>(pPoolVoice->eg);
        fxi.InitialState = false;
        fxi.OutputChannels = mChannels;
        XAUDIO2_EFFECT_CHAIN effectChain;
        effectChain.EffectCount = 1;
        effectChain.pEffectDescriptors = &fxi;
        MemDoTempAllocations t;
        {
            CritSecTracker cst(TheXboxSynth->GetVoiceDestructionCS());
            HRESULT hr = TheXboxSynth->GetXAudio2()->CreateSourceVoice(
                &pPoolVoice->xVoice,
                &format,
                0,
                s_MaxVoiceFreqRatio,
                nullptr,
                pSends,
                &effectChain
            );
            if (hr != ERROR_SUCCESS) {
                MEMORYSTATUS status;
                char buffer[2048] = { 0 };
                GlobalMemoryStatus(&status);
                Hx_snprintf(
                    buffer,
                    sizeof(buffer),
                    "XAudio2: CreateSourceVoice failed with 0x%X\n",
                    hr
                );
                MemPrintOverview(-3, buffer + strlen(buffer));
                MILO_FAIL(buffer);
                return hr;
            }
        }
        s_TotalInUseVoices++;
        pPoolVoice->wav = format;
        mHasOutputVoices = !pSends || pSends->SendCount > 0;
        return ERROR_SUCCESS;
    }
}

int Voice::GetAddr() {
    if (GetVoice() && !mXMA) {
        XAUDIO2_VOICE_STATE st;
        GetVoice()->GetState(&st, 0);
        unsigned int i4 = mStartSamp + st.SamplesPlayed;
        if (mBuffer) {
            unsigned int i1 = mBufferBytes / (mChannels * 2);
            i4 = (i4 % i1) * mChannels;
        } else {
            i4 *= mChannels;
        }
        return i4 * 2;
    } else {
        return 0;
    }
}

void Voice::InitVoiceParameters(XMA2WAVEFORMATEX &xma2Format, XAUDIO2_BUFFER buffer) {
    if (mXMA) {
        xma2Format.wfx.wFormatTag = 0x166;
        xma2Format.wfx.nChannels = mChannels;
        xma2Format.wfx.nSamplesPerSec = mSampleRate;
        xma2Format.wfx.wBitsPerSample = 16;
        xma2Format.wfx.cbSize = 0x22;
        xma2Format.NumStreams = 1;
        xma2Format.wfx.nBlockAlign =
            (xma2Format.wfx.nChannels * xma2Format.wfx.wBitsPerSample) / 8;
        if (mChannels == 1) {
            xma2Format.ChannelMask = 4;
        } else if (mChannels == 2) {
            xma2Format.ChannelMask = 3;
        } else if (mChannels == 5) {
            xma2Format.ChannelMask = 0x60F;
        }
        xma2Format.SamplesEncoded = mNumSamples;
        xma2Format.PlayBegin = buffer.PlayBegin;
        xma2Format.PlayLength = buffer.PlayLength;
        xma2Format.BytesPerBlock = 0x10000;
        xma2Format.LoopBegin = buffer.LoopBegin;
        xma2Format.LoopLength = buffer.LoopLength;
        xma2Format.EncoderVersion = 4;
        xma2Format.LoopCount = buffer.LoopCount;
        xma2Format.BlockCount = ceilf(mBufferBytes / 65536.0f);
    } else {
        xma2Format.wfx.wFormatTag = 1;
        xma2Format.wfx.nChannels = mChannels;
        xma2Format.wfx.nSamplesPerSec = mSampleRate;
        xma2Format.wfx.wBitsPerSample = 16;
        xma2Format.wfx.nBlockAlign =
            (xma2Format.wfx.nChannels * xma2Format.wfx.wBitsPerSample) / 8;
        xma2Format.wfx.nAvgBytesPerSec =
            xma2Format.wfx.nBlockAlign * xma2Format.wfx.nSamplesPerSec;
        xma2Format.wfx.cbSize = 0;
    }
}

void Voice::UpdateSends() {
    if (GetVoice()) {
        XAUDIO2_SEND_DESCRIPTOR desc;
        desc.Flags = 0;
        desc.pOutputVoice = OutputVoice();

        XAUDIO2_VOICE_SENDS sends = { 1, &desc };

        if (mReverbEnable) {
            // FIXME: i called this "lmao" because it's gonna get destroyed
            // once this branch goes out of scope
            // this can't be what hmx wrote right? surely someone would've noticed this
            XAUDIO2_SEND_DESCRIPTOR lmao[2];
            lmao[0].Flags = 0;
            lmao[0].pOutputVoice = TheXboxSynth->GetReverbVoice();
            sends.SendCount = 1;
            sends.pSends = lmao;
            if (OutputVoice()) {
                sends.SendCount = 2;
            }
        }

        HRESULT hr;
        if (!desc.pOutputVoice && !mReverbEnable) {
            XAUDIO2_VOICE_SENDS nullSendList = { 0, nullptr };
            hr = GetVoice()->SetOutputVoices(&nullSendList);
            mHasOutputVoices = false;
            mReverbVoiceConnected = false;
        } else {
            hr = GetVoice()->SetOutputVoices(&sends);
            mHasOutputVoices = true;
            if (mReverbEnable) {
                mReverbVoiceConnected = true;
            }
        }
        MILO_ASSERT(SUCCEEDED(hr), 0x462);
        UpdateMix();
    }
}
