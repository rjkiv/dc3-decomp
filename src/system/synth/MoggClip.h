#pragma once
#include "obj/Object.h"
#include "synth/Faders.h"
#include "synth/FxSend.h"
#include "synth/PlayableSample.h"
#include "synth/StandardStream.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"

/** "Allows dynamic playback of Mogg-based audio clips, most notably crowd audio loops."
 */
class MoggClip : public Hmx::Object, public PlayableSample {
public:
    struct PanInfo {
        PanInfo(int, float);
        int channel;
        float panning;
    };
    // Hmx::Object
    virtual ~MoggClip();
    OBJ_CLASSNAME(MoggClip);
    OBJ_SET_TYPE(MoggClip);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    virtual bool IsPlaying() const { return mPlaying; }
    // SynthPollable
    virtual const char *GetSoundDisplayName();
    virtual void SynthPoll();
    // PlayableSample
    virtual void Play(float);
    virtual void Stop(bool);
    virtual void Pause(bool);
    virtual bool DonePlaying();
    virtual void SetVolume(float);
    virtual void SetPan(float);
    virtual void SetADSR(const ADSRImpl &) {}
    virtual void SetSpeed(float) {}
    virtual void SetReverbMixDb(float) {}
    virtual void SetReverbEnable(bool) {}
    virtual void SetSend(FxSend *);
    virtual void SetEventReceiver(Hmx::Object *o) { mEventReceiver = o; }
    virtual Hmx::Object *GetEventReceiver() { return mEventReceiver; }
    virtual void EndLoop();
    virtual float ElapsedTime();

    OBJ_MEM_OVERLOAD(0x18);
    NEW_OBJ(MoggClip)

    bool IsStreaming() const;
    void FadeOut(float);
    void UnloadWhenFinishedPlaying(bool);
    bool IsReadyToPlay() const;
    void SetLoop(bool, int, int);
    void SetFile(const char *);
    void SetPan(int, float);
    void AddFader(Fader *);
    const FilePath Path() const;
    StandardStream *GetStream() const { return mStream; }
    int NumChannels() const { return unk58; }

private:
    void ApplyLoop(bool, int, int);
    void KillStream();
    void UnloadData();
    bool EnsureLoaded();
    void UpdateFaders();
    void UpdatePanInfo();
    void LoadNumChannels();
    void LoadFile(BinStream *);

protected:
    MoggClip();

    /** "The mogg audio file to be played." */
    FilePath mMoggFile; // 0x38
    /** "Volume in dB (0 is full volume, -96 is silence)." */
    float mVolume; // 0x40
    float unk44;
    StandardStream *mStream; // 0x48
    float unk4c;
    void *mData; // 0x50
    int unk54;
    int unk58; // 0x58
    FileLoader *mLoader; // 0x5c
    std::vector<Fader *> mFaders; // 0x60
    std::vector<PanInfo> mPanInfos; // 0x6c
    ObjPtr<FxSend> unk78; // 0x78
    Fader *mFader; // 0x8c
    bool unk90;
    bool mUnloadWhenFinished; // 0x91
    bool mPlaying; // 0x92
    bool mLoop; // 0x93
    int unk94; // 0x94
    int unk98; // 0x98
    Hmx::Object *mEventReceiver; // 0x9c
    /** "Number of seconds to buffer (uses default value if set to 0.0)" */
    float mBufSecs; // 0xa0
};
