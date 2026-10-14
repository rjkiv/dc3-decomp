#include "synth/ThreeDSound.h"
#include "math/Decibels.h"
#include "math/Easing.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "synth/Sound.h"
#include "synth/Utl.h"
#include "utl/BinStream.h"

const float sSpeedCaps[2] = { 0.00390625f, 4.0f };

ThreeDSound::ThreeDSound()
    : mIsActive(0), mPlayWhenSilent(0), mVolume(0), mPan(0), mTranspose(0),
      mEventReceiver(0), mDelayMs(0), mFalloffType(kEaseLinear), mFalloffParam(2),
      mMinimumFalloff(10), mSilenceDistance(100), mEnableDoppler(1), mEnablePan(1),
      mShape(kSphere), mRadius(10), mDistance(100.0f), mYZProjection(0),
      mDopplerShiftPower(1), mStartedPlaying(0) {
    mFader = static_cast<Fader *>(Fader::NewObject());
    mFaders.Add(mFader);
    CalculateFaderVolume();
    Vector3 v(mMinimumFalloff, mSilenceDistance, 1);
    SetLocalScale(this, v);
}

BEGIN_HANDLERS(ThreeDSound)
    HANDLE_SUPERCLASS(Sound)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(ThreeDSound)
    SYNC_PROP_SET(enable_doppler, mEnableDoppler, EnableDoppler(_val.Int()))
    SYNC_PROP_SET(enable_pan, mEnablePan, EnablePan(_val.Int()))
    SYNC_PROP_SET(falloff_type, (int &)mFalloffType, SetFalloffType((EaseType)_val.Int()))
    SYNC_PROP_SET(falloff_parameter, mFalloffParam, SetFalloffParameter(_val.Float()))
    SYNC_PROP_SET(
        min_falloff_distance, mMinimumFalloff, SetMinFalloffDistance(_val.Float())
    )
    SYNC_PROP_SET(silence_distance, mSilenceDistance, SetSilenceDistance(_val.Float()))
    SYNC_PROP_SET(shape, mShape, SetShape((SoundShape)_val.Int()))
    SYNC_PROP_SET(radius, mRadius, SetRadius(_val.Float()))
    SYNC_PROP(doppler_power, mDopplerShiftPower)
    SYNC_SUPERCLASS(Sound)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(ThreeDSound)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    mFaders.Remove(mFader);
    SAVE_SUPERCLASS(Sound)
    mFaders.Add(mFader);
    bs << mFalloffType;
    bs << mFalloffParam;
    bs << mMinimumFalloff;
    bs << mSilenceDistance;
    bs << mEnableDoppler;
    bs << mEnablePan;
    bs << mShape;
    bs << mRadius;
    bs << mPlayWhenSilent;
    bs << mDopplerShiftPower;
END_SAVES

BEGIN_COPYS(ThreeDSound)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    mFaders.Remove(mFader);
    COPY_SUPERCLASS(Sound)
    mFaders.Add(mFader);
    CREATE_COPY(ThreeDSound)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mFalloffType)
            COPY_MEMBER(mFalloffParam)
            COPY_MEMBER(mMinimumFalloff)
            COPY_MEMBER(mSilenceDistance)
            COPY_MEMBER(mEnableDoppler)
            COPY_MEMBER(mEnablePan)
            COPY_MEMBER(mShape)
            COPY_MEMBER(mRadius)
            COPY_MEMBER(mPlayWhenSilent)
            COPY_MEMBER(mEnableDoppler)
            COPY_MEMBER(mDopplerShiftPower)
        }
    END_COPYING_MEMBERS
    Vector3 v(mMinimumFalloff, mSilenceDistance, mRadius);
    SetLocalScale(this, v);
    CalculateFaderVolume();
END_COPYS

INIT_REVS(6, 0)

BEGIN_LOADS(ThreeDSound)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev >= 2) {
        LOAD_SUPERCLASS(RndTransformable)
    }
    LOAD_SUPERCLASS(Sound)
    d >> (int &)mFalloffType;
    d >> mFalloffParam;
    d >> mMinimumFalloff;
    d >> mSilenceDistance;
    mDistance = mSilenceDistance;
    if (d.rev < 2) {
        ObjPtr<RndTransformable> t(this);
        d >> t;
        SetTransParent(t, false);
    }
    if (d.rev >= 1) {
        d >> mEnableDoppler;
    }
    if (d.rev >= 3) {
        d >> mEnablePan;
    }
    if (d.rev >= 4) {
        d >> (int &)mShape;
        d >> mRadius;
    }
    if (d.rev >= 5) {
        d >> mPlayWhenSilent;
    }
    if (d.rev >= 6) {
        d >> mDopplerShiftPower;
    }
    SetLocalScale(this, Vector3(mMinimumFalloff, mSilenceDistance, mRadius));
    CalculateFaderVolume();
END_LOADS

