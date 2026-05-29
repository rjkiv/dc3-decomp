#pragma once
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Mic.h"
#include "synth_xbox/Mic.h"
#include "synth_xbox/Voice.h"
#include "types.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include "xdk/XVH2.h"
#include "xdk/xvh2/xvh2.h"

// size 0x58
class ChatReceiver {
public:
    ChatReceiver(IXHV2Engine *, int);
    ~ChatReceiver();
    void ActivateProcessing(bool);

private:
    void ProcessChatData(void *, unsigned int, int *);

    IXHV2Engine *mXHV; // 0x0
    DWORD unk4; // 0x4
    bool unk8; // 0x8
    bool unk9; // 0x9
    float unkc;
    float unk10;
    int unk14;
    int unk18;
    Timer unk20;
    MemStream *unk50;
    int unk54;
};

// size 0x90d0
class MicXbox : public Mic {
public:
    MicXbox(int, float);
    virtual ~MicXbox();
    virtual void Start();
    virtual void Stop();
    virtual bool IsRunning() const { return mRunning; }
    virtual Type GetType() const;
    virtual void SetDMA(bool) {}
    virtual bool GetDMA() const { return false; }
    virtual void SetGain(float);
    virtual float GetGain() const;
    virtual void SetEarpieceVolume(float) {}
    virtual float GetEarpieceVolume() const { return 0; }
    virtual void SetMute(bool);
    virtual bool GetClipping() const;
    virtual void SetOutputGain(float);
    virtual float GetOutputGain() const;
    virtual void SetSensitivity(float);
    virtual float GetSensitivity() const;
    virtual void SetVolume(float);
    virtual void SetFxSend(FxSend *);
    virtual void SetChangeNotify(bool);
    virtual void StartPlayback();
    virtual void StopPlayback();
    virtual bool IsPlaying();
    virtual void SetCompressor(bool) {}
    virtual bool GetCompressor() const { return false; }
    virtual void SetCompressorParam(float) {}
    virtual float GetCompressorParam() const { return 0; }
    virtual void ClearBuffers();
    virtual short *GetRecentBuf(int &);
    virtual short *GetContinuousBuf(int &);
    virtual int GetDroppedSamples();
    virtual int GetSampleRate() const { return 48000; }
    virtual const Symbol &GetName() const { return mName; }

    void Poll();
    void AddData(void *, int);
    void OnMicConnected(unsigned long, bool, Symbol const &);
    void OnMicDisconnected();

private:
    void ReadChatBuffer(void *, unsigned int);
    static bool AddToBuffer(std::vector<short> &, void *, int, int *);

    bool unkc;
    bool mRunning; // 0xd
    int unk10;
    bool mChangeNotify; // 0x14
    Voice *mVoice; // 0x18
    short mVoiceBuffer[0x1800]; // 0x1c
    short *unk301c; // 0x301c
    std::vector<short> unk3020;
    RingBuffer mRingBufferRecent; // 0x302c
    RingBuffer mRingBufferContinuous; // 0x3040
    short unk3054[0x3000];
    float unk9054; // 0x9054 - speed
    float unk9058;
    float unk905c;
    FxSend *mSend; // 0x9060
    float mVolume; // 0x9064
    bool mMute; // 0x9068
    float unk906c;
    float mGain; // 0x9070
    float mOutputGain; // 0x9074
    float mSensitivity; // 0x9078
    short unk907c;
    u32 unk9080;
    u32 unk9084;
    u32 unk9088;
    Timer unk9090;
    int mDroppedSamples; // 0x90c0
    Symbol mName; // 0x90c4
    bool mClipping; // 0x90c8
    int unk90cc;
};

// size 0x90
class MicManagerXbox {
public:
    // size 0x3f8
    struct ChatBuffer {
        int unk0;
        int unk4;
        int unk8[252];
    };

    void RequirePushToTalk(bool, int);
    void Poll();
    void RemoveMic(MicXbox *);
    void AddMic(MicXbox *);
    void Shutdown();
    void AddRemoteMic(unsigned long long const &, XAUDIO2_EFFECT_CHAIN *);
    void Init();
    CriticalSection *CritSec() { return &mMicArrayLock; }
    void SetMicsChanged() { mMicsChanged = true; }
    void ClearMicsChanged() { mMicsChanged = false; }
    bool MicsChanged() const { return mMicsChanged; }

    static MicManagerXbox *GetInstance();

private:
    MicManagerXbox();
    ~MicManagerXbox();

    void OnDataReady(unsigned long, void *, unsigned long, int *);

    static void DataReadyCallback(unsigned long, void *, unsigned long, int *);
    static MicManagerXbox *sInstance;

    std::vector<MicXbox *> mMics; // 0x0
    std::vector<ChatReceiver *> mChatReceivers; // 0xc
    int unk18; // 0x18 - DWORD user index?
    IXHV2Engine *mXHVEngine; // 0x1c
    std::vector<ChatBuffer> mChatBuffers; // 0x20
    HANDLE mXHVWorkerThread; // 0x2c
    bool mMicsChanged; // 0x30
    Timer unk38;
    CriticalSection mMicArrayLock; // 0x68
    int mPad; // 0x88
};
