#pragma once
#include "AudioDucker.h"
#include "SynthSample.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/ADSR.h"
#include "synth/Faders.h"
#include "synth/FxSend.h"
#include "synth/MoggClip.h"
#include "synth/Pollable.h"
#include "synth/PlayableSample.h"
#include "synth/SynthSample.h"
#include "utl/MemMgr.h"

/** "Basic sound effect object.  Plays several samples with a given volume, pan,
 * transpose, and envelope settings." */
class Sound : public virtual Hmx::Object, public SynthPollable {
public:
    struct DelayArgs {
        DelayArgs(float v, float p, float t, Hmx::Object *o, float ms)
            : volume(v), pan(p), transpose(t), receiver(o), timeLeftMs(ms) {}
        float volume; // 0x0
        float pan; // 0x4
        float transpose; // 0x8
        Hmx::Object *receiver; // 0xc
        float timeLeftMs; // 0x10
    };
    // Hmx::Object
    virtual ~Sound();
    OBJ_CLASSNAME(Sound);
    OBJ_SET_TYPE(Sound);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // SynthPollable
    virtual const char *GetSoundDisplayName();
    virtual void SynthPoll();
    virtual void Play(
        float volume,
        float pan,
        float transpose,
        Hmx::Object *event_receiver,
        float delayMs
    );
    virtual void Stop(Hmx::Object *receiver, bool force);
    virtual void Pause(bool p);
    virtual bool IsPlaying() const { return !mInsts.empty() || !mDelayedSounds.empty(); }

    void SetVolume(float v, Hmx::Object *listener);
    float GetVolume() const { return mVol; }
    void SetSpeed(float s, Hmx::Object *listener);
    //  public: void SetTranspose(float, Hmx::Object*);
    //  public: float GetTranspose() const;
    void SetPan(float t, Hmx::Object *listener);
    float GetPan() const { return mPan; }
    void SetSend(FxSend *send);
    FxSend *GetSend() { return mSend; }
    void SetReverbMixDb(float reverbMixDb);
    float GetReverbMixDb() { return mReverbMixDb; }
    void SetReverbEnable(bool reverbEnable);
    bool GetReverbEnable() { return mReverbEnable; }
    void SetSynthSample(SynthSample *sample);
    void SetMoggClip(MoggClip *mogg);
    void EndLoop(Hmx::Object *listener);
    bool Loop() const { return mLoop; }
    SynthSample *Sample();
    MoggClip *Mogg() { return mMoggClip; }
    FaderGroup &Faders() { return mFaders; }
    ADSR *Adsr() { return mADSR; }
    //  public: stlpmtx_std::vector<SampleMarker >& AccessMarkers();
    int NumMarkers() const;
    bool DisablePan(DataArray *unused); // lmao
    float ElapsedTime();
    void SetEventReceiver(Hmx::Object *rcvr) { mEventReceiver = rcvr; }
    bool IsMoggReadyToPlay();

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(Sound)
    REGISTER_OBJ(Sound)

private:
    void OnTriggerSound(int play);

    DataNode OnPlay(DataArray *);

protected:
    Sound();

    std::list<PlayableSample *> mInsts; // 0x10
    /** "Volume in dB (0 is full volume, -96 is silence)". */
    float mVol; // 0x18
    /** Song speed. Can be used to calculate transpose.
        Transpose: "Transpose in half steps (SynthSample only)".
            Ranges from -96 to 12.
    */
    float mSpeed; // 0x1c
    /** "Surround pan, between -4 and 4" */
    float mPan; // 0x20
    /** "Effect chain to use" */
    ObjPtr<FxSend> mSend; // 0x24
    /** "Reverb send for this sfx". Ranges from -96 to 20. */
    float mReverbMixDb; // 0x38
    /** "Enable reverb send (SynthSample only)" */
    bool mReverbEnable; // 0x3c
    bool mCanStop; // 0x3c
    /** "Which sample to play" */
    ObjPtr<SynthSample> mSynthSample; // 0x40
    /** "Which sample to play" */
    ObjPtr<MoggClip> mMoggClip; // 0x54
    /** "Envelope for this sound (SynthSample only)" */
    ObjPtr<ADSR> mADSR; // 0x68
    FaderGroup mFaders; // 0x7c
    AudioDuckerGroup mDuckerGrp; // 0x94
    /** "Loop this sample" */
    bool mLoop; // 0xa4
    /** "Start of the loop, in samples. Ignored if 'looped' is unchecked." */
    int mLoopStartSamp; // 0xa8
    /** "End of the loop, in samples.  Use -1 for the end of the sample." */
    int mLoopEndSamp; // 0xac
    /** "Maximum number of times this sound can be playing simultaneously.
        Use 0 for no limit. (SynthSample only)" */
    int mMaxPolyphony; // 0xb0
    bool mAddZombiesOnDelete; // 0xb4
    ObjPtr<Hmx::Object> mEventReceiver; // 0xb8
    std::list<DelayArgs *> mDelayedSounds; // 0xcc
};

#include "obj/Msg.h"

DECLARE_MESSAGE(SoundPlayMsg, "sound_play")
SoundPlayMsg(Sound *snd) : Message(Type(), snd) {}
Sound *GetSound() const { return mData->Obj<Sound>(2); }
END_MESSAGE
