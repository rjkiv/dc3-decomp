#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "synth/PlayableSample.h"
#include "synth/Stream.h"
#include "synth/SynthSample.h"

class SampleInst : public Hmx::Object, public PlayableSample {
public:
    SampleInst(SynthSample *);
    virtual ~SampleInst();
    virtual bool IsPlaying() const = 0;
    virtual void SetFXCore(FXCore) = 0;
    virtual float GetProgress() { return 0; }
    virtual void SetStartProgress(float) {}
    virtual void StartImpl() = 0;
    virtual void StopImpl(bool) = 0;
    virtual void SetVolumeImpl(float) = 0;
    virtual void SetPanImpl(float) = 0;
    virtual void SetSpeedImpl(float) = 0;
    // PlayableSample
    virtual void SynthPoll();
    virtual void Play(float);
    virtual void Stop(bool);
    virtual void Pause(bool) = 0;
    virtual bool DonePlaying();
    virtual void SetVolume(float);
    virtual void SetPan(float);
    virtual void SetADSR(const ADSRImpl &) = 0;
    virtual void SetSpeed(float);
    virtual void SetReverbMixDb(float);
    virtual void SetReverbEnable(bool);
    virtual void SetSend(FxSend *);
    virtual void SetEventReceiver(Hmx::Object *);
    virtual Hmx::Object *GetEventReceiver() { return mEventReceiver; }
    virtual void EndLoop();
    virtual float ElapsedTime() { return 0; }

    void SetBankVolume(float);
    void SetBankPan(float);
    void SetBankSpeed(float);

protected:
    virtual void SetSendImpl(FxSend *) {}
    virtual void SetReverbMixDbImpl(float) {}
    virtual void SetReverbEnableImpl(bool) {}
    virtual void EndLoopImpl() {
        MILO_NOTIFY("EndLoop not implemented on this platform\n");
    }

    void UpdateVolume();

    ObjPtr<SynthSample> mSample; // 0x38
    float mVolume; // 0x4c
    float mBankVolume; // 0x50
    float mPan; // 0x54
    float mBankPan; // 0x58
    float mSpeed; // 0x5c
    float mBankSpeed; // 0x60
    ObjPtr<FxSend> mSend; // 0x64
    float mReverbMixDb; // 0x78
    bool mReverbEnabled; // 0x7c
    ObjPtr<Hmx::Object> mEventReceiver; // 0x80
    int unk94; // 0x94
    double unk98; // 0x98
    bool unka0; // 0xa0
    bool unka1; // 0xa1
};
