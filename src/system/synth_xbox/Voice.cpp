#include "synth_xbox/Voice.h"
#include "math/Utl.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "synth/FxSend.h"
#include "synth_xbox/EnvelopeGenerator.h"
#include "synth_xbox/Synth.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xaudio2/xaudio2.h"
#include <list>
#include <deque>

HANDLE gEvent = INVALID_HANDLE_VALUE;
HANDLE gVoiceThread = INVALID_HANDLE_VALUE;

bool gShutdownVoiceThread = false;
bool gHasPendingStopCommits = false;
bool gCommitSyncVoices = false;
bool gWasCommitSyncVoices = false;
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
    : unk4(0), mBuffer(0), mBufSizeBytes(0), mNumSamples(0), mSampleRate(0),
      mSampleStart(0), mLoopStart(-1), mLoopEnd(-1), mVolume(1.0f), mPan(0), mSpeed(1.0f),
      unk30(0.001f), unk34(0.001f), mXMA(xma), mSend(0), mReverb(false),
      mReverbMixDb(-96.0f), unk48(false), unk49(b2), mChannels(chans), unk50(0),
      unk54(false) {
    mPoolVoice.eg = 0;
    mPoolVoice.egParams = 0;
    mPoolVoice.voice = 0;
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
    while (unk4 == 2) {
        if (unk49) {
            StartSynchronizedVoices();
        }
        Sleep(0);
    }
    if (mSend) {
        mSend->RemoveOwnerVoice(this);
    }
    if (GetVoice()) {
        GetVoice()->Stop(0, 0);
        dispose(&mPoolVoice, unk0);
    }
}

// clang-format off
void Voice::Init(bool b1) {
    if (TheXboxSynth->OutputVoice()) {
        if (!b1) {
            unk4 = 1;
        }
        MILO_ASSERT(0 < mSampleRate && mSampleRate <= 48000, 0x160);
        MILO_ASSERT(mBuffer, 0x161);
        if (mSend && !mSend->HasVoices()) {
            FxSend *send = dynamic_cast<FxSend *>(mSend);
            send->RebuildChain();
        }
        XAUDIO2_SEND_DESCRIPTOR desc;
        desc.Flags = 0;
        IXAudio2Voice* iv;
        if(mSend){
            iv = mSend->GetOutputVoice();
        } else {
            iv = TheXboxSynth->OutputVoice();
        }
        desc.pOutputVoice = iv;
        std::vector <XAUDIO2_SEND_DESCRIPTOR> descs;
        if (desc.pOutputVoice) {
            descs.push_back(desc);
        }
        if (mReverb) {
            XAUDIO2_SEND_DESCRIPTOR desc2;
            desc2.Flags = 0;
            desc2.pOutputVoice = TheXboxSynth->UnkF8();
            descs.push_back(desc2);
            unk48 = true;
        }
        XAUDIO2_SEND_DESCRIPTOR desc3;
        desc3.Flags = 0;
        desc3.pOutputVoice = TheXboxSynth->GetHeadsetSubmix(sHeadsetTarget);
        if(desc3.pOutputVoice){
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

        XAUDIO2_VOICE_SENDS* sendsPtr = nullptr;
        if(sends.SendCount){
            sendsPtr = &sends;
        } 
        HRESULT hr = createOrReuse(&mPoolVoice, unk0, fmtex.wfx, sendsPtr);
        
        MILO_ASSERT(SUCCEEDED(hr), 0x19D);
        MILO_ASSERT(GetVoice(), 0x19E);
        hr = GetVoice()->SubmitSourceBuffer(&xbuffer, nullptr);
        MILO_ASSERT(SUCCEEDED(hr), 0x1A3);
        UpdateMix();
        if(GetVoice()){
            GetVoice()->SetFrequencyRatio(mSpeed, 0);
        }
        mPoolVoice.egParams->unk0 = unk30;
        mPoolVoice.egParams->unk4 = unk34;
        mPoolVoice.egParams->unk8 = 0;
        mPoolVoice.egParams->unkc = 0;
        hr = GetVoice()->SetEffectParameters(0, mPoolVoice.egParams, sizeof(EnvelopeGeneratorParams), 0);
        MILO_ASSERT(SUCCEEDED(hr), 0x1B0);
    }
}
// clang-format on

void Voice::blockingStart(bool b1) {
    if (!gShutdownVoiceThread && TheXboxSynth->OutputVoice()) {
        CritSecTracker t(&gLockPendingLists);
        Init(b1);
        HRESULT hr = GetVoice()->Start(0, unk49 != false);
        MILO_ASSERT(SUCCEEDED(hr), 0x29B);
        unk4 = 3;
    }
}

void Voice::Pause(bool b1) {
    if (b1 != (unk4 == 4) && IsPlaying()) {
        if (unk49 && unk4 == 2 && b1) {
            StartSynchronizedVoices();
        }
        while (unk4 == 2) {
            Sleep(0);
        }
        MILO_ASSERT(GetVoice(), 0x2B4);
        if (b1) {
            gHasPendingStopCommits = true;
            HRESULT hr = GetVoice()->Stop(0, unk49 ? 2 : 0);
            MILO_ASSERT(SUCCEEDED(hr), 700);
            unk4 = 4;
        } else {
            SafeRestart();
        }
    }
}

void Voice::SafeRestart() {
    MILO_ASSERT(GetVoice(), 0x471);
    GetVoice()->Start(0, unk49 != false);
    unk4 = 3;
}

void Voice::Stop(bool b1) {
    if (GetVoice()) {
        if (b1) {
            GetVoice()->Stop(0, 0);
        } else {
            MILO_ASSERT(mPoolVoice.egParams, 0x14D);
            mPoolVoice.egParams->unk8 = 1;
            HRESULT hr = GetVoice()->SetEffectParameters(
                0, mPoolVoice.egParams, sizeof(EnvelopeGeneratorParams), 0
            );
            MILO_ASSERT(SUCCEEDED(hr), 0x150);
        }
    }
    unk4 = 1;
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
    if (mReverb != b) {
        mReverb = b;
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
    mSampleStart = samp;
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
    mBufSizeBytes = bytes;
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
    float speed = f1 > 0.01f ? f1 : 0.01f;
    if (speed > 2 && mXMA) {
        MILO_NOTIFY_ONCE("can't pitch an XMA sound up more than one octave");
    }
    mSpeed = speed;
    if (GetVoice()) {
        GetVoice()->SetFrequencyRatio(mSpeed, 0);
    }
}

void Voice::InitSourceBuffer(XAUDIO2_BUFFER &audio_buffer) {
    audio_buffer.pAudioData = (BYTE *)mBuffer;
    audio_buffer.AudioBytes = mBufSizeBytes;
    audio_buffer.pContext = 0;
    audio_buffer.PlayBegin = mSampleStart;
    audio_buffer.PlayLength = 0;
    if (mLoopStart >= 0) {
        if (mLoopEnd < 0) {
            mLoopEnd = mNumSamples;
        }
        if (mXMA) {
            mLoopStart -= (mLoopStart / 128) * 128;
            mLoopEnd -= (mLoopEnd / 128) * 128;
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
