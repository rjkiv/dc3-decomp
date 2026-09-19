#pragma once
#include "math/Easing.h"
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
    virtual bool StartedPlaying() const { return mStartedPlaying; }
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
    void GetVelocity(Vector3 &);
    void SetAngle(float);
    void SetDoppler(float);
    void SetDistance(float, float);

    void EnableDoppler(bool enable) {
        unk1c8->SetTranspose(0);
        mDopplerEnabled = enable;
    }
    void SetFalloffType(EaseType type) {
        mFalloffType = type;
        CalculateFaderVolume();
    }
    void SetFalloffParameter(float param) {
        mFalloffParameter = param;
        CalculateFaderVolume();
    }
    void SetMinFalloffDistance(float dist) {
        if (dist <= 0) {
            dist = 1.1754944E-38f;
        }
        mMinFalloffDistance = Min(dist, mSilenceDistance);
        CalculateFaderVolume();
    }
    void SetSilenceDistance(float dist) {
        if (dist <= 0) {
            dist = 1.1754944E-38f;
        }
        mSilenceDistance = Max(dist, mMinFalloffDistance);
        CalculateFaderVolume();
    }
    void SetShape(int shape) {
        mShape = shape;
        CalculateFaderVolume();
    }
    void SetRadius(float rad) {
        mRadius = rad;
        CalculateFaderVolume();
    }
    int GetShape() const { return mShape; }
    float GetSilenceDistance() const { return mSilenceDistance; }
    bool PanEnabled() const { return mPanEnabled; }
    bool DopplerEnabled() const { return mDopplerEnabled; }

private:
    void CalculateFaderVolume();

protected:
    ThreeDSound();

    bool unk194; // 0x194
    bool unk195; // 0x195
    float unk198; // 0x198
    float unk19c; // 0x19c
    float unk1a0; // 0x1a0
    Hmx::Object *unk1a4; // 0x1a4
    float unk1a8; // 0x1a8
    /** "Equation used to determine falloff.
        See http://deki/Projects/Tool_Projects/Milo/Flow/Easing_equations" */
    EaseType mFalloffType; // 0x1ac
    /** "Optional parameter for falloff equation". Ranges from 0 to 100. */
    float mFalloffParameter; // 0x1b0
    /** "Distance before any falloff is applied". Ranges from 0 to mSilenceDistance. */
    float mMinFalloffDistance; // 0x1b4
    /** "Distance at which this sound is silent".
        Ranges from mMinFalloffDistance to 2147483647. */
    float mSilenceDistance; // 0x1b8
    /** "Enable the doppler effect on this object" */
    bool mDopplerEnabled; // 0x1bc
    bool mPanEnabled; // 0x1bd
    int mShape; // 0x1c0
    float mRadius; // 0x1c4
    Fader *unk1c8; // 0x1c8
    Transform unk1cc; // 0x1cc
    float mDistance; // 0x20c
    float unk210; // 0x210
    float mDopplerPower; // 0x214
    bool mStartedPlaying; // 0x218
};
