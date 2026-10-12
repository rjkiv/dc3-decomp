#pragma once
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth/StreamReceiver.h"
#include "synth360/Voice.h"
#include "utl/MemMgr.h"

class StreamReceiver360 : public StreamReceiver {
public:
    StreamReceiver360(int, int, bool);
    virtual ~StreamReceiver360();
    virtual void SetVolume(float);
    virtual void SetPan(float);
    virtual void SetSpeed(float);
    virtual void SetADSR(const ADSRImpl &);
    virtual void Tag();
    virtual void Poll();
    virtual void SetSlipOffset(float);
    virtual void SlipStop();
    virtual void SetSlipSpeed(float);
    virtual float GetSlipOffset();
    virtual void SetFXSend(class FxSend *);

    MEM_TEMP_OVERLOAD(StreamReceiver, 0x23);
    static void Init();

protected:
    virtual int GetPlayCursor();
    virtual void PauseImpl(bool);
    virtual void PlayImpl();
    virtual void StartSendImpl(unsigned char *, int, int);
    virtual bool SendDoneImpl() { return true; }

private:
    void UpdateADSR();

    std::list<Voice *> mOldVoices; // 0x802c
    char *mVoiceBuffer; // 0x8034
    Voice *mVoice; // 0x8038 - slip voice?
    Voice *mRefVoice; // 0x803c
    int mSampleRate; // 0x8040
    int mNumBuffers; // 0x8044
    float mVolume; // 0x8048
    float mPan; // 0x804c
    float mSpeed; // 0x8050
    ADSRImpl mADSR; // 0x8054
    FxSend *mFxSend; // 0x8078
    bool mTagged; // 0x807c
};
