#pragma once
#include "synth360/EnvelopeGenerator.h"
#include "types.h"
#include "utl/PoolAlloc.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/XAUDIO2.h"

// size 0x24
struct PoolVoice {
    IXAudio2SourceVoice *voice; // 0x0
    EnvelopeGenerator *eg; // 0x4
    EnvelopeGeneratorParams *egParams; // 0x8
    tWAVEFORMATEX wav; // 0xc
    DWORD unk20; // 0x20
};

// size 0x7c
class Voice {
public:
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

    IXAudio2SourceVoice *GetVoice() { return mPoolVoice.voice; }
    void SetAttackRate(float r) { unk30 = r; }
    void SetReleaseRate(float r) { unk34 = r; }
    void SetUnk50(int i) { unk50 = i; }

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

    unsigned int unk0;
    int unk4; // 0x4 - state? 3 = started, 1 = stopped, 4 = paused
    const void *mBuffer; // 0x8
    int mBufSizeBytes; // 0xc
    int mNumSamples; // 0x10
    int mSampleRate; // 0x14
    int mSampleStart; // 0x18 - play begin
    int mLoopStart; // 0x1c
    int mLoopEnd; // 0x20
    float mVolume; // 0x24
    float mPan; // 0x28
    float mSpeed; // 0x2c
    float unk30;
    float unk34;
    bool mXMA; // 0x38
    FxSend360 *mSend; // 0x3c
    bool mReverb; // 0x40
    float mReverbMixDb; // 0x44
    bool unk48;
    bool unk49;
    int mChannels; // 0x4c
    int unk50;
    bool unk54;
    PoolVoice mPoolVoice; // 0x58
};

void TerminateVoiceThread();
