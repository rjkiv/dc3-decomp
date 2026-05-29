#pragma once
#include "FxSend.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Mic.h"
#include "synth/Synth.h"
#include "xdk/xaudio2/xaudio2.h"
#include "xdk/xvh2/xvh2.h"

class FxSend360;

class Synth360 : public Synth {
public:
    Synth360();
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreInit();
    virtual void Init();
    virtual void SetDolby(bool, bool);
    virtual bool IsUsingDolby() const;
    virtual void Terminate();
    virtual void Poll();
    virtual bool HasPendingVoices();
    virtual Mic *GetMic(int);
    virtual int GetNumConnectedMics();
    virtual int GetNextAvailableMicID() const;
    virtual bool IsMicConnected(int) const;
    virtual void CaptureMic(int);
    virtual void ReleaseMic(int);
    virtual void ReleaseAllMics();
    virtual bool DidMicsChange() const;
    virtual void ResetMicsChanged();
    virtual Stream *NewStream(char const *, float, float, bool);
    virtual Stream *NewBufStream(void const *, int, Symbol, float, bool);
    virtual StreamReader *NewStreamDecoder(File *, StandardStream *, Symbol);
    virtual void NewStreamFile(char const *, File *&, Symbol &);
    virtual void EnableLevels(bool);
    virtual void RequirePushToTalk(bool, int);

    IXAudio2Voice *OutputVoice() const { return unkf0; }
    IXAudio2Voice *UnkF8() const { return unkf8; }

    void SetGlobalReverbPreset(const char *);
    IXAudio2SubmixVoice *GetHeadsetSubmix(int);
    void RemoveFxSend(FxSend360 *);
    void AddFxSend(FxSend360 *);

private:
    void UpdateDolby();
    void SetupHeadsetSubmixes();

    CriticalSection unkb0;
    std::vector<Mic *> mMics; // 0xd0
    std::vector<IXAudio2SubmixVoice *> unkdc;
    int unke8;
    int unkec; // 0xec - IXAudio2*
    IXAudio2Voice *unkf0;
    int unkf4;
    IXAudio2Voice *unkf8;
    int unkfc;
    u32 unk100;
    bool unk104;
    bool unk105;
    Timer unk108;
    bool unk138;
    int unk13c;
    std::vector<FxSend360 *> unk140;
    bool unk14c;
};

extern Synth360 *TheXboxSynth;
