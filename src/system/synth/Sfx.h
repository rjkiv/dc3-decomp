#pragma once
#include "obj/Object.h"
#include "synth/ADSR.h"
#include "synth/FxSend.h"
#include "synth/MoggClipMap.h"
#include "synth/Sequence.h"
#include "synth/SynthSample.h"
#include "synth/SampleInst.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"

class SfxMap {
public: // RB2 said so
    SfxMap(Hmx::Object *);
    void Save(BinStream &) const;
    void Load(BinStreamRev &);

    /** "Which sample to play" */
    ObjPtr<SynthSample> mSample; // 0x0
    /** "Volume in dB (0 is full volume, -96 is silence)" */
    float mVolume; // 0x14
    /** "Surround pan, between -4 and 4" */
    float mPan; // 0x18
    /** "Transpose in half steps" */
    float mTranspose; // 0x1c
    /** "Which core's digital FX should be used in playing this sample" */
    FXCore mFXCore; // 0x20
    ADSRImpl mADSR; // 0x24
};

BinStream &operator<<(BinStream &, const SfxMap &);
BinStream &operator>>(BinStreamRev &, SfxMap &);

class SfxInst : public SeqInst {
public:
    SfxInst(class Sfx *);
    virtual ~SfxInst();
    virtual void Stop();
    virtual bool IsRunning();
    virtual void UpdateVolume();
    virtual void SetPan(float);
    virtual void SetTranspose(float);
    virtual void StartImpl();

    void Pause(bool);
    void SetSend(FxSend *);
    void SetReverbMixDb(float);
    void SetReverbEnable(bool);
    void SetSpeed(float);

    POOL_OVERLOAD(SfxInst, 0x4E);

private:
    Sfx *mSfx; // 0x44
    std::vector<SampleInst *> mSamples; // 0x48
    float mStartProgress; // 0x54
};

/** "Legacy sound effect object.  Plays several samples with a given volume, pan,
 * transpose, and envelope settings." */
class Sfx : public Sequence {
public:
    // Hmx::Object
    OBJ_CLASSNAME(Sfx);
    OBJ_SET_TYPE(Sfx);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // Sequence
    virtual SeqInst *MakeInstImpl();
    // SynthPollable
    virtual void SynthPoll() { Sequence::SynthPoll(); }

    void SetSend(FxSend *);
    void SetReverbMixDb(float);
    void SetReverbEnable(bool);
    FxSend *GetSend() const { return mSend; }
    float GetReverbMixDb() const { return mReverbMixDb; }
    bool GetReverbEnable() const { return mReverbEnable; }
    ObjVector<SfxMap> &SfxMaps() { return mMaps; }
    ObjVector<MoggClipMap> &MoggClipMaps() { return mMoggClipMaps; }

    OBJ_MEM_OVERLOAD(0x1E);
    NEW_OBJ(Sfx);
    static void Init() { REGISTER_OBJ_FACTORY(Sfx) }

    void Pause(bool);

protected:
    Sfx();

    ObjVector<SfxMap> mMaps; // 0x80
    ObjVector<MoggClipMap> mMoggClipMaps; // 0x90
    /** "Effect chain to use" */
    ObjPtr<FxSend> mSend; // 0xa0
    /** "Reverb send for this sfx" */
    float mReverbMixDb; // 0xb4
    /** "Enable reverb send" */
    bool mReverbEnable; // 0xb8
    ObjPtrList<SfxInst> mSfxInsts; // 0xbc
};
