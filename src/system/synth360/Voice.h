#pragma once
#include "synth360/EnvelopeGenerator.h"
#include "types.h"
#include "utl/PoolAlloc.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/XAUDIO2.h"

// size 0x24
struct PoolVoice {
    IXAudio2SourceVoice *xVoice; // 0x0
    EnvelopeGenerator *eg; // 0x4
    EnvelopeGeneratorParams *egParams; // 0x8
    tWAVEFORMATEX fmt; // 0xc
    DWORD tickCount; // 0x20
};

// size 0x7c
class Voice {
public:
    enum VoiceState {
        kUninitialized = 0x0000,
        kStopped = 0x0001,
        kPendingStart = 0x0002,
        kPlaying = 0x0003,
        kPaused = 0x0004
    };

    Voice(bool, int, bool);
    ~Voice();
    void InitSourceBuffer(XAUDIO2_BUFFER &);
    int GetAddr();
    void SetData(const void *, int, int);
    void Stop(bool);
    void InitVoiceParameters(XMA2WAVEFORMATEX &, XAUDIO2_BUFFER);
    void SetSampleRate(int);
    void SetLoopRegion(int, int);
    void EndLoop();
    bool IsPlaying();
    void SetStartSamp(int);
    void SetReverbMixDb(float);
    void Pause(bool);
    void SetVolume(float);
    void SetPan(float);
    void SetReverbEnable(bool);
    void SetSend(class FxSend360 *);
    void SetSpeed(float);
    void Init(bool);
    void blockingStart(bool);
    void Start();

    IXAudio2SourceVoice *GetVoice() { return mPoolVoice.xVoice; }
    int GetStartSamp() { return mStartSamp; }
    int GetSampleCount() const { return mNumSamples; }
    int GetLoopStart() const { return mLoopStart; }
    int GetLoopEnd() const { return mLoopEnd; }
    void SetAttackRate(float r) { mAttackRate = r; }
    void SetReleaseRate(float r) { mReleaseRate = r; }
    void SetTag(int i) { mTag = i; }

    static bool HasPendingVoices();
    static int sHeadsetTarget;

    POOL_OVERLOAD(Voice, 0x28);

private:
    HRESULT
    createOrReuse(PoolVoice *, unsigned int &, tWAVEFORMATEX &, XAUDIO2_VOICE_SENDS *);
    void UpdateMix();
    void UpdateSends();
    void SafeRestart();
    void SetSendImpl(class FxSend360 *);
    void dispose(PoolVoice *, unsigned int);

    unsigned int mFmtHash; // 0x0
    VoiceState mState; // 0x4
    const void *mBuffer; // 0x8
    int mBufferBytes; // 0xc
    int mNumSamples; // 0x10
    int mSampleRate; // 0x14
    int mStartSamp; // 0x18
    int mLoopStart; // 0x1c
    int mLoopEnd; // 0x20
    float mVolume; // 0x24
    float mPan; // 0x28
    float mSpeed; // 0x2c
    float mAttackRate; // 0x30
    float mReleaseRate; // 0x34
    bool mXMA; // 0x38
    FxSend360 *mSend; // 0x3c
    bool mReverbEnable; // 0x40
    float mReverbMixDb; // 0x44
    bool mReverbVoiceConnected; // 0x48
    bool mCoherentSet; // 0x49
    int mChannels; // 0x4c
    int mTag; // 0x50
    bool mHasOutputVoices; // 0x54
    PoolVoice mPoolVoice; // 0x58
};

void TerminateVoiceThread();