void ThreeDSound::Highlight() {
    if (mShape >= 1U) {
        if (mShape != 1U) {
            MILO_FAIL("Trying to drawn unknown sound shape %d\n", mShape);
        } else {
            Transform xfm = WorldXfm();
            Vector3 vscale;
            MakeScale(xfm.m, vscale);
            vscale.x = 1.0f / vscale.x;
            vscale.y = 1.0f / vscale.y;
            vscale.z = 1.0f / vscale.z;
            Scale(vscale, xfm.m, xfm.m);
            if (mMinimumFalloff <= mRadius) {
                UtilDrawSphere(
                    WorldXfm().v, mMinimumFalloff, Hmx::Color(1, 0, 0), nullptr
                );
            } else {
                UtilDrawCylinder(xfm, mRadius, mMinimumFalloff, Hmx::Color(1, 0, 0), 8);
            }
            if (mSilenceDistance <= mRadius) {
                UtilDrawSphere(
                    WorldXfm().v, mSilenceDistance, Hmx::Color(0, 1, 0), nullptr
                );
            } else {
                UtilDrawCylinder(xfm, mRadius, mSilenceDistance, Hmx::Color(0, 1, 0), 8);
            }
        }
    } else {
        UtilDrawSphere(WorldXfm().v, mMinimumFalloff, Hmx::Color(1, 0, 0), nullptr);
        UtilDrawSphere(WorldXfm().v, mSilenceDistance, Hmx::Color(0, 1, 0), nullptr);
    }
}

void ThreeDSound::Play(
    float volume, float pan, float transpose, Hmx::Object *o4, float delayMs
) {
    mStartedPlaying = true;
    if (mLoop && !mPlayWhenSilent) {
        mVolume = volume;
        mEventReceiver = o4;
        mPan = pan;
        mIsActive = true;
        mTranspose = transpose;
        mDelayMs = delayMs;
    } else {
        Sound::Play(volume, pan, transpose, o4, delayMs);
    }
}

void ThreeDSound::Stop(Hmx::Object *obj, bool b2) {
    mIsActive = false;
    Sound::Stop(obj, b2);
}

bool ThreeDSound::IsPlaying() const { return mIsActive || SoundEmpty(); }

void ThreeDSound::SaveWorldXfm() { mPreviousTrans = WorldXfm(); }

bool ThreeDSound::HasMoved() { return WorldXfm() != mPreviousTrans; }

void ThreeDSound::EnablePan(bool enable) {
    mFader->SetPan(0);
    mEnablePan = enable;
    BroadcastPropertyChange("fader_pan");
}

void ThreeDSound::GetVelocity(Vector3 &vel) {
    Subtract(WorldXfm().v, mPreviousTrans.v, vel);
}

void ThreeDSound::SetAngle(float radians) {
    MILO_ASSERT(radians >= -PI && radians <= PI, 0x191);
    if (mEnablePan && !Sound::DisablePan(nullptr)) {
        mFader->SetPan(std::sin(radians));
        BroadcastPropertyChange("fader_pan");
    }
}

void ThreeDSound::SetDoppler(float doppler) {
    float powed = std::pow(doppler, mDopplerShiftPower);
    if (powed > sSpeedCaps[1] || powed < sSpeedCaps[0]) {
        powed = 1.0f;
    }
    mFader->SetTranspose(CalcTransposeFromSpeed(powed));
}

void ThreeDSound::SetDistance(float f1, float f2) {
    mDistance = f1;
    mStartedPlaying = false;
    MILO_ASSERT(!IsNaN(mDistance), 0x182);
    mYZProjection = f2;
    CalculateFaderVolume();
    float f = -96.0f;
    if (mIsActive && mFader->DuckedValue() > f && !SoundEmpty()) {
        Sound::Play(mVolume, mPan, mTranspose, mEventReceiver, mDelayMs);
    } else if (mIsActive && mFader->DuckedValue() <= f && SoundEmpty()) {
        Sound::Stop(nullptr, true);
    }
}

void ThreeDSound::CalculateFaderVolume() {
    float volume;
    if (mDistance >= mSilenceDistance) {
        volume = -96.0f;
    } else if (mDistance <= mMinimumFalloff) {
        volume = 0;
    } else {
        switch (mShape) {
        case kCylinder: {
            if (mYZProjection > mRadius) {
                mFader->SetVolume(-96.0f);
                return;
            }
        }
        case kSphere:
            break;
        default:
            MILO_FAIL("Calculating volume for unknown shape %d\n", mShape);
            break;
        }

        float val1 = 1.0f / (mMinimumFalloff - mSilenceDistance);
        float val2 = val1 * mDistance + (-(mMinimumFalloff * val1 - 1.0f));
        float ease = GetEaseFunction(mFalloffType)(val2, mFalloffParam, 0);
        ease = Clamp(0.0f, 1.0f, ease);
        volume = RatioToDb(ease);
        volume = Max(volume, -96.0f);
    }
    mFader->SetVolume(volume);
}
