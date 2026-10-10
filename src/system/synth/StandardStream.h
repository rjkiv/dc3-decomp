#pragma once
#include "StreamReceiver.h"
#include "os/File.h"
#include "synth/ADSR.h"
#include "synth/Pollable.h"
#include "synth/Stream.h"
#include "synth/StreamReader.h"
#include "utl/VarTimer.h"

class StreamReceiver;

class StandardStream : public Stream, public SynthPollable {
public:
    enum State {
        kInit = 0,
        kBuffering = 1,
        kReady = 2,
        kPlaying = 3,
        kSuspended = 4,
        kStopped = 5,
        kFinished = 6,
    };
    struct ChannelParams {
        ChannelParams();
        float mPan; // 0x0
        float mSlipSpeed; // 0x4
        bool mSlipEnabled; // 0x8
        ADSRImpl mADSR; // 0xc
        FaderGroup mFaders; // 0x30
        ObjPtr<FxSend> mFxSend; // 0x48
    };
    StandardStream(File *, float, float, Symbol, bool, bool, bool);
    // Stream
    virtual ~StandardStream();
    virtual bool Fail();
    virtual bool IsReady() const;
    virtual bool IsFinished() const;
    virtual int GetNumChannels() const;
    virtual int GetNumChanParams() const;
    virtual void Play();
    virtual void Stop();
    virtual bool IsPlaying() const;
    virtual bool IsPaused() const;
    virtual void Resync(float);
    virtual void Fill() {}
    virtual bool FillDone() const { return true; }
    virtual void EnableReads(bool);
    virtual float GetTime();
    virtual float GetJumpBackTotalTime(float) const;
    virtual float GetInSongTime();
    virtual std::vector<struct JumpInstance> *GetJumpInstances() {
        return &mJumpInstances;
    }
    virtual float GetFilePos() const { return 0; }
    virtual float GetFileLength() const { return 0; }
    virtual void SetVolume(int, float);
    virtual float GetVolume(int) const;
    virtual void SetPan(int, float);
    virtual float GetPan(int) const;
    virtual void SetFX(int, bool) {}
    virtual bool GetFX(int) const { return false; }
    virtual void SetFXCore(int, FXCore) {}
    virtual FXCore GetFXCore(int) const { return kFXCoreNone; }
    virtual void SetFXSend(int, FxSend *);
    virtual void SetSpeed(float);
    virtual float GetSpeed() const { return mSpeed; }
    virtual void LoadMarkerList(const char *);
    virtual void ClearMarkerList();
    virtual void AddMarker(const Marker &);
    virtual int MarkerListSize() const;
    virtual bool MarkerAt(int, Marker &) const;
    virtual void SetJump(float, float, const char *);
    virtual void SetJump(String &, String &);
    virtual bool CurrentJumpPoints(Marker &, Marker &);
    virtual void ClearJump();
    virtual void EnableSlipStreaming(int);
    virtual void SetSlipOffset(int, float);
    virtual void SlipStop(int);
    virtual float GetSlipOffset(int);
    virtual void SetSlipSpeed(int, float);
    virtual void SetStereoPair(int, int) {}
    virtual FaderGroup &ChannelFaders(int);
    virtual void AddVirtualChannels(int);
    virtual void RemapChannel(int, int);
    virtual StreamReceiver *GetChannel(int i) const { return mChannels[i]; }
    virtual void UpdateTime();
    virtual void UpdateTimeByFiltering();
    virtual float GetRawTime();
    virtual void SetADSR(int, const ADSR &) {}
    virtual void SetADSR(int, const ADSRImpl &);
    virtual void SetJumpSamples(int, int, const char *);
    virtual int GetSampleRate() { return mSampleRate; }
    // SynthPollable
    virtual const char *GetSoundDisplayName();

    void PollStream();
    bool IsPastStreamJumpPointOfNoReturn();
    void InitInfo(int, int, bool, int);
    float GetBufferAheadTime() const;
    int ConsumeData(void **, int, int);
    void SetBufSecs(float secs) { mBufSecs = secs; }
    int NumInfoChannels() const { return mInfoChannels; }

    static const float kStreamEndMs;

private:
    virtual void SynthPoll();

    void Init(float, float, Symbol, bool);
    void Destroy();
    void UpdateVolumes();
    void UpdateSpeed(int);
    void setJumpSamplesFromMs(float, float);
    void ClearJumpMarkers();
    void UpdateFXSends();
    int MsToSamp(float) const;
    float SampToMs(int) const;
    bool StuffChannels();
    void DoJump();

    static bool sReportLargeTimerErrors;

protected:
    State mState; // 0x14
    File *mFile; // 0x18
    StreamReader *mRdr; // 0x1c
    std::vector<StreamReceiver *> mChannels; // 0x20
    int mSampleRate; // 0x2c
    float mBufSecs; // 0x30
    float mFileStartMs; // 0x34
    float mStartMs; // 0x38
    float mLastStreamTime; // 0x3c
    VarTimer mTimer; // 0x40
    std::vector<ChannelParams *> mChanParams; // 0x78
    int mJumpFromSamples; // 0x84
    int mJumpToSamples; // 0x88
    float mJumpFromMs; // 0x8c
    float mJumpToMs; // 0x90
    bool mJumpSamplesInvalid; // 0x94
    String mJumpFile; // 0x98
    int mCurrentSamp; // 0xa0
    float mSpeed; // 0xa4
    Timer mFrameTimer; // 0xa8
    float mThrottle; // 0xd8
    Symbol mExt; // 0xdc
    bool mFloatSamples; // 0xe0
    int mVirtualChans; // 0xe4
    int mInfoChannels; // 0xe8
    float mTotalSecs; // 0xec
    bool mGetInfoOnly; // 0xf0
    std::vector<void *> mVirtBufs; // 0xf4
    std::vector<std::pair<int, int> > mChanMaps; // 0x100
    std::vector<float *> unk10c; // 0x10c
    std::vector<Marker> mMarkerList; // 0x118
    std::vector<JumpInstance> mJumpInstances; // 0x124
    Marker mStartMarker; // 0x130
    Marker mEndMarker; // 0x140
    bool unk150; // 0x150
    int mTotalSamples; // 0x154
    bool mUseFileReceivers; // 0x158
};
