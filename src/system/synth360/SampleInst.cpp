#include "synth360/SampleInst.h"
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth360/SynthSample.h"
#include "synth360/FxSend.h"

SampleInst360::SampleInst360(SynthSample360 *sample, bool loop, int loopStart, int loopEnd)
    : SampleInst(sample),
      mVoice(new Voice(sample->IsXMA(), sample->GetNumChannels(), false)) {
    mVoice->SetSampleRate(sample->GetSampleRate());
    mVoice->SetData(sample->GetData(), sample->GetNumBytes(), sample->GetNumSamples());
    if (loop) {
        mVoice->SetLoopRegion(loopStart, loopEnd);
    }
}

SampleInst360::~SampleInst360() { delete mVoice; }

bool SampleInst360::IsPlaying() { return mVoice->IsPlaying(); }

void SampleInst360::Pause(bool b1) { return mVoice->Pause(b1); }

void SampleInst360::SetADSR(const ADSRImpl &adsr) {
    mVoice->SetAttackRate(adsr.GetAttackRate());
    mVoice->SetReleaseRate(adsr.GetReleaseRate());
}

float SampleInst360::ElapsedTime() {
    XAUDIO2_VOICE_STATE state;
    mVoice->GetVoice()->GetState(&state, 0);
    float samples = state.SamplesPlayed;
    float rate = mSample->GetSampleRate();
    return samples / rate;
}

void SampleInst360::StartImpl() { mVoice->Start(); }
void SampleInst360::StopImpl(bool b1) { mVoice->Stop(b1); }
void SampleInst360::SetVolumeImpl(float f1) { mVoice->SetVolume(f1); }
void SampleInst360::SetPanImpl(float f1) { mVoice->SetPan(f1); }
void SampleInst360::SetSpeedImpl(float f1) { mVoice->SetSpeed(f1); }
void SampleInst360::SetSendImpl(FxSend *send) {
    mVoice->SetSend(dynamic_cast<FxSend360 *>(send));
}
void SampleInst360::SetReverbMixDbImpl(float f1) { mVoice->SetReverbMixDb(f1); }
void SampleInst360::SetReverbEnableImpl(bool b1) { mVoice->SetReverbEnable(b1); }
void SampleInst360::EndLoopImpl() { mVoice->EndLoop(); }
