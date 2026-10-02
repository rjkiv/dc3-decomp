#pragma once
#include "synth/ADSR.h"
#include "synth/Faders.h"
#include "synth/FxSend.h"

enum FXCore {
    kFXCore0 = 0,
    kFXCore1 = 1,
    kFXCoreNone = -1
};

struct Marker {
    Marker(const String &str = String()) : name(str), position(0), posMS(0) {}
    Marker(const String &name, int position, float posMS)
        : name(name), position(position), posMS(posMS) {}
    String name; // 0x0
    int position; // 0x8
    float posMS; // 0xc
};
struct JumpInstance {
    float unk0;
    float unk4;
    float unk8;
    float unkc;
};

class Stream {
public:
    virtual ~Stream() { delete mFaders; }
    virtual bool Fail() { return false; }
    virtual bool IsReady() const = 0;
    virtual bool IsFinished() const = 0;
    virtual int GetNumChannels() const = 0;
    virtual int GetNumChanParams() const = 0;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual bool IsPlaying() const = 0;
    virtual bool IsPaused() const { return false; }
    virtual void Resync(float) = 0;
    virtual void Fill() = 0;
    virtual bool FillDone() const = 0;
    virtual void EnableReads(bool) = 0;
    virtual float GetTime() = 0;
    virtual float GetJumpBackTotalTime(float) const = 0;
    virtual float GetInSongTime() = 0;
    virtual std::vector<struct JumpInstance> *GetJumpInstances() = 0;
    virtual float GetFilePos() const = 0;
    virtual float GetFileLength() const = 0;
    virtual void SetVolume(int, float) = 0;
    virtual float GetVolume(int) const = 0;
    virtual void SetPan(int, float) = 0;
    virtual float GetPan(int) const = 0;
    virtual void SetFX(int, bool) = 0;
    virtual bool GetFX(int) const = 0;
    virtual void SetFXCore(int, FXCore) = 0;
    virtual FXCore GetFXCore(int) const = 0;
    virtual void SetFXSend(int, FxSend *) {}
    virtual void SetADSR(int, const ADSR &) {}
    virtual void SetSpeed(float) = 0;
    virtual float GetSpeed() const = 0;
    virtual void LoadMarkerList(const char *) = 0;
    virtual void ClearMarkerList() {}
    virtual void AddMarker(const Marker &) {}
    virtual int MarkerListSize() const { return 0; }
    virtual bool MarkerAt(int, Marker &) const { return 0; }
    virtual void SetJump(String &, String &) = 0;
    virtual void SetJump(float, float, const char *) = 0;
    virtual bool CurrentJumpPoints(Marker &, Marker &) { return 0; }
    virtual void ClearJump() = 0;
    virtual void EnableSlipStreaming(int) = 0;
    virtual void SetSlipOffset(int, float) = 0;
    virtual void SlipStop(int) = 0;
    virtual float GetSlipOffset(int) = 0;
    virtual void SetSlipSpeed(int, float) = 0;
    virtual void SetStereoPair(int, int) {}
    virtual FaderGroup &ChannelFaders(int) = 0;
    virtual void AddVirtualChannels(int) {}
    virtual void RemapChannel(int, int) {}

    void SetVolume(float);

    FaderGroup *Faders() const { return mFaders; }

    static const float kStreamEndMs;
    static const int kStreamEndSamples;

protected:
    Stream();

    FaderGroup *mFaders; // 0x4
};
