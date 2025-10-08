#pragma once
#include "SynthSample.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "synth/ADSR.h"
#include "synth/ByteGrinder.h"
#include "synth/FxSendPitchShift.h"
#include "synth/Mic.h"
#include "synth/MicClientMapper.h"
#include "synth/MidiSynth.h"
#include "synth/StandardStream.h"
#include "synth/Stream.h"
#include "utl/Str.h"

enum FXMode {
    kFXModeOff,
    kFXModeRoom,
    kFXModeSmallStudio,
    kFXModeMedStudio,
    kFXModeLargeStudio,
    kFXModeHall,
    kFXModeSpace,
    kFXModeEcho,
    kFXModeDelay,
    kFXModePipe,
    kFXModeChorus,
    kFXModeWah,
    kFXModeFlanger
};

class StreamReader;
class TranscodableMixerOutput;

class Synth : public Hmx::Object, public RndOverlay::Callback {
public:
    Synth();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreInit() {}
    virtual void Init();
    virtual void InitSecurity();
    virtual void SetDolby(bool, bool) {}
    virtual bool IsUsingDolby() const { return false; }
    virtual bool Fail() { return false; }
    virtual void Terminate();
    virtual void Poll();
    virtual bool HasPendingVoices() { return false; }
    virtual void SetFXMode(int, FXMode) {}
    virtual FXMode GetFXMode(int) const { return kFXModeOff; }
    virtual void SetFXVolume(int, float) {}
    virtual float GetFXVolume(int) const { return 0; }
    virtual void SetFXDelay(int, float) {}
    virtual float GetFXDelay(int) const { return 0; }
    virtual void SetFXFeedback(int, float) {}
    virtual float GetFXFeedback(int) const { return 0; }
    virtual void SetFXChain(bool) {}
    virtual bool GetFXChain() const { return false; }
    virtual void SetChatVoiceGain(int, float) {}
    virtual float GetChatVoiceGain(int) { return 1; }
    virtual Mic *GetMic(int);
    virtual void SetMicFX(bool) {}
    virtual bool GetMicFX() const { return false; }
    virtual void SetMicVolume(float) {}
    virtual float GetMicVolume() const { return 0; }
    virtual void SuspendMics() {}
    virtual void ResumeMics() {}
    virtual int GetNumConnectedMics() { return 0; }
    virtual int GetNextAvailableMicID() const { return -1; }
    virtual bool IsMicConnected(int) const { return false; }
    virtual void CaptureMic(int) {}
    virtual void ReleaseMic(int) {}
    virtual void ReleaseAllMics() {}
    virtual TranscodableMixerOutput *GetSecureOutput() { return nullptr; }
    virtual bool DidMicsChange() const { return false; }
    virtual void ResetMicsChanged() {}
    virtual Stream *NewStream(const char *, float, float, bool);
    virtual Stream *NewBufStream(const void *, int, Symbol, float, bool);
    virtual StreamReader *NewStreamDecoder(File *, StandardStream *, Symbol) {
        return nullptr;
    }
    virtual void NewStreamFile(const char *, File *&, Symbol &);
    virtual void EnableLevels(bool) {}
    virtual void RequirePushToTalk(bool, int) {}
    virtual void SetIncomingVoiceChatVolume(float) {}
    virtual FxSendPitchShift *CreatePitchShift(int, SendChannels);
    virtual void DestroyPitchShift(FxSendPitchShift *);

    // RndOverlay::Callback
    virtual float UpdateOverlay(RndOverlay *, float);

    Fader *MasterFader() const { return mMasterFader; }
    void RemovePlayHandler(Hmx::Object *);
    bool CheckCommonBank(bool);

    template <class T>
    T *Find(const char *name, bool fail) {
        if (!CheckCommonBank(false))
            return nullptr;
        else {
            T *obj = unk64->Find<T>(name, false);
            if (!obj && fail) {
                MILO_FAIL(
                    "Synth::Find() - %s %s not found in %s",
                    T::StaticClassName(),
                    name,
                    unk64->GetPathName()
                );
            }
            return obj;
        }
    }

protected:
    virtual ~Synth() {}

    std::vector<LevelData> mLevelData; // 0x30
    bool mTrackLevels; // 0x3c
    ByteGrinder mByteGrinder; // 0x40
    int mNumMics; // 0x44
    MidiSynth *mMidiSynth; // 0x48
    std::vector<Mic *> mMics; // 0x4c
    bool mMuted; // 0x58
    std::list<ObjectDir *> unk5c; // 0x5c
    ObjDirPtr<ObjectDir> unk64; // 0x64
    std::list<SampleInst *> mZombieInsts; // 0x78
    Fader *mMasterFader; // 0x80
    Fader *mSfxFader; // 0x84
    Fader *mMidiInstrumentFader; // 0x88
    std::list<Hmx::Object *> mPlayHandlers; // 0x8c
    MicClientMapper *mMicClientMapper; // 0x94
    int unk98; // TranscodableMixer* mSecureMixer?
    int unk9c; // Stream* mDebugStream?
    RndOverlay *mHud; // 0xa0
    ADSRImpl *mADSR; // 0xa4
    String unka8; // 0xa8
};

void SynthInit();
void SynthTerminate();

extern Synth *TheSynth;
