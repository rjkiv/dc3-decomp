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
    enum SoundShape {
        kSphere = 0x0000,
        kCylinder = 0x0001
    };

    // Hmx::Object
    virtual ~ThreeDSound() { RELEASE(mFader); }
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
        mFader->SetTranspose(0);
        mEnableDoppler = enable;
    }
    void SetFalloffType(EaseType type) {
        mFalloffType = type;
        CalculateFaderVolume();
    }
    void SetFalloffParameter(float param) {
        mFalloffParam = param;
        CalculateFaderVolume();
    }
    void SetMinFalloffDistance(float dist) {
        if (dist <= 0) {
            dist = 1.1754944E-38f;
        }
        mMinimumFalloff = Min(dist, mSilenceDistance);
        CalculateFaderVolume();
    }
    void SetSilenceDistance(float dist) {
        if (dist <= 0) {
            dist = 1.1754944E-38f;
        }
        mSilenceDistance = Max(dist, mMinimumFalloff);
        CalculateFaderVolume();
    }
    void SetShape(SoundShape shape) {
        mShape = shape;
        CalculateFaderVolume();
    }
    void SetRadius(float rad) {
        mRadius = rad;
        CalculateFaderVolume();
    }
    SoundShape GetShape() const { return mShape; }
    float GetSilenceDistance() const { return mSilenceDistance; }
    bool PanEnabled() const { return mEnablePan; }
    bool DopplerEnabled() const { return mEnableDoppler; }

private:
    void CalculateFaderVolume();

protected:
    ThreeDSound();

private:
    bool mIsActive; // 0x194
    bool mPlayWhenSilent; // 0x195
    float mVolume; // 0x198
    float mPan; // 0x19c
    float mTranspose; // 0x1a0
    Hmx::Object *mEventReceiver; // 0x1a4
    float mDelayMs; // 0x1a8
    /** "Equation used to determine falloff.
        See http://deki/Projects/Tool_Projects/Milo/Flow/Easing_equations" */
    EaseType mFalloffType; // 0x1ac
    /** "Optional parameter for falloff equation". Ranges from 0 to 100. */
    float mFalloffParam; // 0x1b0
    /** "Distance before any falloff is applied". Ranges from 0 to mSilenceDistance. */
    float mMinimumFalloff; // 0x1b4
    /** "Distance at which this sound is silent".
        Ranges from mMinFalloffDistance to 2147483647. */
    float mSilenceDistance; // 0x1b8
    /** "Enable the doppler effect on this object" */
    bool mEnableDoppler; // 0x1bc
    bool mEnablePan; // 0x1bd
    SoundShape mShape; // 0x1c0
    float mRadius; // 0x1c4
    Fader *mFader; // 0x1c8
    Transform mPreviousTrans; // 0x1cc
    float mDistance; // 0x20c
    float mYZProjection; // 0x210
    float mDopplerShiftPower; // 0x214
    bool mStartedPlaying; // 0x218
};
