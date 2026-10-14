#pragma once
#include "os/Timer.h"
#include "synth/Mic.h"

// size 0xA880
class MicNull : public Mic {
public:
    MicNull();
    virtual ~MicNull() {}
    virtual void Start() {}
    virtual void Stop() {}
    virtual bool IsRunning() const { return true; }
    virtual Type GetType() const { return kUSBMic; }
    virtual void SetDMA(bool) {}
    virtual bool GetDMA() const { return false; }
    virtual void SetGain(float) {}
    virtual float GetGain() const { return 0; }
    virtual void SetEarpieceVolume(float) {}
    virtual float GetEarpieceVolume() const { return 0; }
    virtual bool GetClipping() const { return 0; }
    virtual void SetOutputGain(float) {}
    virtual float GetOutputGain() const { return 0; }
    virtual void SetSensitivity(float) {}
    virtual float GetSensitivity() const { return 0; }
    virtual void SetCompressor(bool) {}
    virtual bool GetCompressor() const { return 0; }
    virtual void SetCompressorParam(float) {}
    virtual float GetCompressorParam() const { return 0; }
    virtual void ClearBuffers() {}
    virtual short *GetRecentBuf(int &);
    virtual short *GetContinuousBuf(int &);
    virtual int GetDroppedSamples() { return 0; }
    virtual int GetSampleRate() const;

    virtual void SetEarpiece(bool) {}
    virtual bool GetEarpiece() const { return false; }

private:
    short mRecentBuf[1536]; // 0xc
    short mContinuousBuf[10000]; // 0xc0c
    Timer mContinuousTimer; // 0x5a30
    short mReadBuf[10000]; // 0x5a60
};
