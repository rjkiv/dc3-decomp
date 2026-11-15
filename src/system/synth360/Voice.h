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
    WAVEFORMATEX fmt; // 0xc
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

    Voice(bool isXMA, int channels, bool coherentSet);
    ~Voice();
    void SetData(const void *buffer, int bytes, int numSamples);
    void SetSampleRate(int sr);
    void Init(bool bAsynchronous);
    void InitSourceBuffer(XAUDIO2_BUFFER &sb);
    void InitVoiceParameters(XMA2WAVEFORMATEX &xma2Format, XAUDIO2_BUFFER buffer);
    void Start();
    void Stop(bool);
    void Pause(bool p);
    bool IsPlaying();
    int GetAddr();
    void SetVolume(float v);
    void SetPan(float p);
    void SetSpeed(float s);
    void SetAttackRate(float r) { mAttackRate = r; }
    void SetReleaseRate(float r) { mReleaseRate = r; }
    void SetSend(class FxSend360 *send);
    void SetReverbMixDb(float reverbMixDb);
    void SetReverbEnable(bool enable);
    void EndLoop();
    void SetLoopRegion(int loopStart, int loopEnd);
    int GetLoopStart() const { return mLoopStart; }
    int GetLoopEnd() const { return mLoopEnd; }
    void SetStartSamp(int samp);
    int GetStartSamp() { return mStartSamp; }
    void Tag(int tag) { mTag = tag; }
    IXAudio2SourceVoice *GetVoice() { return mPoolVoice.xVoice; }
    void blockingStart(bool bAsynchronous);
    // public: void nonBlockingStart();

    int GetSampleCount() const { return mNumSamples; } // not in pdb

    static bool HasPendingVoices();
    static int sHeadsetTarget;

    POOL_OVERLOAD(Voice, 0x28);

private:
    HRESULT
    createOrReuse(
        PoolVoice *pPoolVoice,
        unsigned int &fmtHash,
        WAVEFORMATEX &format,
        XAUDIO2_VOICE_SENDS *pSends
    );
    void dispose(PoolVoice *pPoolVoice, unsigned int fmtHash);
    // private: uint32_t hashFormat(tWAVEFORMATEX&);
    void SetSendImpl(class FxSend360 *send);
    void UpdateSends();
    void UpdateMix();
    // private: void UpdateSpeed();
    void SafeRestart();
    // private: IXAudio2Voice* OutputVoice();

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
