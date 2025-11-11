#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "synth/Faders.h"
#include "synth/Sound.h"
#include "utl/MemMgr.h"

/** "Sound effect object tied to a position.  Changes volume and pan based on the current
 * camera position." */
class ThreeDSound : public RndTransformable, public Sound {
public:
    // Hmx::Object
    virtual ~ThreeDSound() { RELEASE(unk1c8); }
    OBJ_CLASSNAME(ThreeDSound)
    OBJ_SET_TYPE(ThreeDSound)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndTransformable
    virtual bool StartedPlaying() const;
    virtual void Highlight();
    // SynthPollable
    virtual void Play(float, float, float, Hmx::Object *, float);
    virtual void Stop(Hmx::Object *, bool);
    virtual bool IsPlaying() const;

    OBJ_MEM_OVERLOAD(0x14);
    NEW_OBJ(ThreeDSound)

    void EnablePan(bool);
    void SaveWorldXfm();
    bool HasMoved();

private:
    void CalculateFaderVolume();

protected:
    ThreeDSound();

    bool unk194;
    bool unk195;
    float unk198;
    float unk19c;
    float unk1a0;
    int unk1a4;
    float unk1a8;
    int unk1ac;
    float unk1b0;
    float unk1b4;
    float unk1b8;
    bool unk1bc;
    bool unk1bd;
    int mShape;
    float mRadius; // 0x1C4
    Fader *unk1c8;
    Transform unk1cc;
    float unk20c;
    float unk210;
    float mDopplerPower; // 0x214
    bool unk218;
};
