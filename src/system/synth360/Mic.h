#pragma once
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Mic.h"
#include "synth360/Mic.h"
#include "synth360/Voice.h"
#include "types.h"
#include "utl/MemStream.h"
#include "utl/Symbol.h"
#include "xdk/XHV2.h"
#include "xdk/xhv2/xhv2.h"

// size 0x58
class ChatReceiver {
public:
    ChatReceiver(IXHV2Engine *, int);
    ~ChatReceiver();
    void ActivateProcessing(bool);

    IXHV2Engine *GetIXHV2Engine() const { return mXHV; }
    int GetPad() { return mPad; }
    bool GetEnabled() const { return mEnabled; }

    friend class MicManagerXbox;

private:
    void ProcessChatData(void *, unsigned int, int *);

    IXHV2Engine *mXHV; // 0x0
    int mPad; // 0x4
    bool mEnabled; // 0x8
    bool mProcessingActivated; // 0x9
    float mDelay; // 0xc
    float mFB; // 0x10
    int mHoldFrames; // 0x14
    int mFramesTalking; // 0x18
    Timer mNetTimer; // 0x20
    MemStream *mNetBuffer; // 0x50
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

    friend class MicManagerXbox;

private:
    void ReadChatBuffer(void *, unsigned int);
    static bool AddToBuffer(std::vector<short> &, void *, int, int *);

    bool mIsWireless; // 0xc
    bool mRunning; // 0xd
    Mic::Type mLastType; // 0x10
    bool mChangeNotify; // 0x14
    Voice *mVoice; // 0x18
    unsigned char mPlaybackBuffer[0x3000]; // 0x1c
    unsigned char *mPlaybackWritePtr; // 0x301c
    std::vector<short> mChatBuffer; // 0x3020
    RingBuffer mRingBufferRecent; // 0x302c
    RingBuffer mRingBufferContinuous; // 0x3040
    short mOutputBuffer[0x3000]; // 0x3054
    float mVoiceSpeed; // 0x9054 - speed
    float mAvgLag; // 0x9058
    float mLastLag; // 0905c
    FxSend *mFxSend; // 0x9060
    float mVolume; // 0x9064
    bool mMute; // 0x9068
    float mRMS; // 0x906c
    float mGain; // 0x9070
    float mOutputGain; // 0x9074
    float mSensitivity; // 0x9078
    short mLastSamp; // 0x907c
    MemStream *mOutputStream; // 0x9080
    MemStream *mUSBStream; // 0x9084
    MemStream *mPreFilterStream; // 0x9088
    Timer mTimer; // 0x9090
    int mDroppedSamples; // 0x90c0
    Symbol mName; // 0x90c4
    bool mClipping; // 0x90c8
};

// size 0x90
class MicManagerXbox {
public:
    // size 0x3f8
    struct ChatBuffer {
        XUID mXuid; // 0x0
        unsigned char mBuffer[1000]; // 0x8
        unsigned int mNumBytes; // 0x3f0
    };

    void RequirePushToTalk(bool, int);
    void Poll();
    void RemoveMic(MicXbox *);
    void AddMic(MicXbox *);
    void Shutdown();
    void AddRemoteMic(unsigned long long const &, XAUDIO2_EFFECT_CHAIN *);
    void Init();
    CriticalSection *CritSec() { return &mCritSec; }
    void SetMicsChanged() { mMicsChanged = true; }
    void ResetMicsChanged() { mMicsChanged = false; }
    bool DidMicsChange() const { return mMicsChanged; }

    static MicManagerXbox *GetInstance();

private:
    MicManagerXbox();
    ~MicManagerXbox();

    void OnDataReady(unsigned long, void *, unsigned long, int *);

    static void DataReadyCallback(unsigned long, void *, unsigned long, int *);
    static MicManagerXbox *sInstance;

    std::vector<MicXbox *> mInterfaces; // 0x0
    std::vector<ChatReceiver *> mChatReceivers; // 0xc
    int mPlayerUsingUSBChat; // 0x18
    IXHV2Engine *mXHV; // 0x1c
    std::vector<ChatBuffer> mChatBuffers; // 0x20
    HANDLE mWorkerThread; // 0x2c
    bool mMicsChanged; // 0x30
    Timer mSpoofTimer; // 0x38
    CriticalSection mCritSec; // 0x68
    int mPushToTalkPad; // 0x88
};
