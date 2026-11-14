#pragma once
#include "ADSR.h"
#include "SynthSample.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Platform.h"
#include "rndobj/Overlay.h"
#include "synth/ADSR.h"
#include "synth/ByteGrinder.h"
#include "synth/FxSendPitchShift.h"
#include "synth/Mic.h"
#include "synth/MicClientMapper.h"
#include "synth/MidiSynth.h"
#include "synth/Sound.h"
#include "synth/StandardStream.h"
#include "synth/Stream.h"
#include "utl/Str.h"

enum FXMode {
    kFXModeOff = 0x0000,
    kFXModeRoom = 0x0001,
    kFXModeSmallStudio = 0x0002,
    kFXModeMedStudio = 0x0003,
    kFXModeLargeStudio = 0x0004,
    kFXModeHall = 0x0005,
    kFXModeSpace = 0x0006,
    kFXModeEcho = 0x0007,
    kFXModeDelay = 0x0008,
    kFXModePipe = 0x0009
};

class StreamReader;
class TranscodableMixerOutput;

class Synth : public Hmx::Object, public RndOverlay::Callback {
    friend void SynthPreInit(); // idc anymore lmao
    friend void SynthTerminate();

public:
    Synth();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreInit() {}
    virtual void Init();
    virtual void InitSecurity();
    virtual void SetDolby(bool on, bool immediate) {} // 0x64
    virtual bool IsUsingDolby() const { return false; }
    virtual bool Fail() { return false; }
    virtual void Terminate();
    virtual void Poll();
    virtual bool HasPendingVoices() { return false; }
    virtual void SetFXMode(int, FXMode) {}
    virtual FXMode GetFXMode(int) const { return kFXModeOff; }
    virtual void SetFXVolume(int core, float volDb) {}
    virtual float GetFXVolume(int) const { return 0; }
    virtual void SetFXDelay(int core, float delay) {}
    virtual float GetFXDelay(int) const { return 0; }
    virtual void SetFXFeedback(int core, float feedbackDb) {}
    virtual float GetFXFeedback(int) const { return 0; }
    virtual void SetFXChain(bool) {}
    virtual bool GetFXChain() const { return false; }
    virtual void SetChatVoiceGain(int ixVoice, float gain) {}
    virtual float GetChatVoiceGain(int) { return 1; }
    virtual Mic *GetMic(int idx) { return mNullMics[idx]; }
    virtual void SetMicFX(bool) {}
    virtual bool GetMicFX() const { return false; }
    virtual void SetMicVolume(float volDb) {}
    virtual float GetMicVolume() const { return 0; }
    virtual void SuspendMics() {}
    virtual void ResumeMics() {}
    virtual int GetNumConnectedMics() { return 0; }
    virtual int GetNextAvailableMicID() const { return -1; }
    virtual bool IsMicConnected(int micID) const { return false; }
    virtual void CaptureMic(int micID) {}
    virtual void ReleaseMic(int micID) {}
    virtual void ReleaseAllMics() {}
    virtual TranscodableMixerOutput *GetSecureOutput() { return nullptr; }
    virtual bool DidMicsChange() const { return false; }
    virtual void ResetMicsChanged() {}
    virtual Stream *
    NewStream(const char *filebase, float startMs, float bufSecs, bool secure);
    virtual Stream *NewBufStream(
        const void *buf, int size, Symbol ext, float startMs, bool pollingEnabled
    );
    virtual StreamReader *NewStreamDecoder(File *, StandardStream *, Symbol ext) {
        return nullptr;
    }
    virtual void NewStreamFile(const char *fileBase, File *&fileOut, Symbol &extOut);
    virtual void EnableLevels(bool) {}
    virtual void RequirePushToTalk(bool require, int pad) {}
    virtual void SetIncomingVoiceChatVolume(float volDb) {}
    virtual FxSendPitchShift *CreatePitchShift(int stage, SendChannels channels);
    virtual void DestroyPitchShift(FxSendPitchShift *pitchShift);

    // RndOverlay::Callback
    virtual float UpdateOverlay(RndOverlay *, float);

