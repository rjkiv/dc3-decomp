#pragma once
#include "synth/SampleInst.h"
#include "synth360/Voice.h"
#include "utl/PoolAlloc.h"

class SampleInst360 : public SampleInst {
public:
    SampleInst360(class SynthSample360 *, bool, int, int);
    virtual ~SampleInst360();
    virtual bool IsPlaying();
    virtual void SetFXCore(FXCore) {}
    virtual float GetProgress();
    virtual void Pause(bool);
    virtual void SetADSR(const ADSRImpl &);
    virtual float ElapsedTime();

    POOL_OVERLOAD(SampleInst360, 0x16);

protected:
    virtual void StartImpl();
    virtual void StopImpl(bool);
    virtual void SetVolumeImpl(float);
    virtual void SetPanImpl(float);
    virtual void SetSpeedImpl(float);
    virtual void SetSendImpl(FxSend *);
    virtual void SetReverbMixDbImpl(float);
    virtual void SetReverbEnableImpl(bool);
    virtual void EndLoopImpl();

    Voice *mVoice; // 0xa8
};
