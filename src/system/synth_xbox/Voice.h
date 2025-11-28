#pragma once
#include "types.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbase.h"

class Voice {
public:
    Voice(bool, int, bool);
    ~Voice();
    void InitSourceBuffer(XAUDIO2_BUFFER &);
    int GetAddr();
    void SetData(void const *, int, int);
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
    // void SetSend(FXSend360 *);
    static bool HasPendingVoices();
    void SetSpeed(float);
    void Init(bool);
    void blockingStart(bool);
    void Start();

    u32 unk0;
    int unk4;
    int unk8;
    int unkc;
    int mNumSamples; // 0x10
    int mSampleRate; // 0x14
    int unk18;
    int mLoopStart; // 0x1c
    int mLoopEnd; // 0x20
    float mVolume; // 0x24
    float mPan; // 0x28
    float unk2c;
    float unk30;
    float unk34;
    bool unk38;
    int unk3c;
    bool unk40;
    float unk44;
    bool unk48;
    bool unk49;
    int unk4c;
    int unk50;
    bool unk54;
    int unk58;
    int unk5c;
    int unk60; // PoolVoice

private:
    // long createOrReuse(PoolVoice *, unsigned int &, tWAVEFORMATEX &,
    // XAUDIO2_VOICE_SENDS *);
    void UpdateMix();
    void UpdateSends();
    void SafeRestart();
    // void SetSendImpl(FxSend360 *);
    // void dispose(PoolVoice *, unsigned int);
};

unsigned long StartVoiceThreadEntry(void *);
