#pragma once
#include "utl/Symbol.h"

// forward decs
class FxSend;
class DataArray;

class Mic {
public:
    enum Type {
        kMicNull = 2
    };
    Mic() : mInUse(0), unk8(1.0f) {}
    virtual ~Mic() {}
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual bool IsRunning() const = 0;
    virtual Type GetType() const = 0;
    virtual void SetDMA(bool) = 0;
    virtual bool GetDMA() const = 0;
    virtual void SetGain(float) = 0;
    virtual float GetGain() const = 0;
    virtual void SetEarpieceVolume(float) = 0;
    virtual float GetEarpieceVolume() const = 0;
    virtual void SetMute(bool) {}
    virtual bool GetClipping() const = 0;
    virtual void SetOutputGain(float) = 0;
    virtual float GetOutputGain() const = 0;
    virtual void SetSensitivity(float) = 0;
    virtual float GetSensitivity() const = 0;
    virtual void SetVolume(float) {}
    virtual void SetFxSend(FxSend *) {}
    virtual bool IsMultiMicDevice() const { return false; }
    virtual void SetChangeNotify(bool) {}
    virtual void StartPlayback() {}
    virtual void StopPlayback() {}
    virtual bool IsPlaying() { return false; }
    virtual void SetCompressor(bool) = 0;
    virtual bool GetCompressor() const = 0;
    virtual void SetCompressorParam(float) = 0;
    virtual float GetCompressorParam() const = 0;
    virtual void ClearBuffers() = 0;
    virtual short *GetRecentBuf(int &) = 0;
    virtual short *GetContinuousBuf(int &) = 0;
    virtual int GetDroppedSamples() { return 0; }
    virtual int GetSampleRate() const = 0;
    virtual float PercentTalking() { return 0.0f; }
    virtual void MarkAsInUse(bool b) { mInUse = b; }
    virtual bool IsInUse() { return mInUse; }
    virtual const Symbol &GetName() const {
        static Symbol generic_usb("generic_usb");
        return generic_usb;
    }

    void Set(const DataArray *);

protected:
    bool mInUse; // 0x4
    float unk8; // 0x8
};

class RingBuffer {
public:
    ~RingBuffer();
    void Reset();
    void Init(int);
    int Write(void *, int);
    int Peek(void *, int);
    int Read(void *, int);

protected:
    int mSize; // 0x0
    int unk4;
    void *mBuffer; // 0x8
    int unkc;
    int unk10;
};