    void RunFlow(const char *);
    //  public: void Play(const char*, float, float, float);
    void PlaySound(const char *name, float volume, float pan, float transpose);
    Fader *MasterFader() const { return mMasterFader; }
    void SetMasterVolume(float);
    float GetMasterVolume();
    Fader *SfxFader() const { return mSfxFader; }
    Fader *MidiInstrumentFader() const { return mMidiInstrumentFader; }
    int GetSampleMem(ObjectDir *, Platform plat);
    int GetSPUOverhead();
    int GetFXOverhead();
    void AddPlayHandler(Hmx::Object *handler);
    void RemovePlayHandler(Hmx::Object *handler);
    //  public: void ClearPlayHandlers();
    void SendToPlayHandlers(Sound *);
    void StopAllSfx(bool);
    void PauseAllSfx(bool);
    //  public: void FlushZombieSounds();
    void SetFX(const DataArray *data);
    //  public: void PlayMidi(unsigned char, unsigned char, unsigned char, int32_t);
    //  public: void PauseMidi(bool);
    //  public: void PlayNoteOn(unsigned char, unsigned char, unsigned char);
    //  public: void PlayNoteOff(unsigned char, unsigned char, unsigned char);
    //  public: void PlayInstrument(unsigned char, unsigned char);
    //  public: void PlayPitchWheel(unsigned char, int32_t);
    //  public: void PlayController(unsigned char, unsigned char, unsigned char);
    //  public: void PlayController(unsigned char, unsigned char);
    //  public: void PlayBank(unsigned char, unsigned char);
    //  public: void PlayVolume(unsigned char, float);
    //  public: void PlayPan(unsigned char, float);
    //  public: void PlayAllNotesOff(unsigned char);
    //  public: void PlayAllSoundOff(unsigned char);
    //  public: void PlayReset(unsigned char);
    int GetNumMics() const;
    void StopPlaybackAllMics();
    void SetMic(const DataArray *data);
    MicClientMapper *GetMicClientMapper() { return mMicClientMapper; }
    class TranscodableMixer *GetSecureMixer() { return mSecureMixer; }
    void ToggleHud();
    bool HudShowing() const { return mHud->Showing(); }
    void DrawMeter(float &y0, float lvl, float pkLvl, const char *label);
    void DrawMeterScale(float &y0);
    ByteGrinder &GetGrinder() { return mGrinder; }
    //  public: void SetDebugStream(Stream*);
    std::vector<LevelData> &GetLevelData() { return mLevelData; }
    void SetCommonBank(ObjectDir *bank) { mCommonBank = bank; }
    bool CheckCommonBank(bool warn);
    const ADSRImpl *DefaultADSR();
    void StopAllSounds();
    void AddZombie(SampleInst *);
    //  public: bool IsInVOEditMode();
    //  public: const String& GetVOEditSound() const;
    //  public: void SetVOEditSound(const char*);

    template <class T>
    T *Find(const char *name, bool fail = true) {
        if (!CheckCommonBank(false))
            return nullptr;
        else {
            T *obj = mCommonBank->Find<T>(name, false);
            if (!obj && fail) {
                MILO_FAIL(
                    "Synth::Find() - %s %s not found in %s",
                    T::StaticClassName(),
                    name,
                    mCommonBank->GetPathName()
                );
            }
            return obj;
        }
    }

private:
    void CullZombies();

    DataNode OnPassthrough(DataArray *msg);
    DataNode OnStartMic(const DataArray *data);
    DataNode OnStopMic(const DataArray *data);
    DataNode OnNumConnectedMics(const DataArray *data);
    DataNode OnSetMicVolume(const DataArray *data);
    DataNode OnSetFX(const DataArray *data);
    DataNode OnSetFXVol(const DataArray *data);

    static Synth *New();

protected:
    virtual ~Synth() {}

    std::vector<LevelData> mLevelData; // 0x30
    bool mTrackLevels; // 0x3c

private:
    ByteGrinder mGrinder; // 0x40
    int mNumMics; // 0x44
    MidiSynth *mMidiSynth; // 0x48
    std::vector<Mic *> mNullMics; // 0x4c
    bool mMuted; // 0x58
    std::list<ObjectDir *> mBanks; // 0x5c
    ObjDirPtr<ObjectDir> mCommonBank; // 0x64
    std::list<SampleInst *> mZombieInsts; // 0x78
    Fader *mMasterFader; // 0x80
    Fader *mSfxFader; // 0x84
    Fader *mMidiInstrumentFader; // 0x88
    std::list<Hmx::Object *> mPlayHandlers; // 0x8c
    MicClientMapper *mMicClientMapper; // 0x94
    class TranscodableMixer *mSecureMixer; // 0x98
    Stream *mDebugStream; // 0x9c
    RndOverlay *mHud; // 0xa0
    ADSRImpl *mADSR; // 0xa4
    String m_VOEditSnd; // 0xa8
};

void SynthPreInit();
void SynthInit();
void SynthTerminate();

extern Synth *TheSynth;
