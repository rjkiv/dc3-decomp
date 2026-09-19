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
            : unk0(v), unk4(p), unk8(t), unkc(o), unk10(ms) {}
        float unk0;
        float unk4;
        float unk8;
        Hmx::Object *unkc;
        float unk10;
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
    virtual void
    Play(float volume, float pan, float transpose, Hmx::Object *, float delayMs);
    virtual void Stop(Hmx::Object *, bool);
    virtual void Pause(bool);
    virtual bool IsPlaying() const;

    void SetVolume(float, Hmx::Object *);
    void SetSpeed(float, Hmx::Object *);
    void SetPan(float, Hmx::Object *);
    void SetSend(FxSend *);
    void SetReverbMixDb(float);
    void SetReverbEnable(bool);
    void SetSynthSample(SynthSample *);
    void SetMoggClip(MoggClip *);
    bool DisablePan(DataArray *);
    void EndLoop(Hmx::Object *);
    float ElapsedTime();
    int NumMarkers() const;
    bool IsMoggReady() const;
    SynthSample *Sample();

    void SetSoundEventReceiver(Hmx::Object *rcvr) { unkb8 = rcvr; }
    float GetVolume() const { return mVolume; }
    bool ReverbEnabled() const { return mReverbEnable; }
    bool Loop() const { return mLoop; }

    bool SoundEmpty() const { return !mSamples.empty() || !mDelayArgs.empty(); }

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(Sound)

private:
    void OnTriggerSound(int);

    DataNode OnPlay(DataArray *);

protected:
    Sound();

    std::list<PlayableSample *> mSamples; // 0x10
    /** "Volume in dB (0 is full volume, -96 is silence)". */
    float mVolume; // 0x18
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
    bool unk3d; // 0x3c
    /** "Which sample to play" */
    ObjPtr<SynthSample> mSynthSample; // 0x40
    /** "Which sample to play" */
    ObjPtr<MoggClip> mMoggClip; // 0x54
    /** "Envelope for this sound (SynthSample only)" */
    ObjPtr<ADSR> mEnvelope; // 0x68
    FaderGroup mFaders; // 0x7c
    AudioDuckerGroup mDuckers; // 0x94
    /** "Loop this sample" */
    bool mLoop; // 0xa4
    /** "Start of the loop, in samples. Ignored if 'looped' is unchecked." */
    int mLoopStart; // 0xa8
    /** "End of the loop, in samples.  Use -1 for the end of the sample." */
    int mLoopEnd; // 0xac
    /** "Maximum number of times this sound can be playing simultaneously.
        Use 0 for no limit. (SynthSample only)" */
    int mMaxPolyphony; // 0xb0
    bool unkb4;
    ObjPtr<Hmx::Object> unkb8;
    std::list<DelayArgs *> mDelayArgs; // 0xcc
};

#include "obj/Msg.h"

DECLARE_MESSAGE(SoundPlayMsg, "sound_play")
SoundPlayMsg(Sound *snd) : Message(Type(), snd) {}
END_MESSAGE
