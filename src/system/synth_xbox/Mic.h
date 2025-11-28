#pragma once
#include "os/Timer.h"
#include "stl/_vector.h"
#include "synth/FxSend.h"
#include "synth/Mic.h"
#include "utl/Symbol.h"

class MicXbox : public Mic {
public:
    virtual ~MicXbox();
    virtual float GetGain() const;
    virtual void ClearBuffers();
    virtual int GetDroppedSamples();
    virtual bool GetClipping() const;
    virtual void SetGain(float);
    virtual Mic::Type GetType() const;
    virtual void SetOutputGain(float);
    virtual void SetSensitivity(float);
    virtual float GetOutputGain() const;
    virtual float GetSensitivity() const;
    virtual Symbol &GetName() const;
    virtual void SetVolume(float);
    virtual void SetChangeNotify(bool);
    virtual void SetMute(bool);
    virtual bool IsPlaying();
    virtual void Start();
    virtual void StartPlayback();
    virtual void StopPlayback();
    virtual void Stop();
    virtual short *GetRecentBuf(int &);
    virtual short *GetContinuousBuf(int &);
    virtual void SetFxSend(FxSend *);

    MicXbox(int, float);
    void Poll();
    void AddData(void *, int);
    void OnMicConnected(unsigned long, bool, Symbol const &);
    void OnMicDisconnected();

    u8 unkc;
    bool unkd;
    int unk10;
    bool mChangeNotify; // 0x14
    int *unk18; // this is a Voice object
    short unk1c[6144];
    int unk301c;
    std::vector<short> unk3020;
    RingBuffer *unk302c;
    u32 unk3030;
    int unk3034;
    u32 unk3038;
    u32 unk303c;
    RingBuffer *unk3040;
    u32 unk3044;
    int unk3048;
    u32 unk304c[6146];
    float unk9054;
    float unk9058;
    float unk905c;
    int unk9060;
    float mVolume; // 0x9064
    bool mMute; // 0x9068
    float unk906c;
    float mGain; // 0x9070
    float mOutputGain; // 0x9074
    float mSensitivity; // 0x9078
    int unk907c;
    u32 unk9080;
    u32 unk9084;
    u32 unk9088;
    Timer unk9090;
    int mDroppedSamples; // 0x90c0
    Symbol unk90c4;
    bool mClipping; // 0x90c8

private:
    void ReadChatBuffer(void *, unsigned int);
    static bool AddToBuffer(std::vector<short> &, void *, int, int *);
};
