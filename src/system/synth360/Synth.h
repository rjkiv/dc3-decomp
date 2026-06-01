#pragma once
#include "FxSend.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Timer.h"
#include "synth/FxSend.h"
#include "synth/Mic.h"
#include "synth/Synth.h"
#include "xdk/XAUDIO2.h"
#include "xdk/XHV2.h"

class FxSend360;

// size 0x150
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

    CriticalSection mCritSec; // 0xb0
    std::vector<Mic *> mMics; // 0xd0
    std::vector<IXAudio2SubmixVoice *> unkdc;
    IXAudio2SourceVoice *unke8; // 0xe8
    IXAudio2 *unkec; // 0xec
    IXAudio2MasteringVoice *unkf0; // 0xf0
    IXAudio2SubmixVoice *unkf4; // 0xf4
    IXAudio2SubmixVoice *unkf8; // 0xf8
    IXAudio2Voice *unkfc; // 0xfc - some ixaudiovoice type
    IUnknown *unk100; // 0x100 - audio reverb*?
    bool unk104;
    bool unk105;
    Timer unk108;
    bool unk138;
    LevelData **unk13c; // 0x13c
    std::vector<FxSend360 *> unk140;
    bool unk14c;
};

extern Synth360 *TheXboxSynth;
