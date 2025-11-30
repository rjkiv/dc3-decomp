#pragma once
#include "FxSend.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Timer.h"
#include "stl/_vector.h"
#include "synth/Mic.h"
#include "synth/Synth.h"
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/xvh2/xvh2.h"

class FxSend360;

class Synth360 : public Synth {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool IsUsingDolby() const;
    virtual bool HasPendingVoices();
    virtual void EnableLevels(bool);
    virtual int GetNumConnectedMics();
    virtual void SetDolby(bool, bool);
    virtual bool DidMicsChange() const;
    virtual void ResetMicsChanged();
    virtual Stream *NewStream(char const *, float, float, bool);
    virtual Stream *NewBufStream(void const *, int, Symbol, float, bool);
    virtual StreamReader *NewStreamDecoder(File *, StandardStream *, Symbol);
    virtual void NewStreamFile(char const *, File *&, Symbol &);
    virtual void CaptureMic(int);
    virtual void ReleaseAllMics();
    virtual void RequirePushToTalk(bool, int);
    virtual void Poll();
    virtual int GetNextAvailableMicID() const;
    virtual bool IsMicConnected(int) const;
    virtual void Terminate();
    virtual Mic *GetMic(int);
    virtual void ReleaseMic(int);
    virtual void PreInit();
    virtual void Init();

    CriticalSection unkb0;
    std::vector<Mic *> unkd0;
    std::vector<IXAudio2SubmixVoice *> unkdc;
    int unke8;
    int unkec;
    int unkf0;
    int unkf4;
    int unkf8;
    int unkfc;
    u32 unk100;
    bool unk104;
    bool unk105;
    Timer unk108;
    bool unk138;
    int unk13c;
    std::vector<int> unk140;
    bool unk14c;

    Synth360();
    void SetGlobalReverbPreset(char const *);
    IXAudio2SubmixVoice *GetHeadsetSubmix(int);
    void RemoveFxSend(FxSend360 *);
    void AddFxSend(FxSend360 *);

private:
    void UpdateDolby();
    void SetupHeadsetSubmixes();
};

extern Synth360 *TheXboxSynth;
