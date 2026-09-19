#include "world/ThreeDSoundManager.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/Trans.h"
#include "synth/ThreeDSound.h"
#include "world/Dir.h"
#include <float.h>

ThreeDSoundManager::ThreeDSoundManager(WorldDir *dir)
    : mParent(dir), mSounds(dir), mListener(dir), unk6c(0), mDopplerPower(1) {}

ThreeDSoundManager::~ThreeDSoundManager() {}

void ThreeDSoundManager::SyncObjects() {
    ObjPtrList<ThreeDSound> sounds(mParent);
    HarvestSounds(mParent, sounds);
    FOREACH (it, sounds) {
        mSounds.remove(*it);
    }
    FOREACH (it, mSounds) {
        (*it)->Stop(nullptr, false);
    }
    mSounds = sounds;
}

void ThreeDSoundManager::HarvestSounds(ObjectDir *dir, ObjPtrList<ThreeDSound> &sounds) {
    MILO_ASSERT(dir, 0x31);
    for (ObjDirItr<ThreeDSound> it(dir, true); it != nullptr; ++it) {
        sounds.push_back(&*it);
        MILO_NOTIFY(
            "Warning, found ThreeDSound object %s in %s!", it->Name(), PathName(dir)
        );
    }
}

float ThreeDSoundManager::CalculateAngle(ThreeDSound *sound, const Transform &xfm) {
    Vector3 vout;
    MultiplyTranspose(sound->WorldXfm().v, xfm, vout);
    return atan2(vout.x, vout.y);
}

void ThreeDSoundManager::CalculateDistance(
    ThreeDSound *sound, const Transform &xfm, float &f1, float &f2
) {
    Vector3 vdiff;
    Subtract(xfm.v, sound->WorldXfm().v, vdiff);
    f1 = Length(vdiff);
    if (IsNaN(f1)) {
        MILO_NOTIFY("Sound %s is NaN meters away", PathName(sound));
        f1 = 0;
        f2 = 0;
    } else if (sound->GetShape() == 1) {
        Vector3 v50;
        Normalize(sound->WorldXfm().m.x, v50);
        Vector3 vneg;
        Negate(v50, vneg);
        float fscalar = Dot(vneg, vdiff);
        Vector3 vtotal;
        ScaleAdd(vdiff, v50, fscalar, vtotal);
        f2 = Length(vtotal);
    }
}

static const float sDopplerFloats[3] = { 340.29f, 1.3348398f, 0.74915355f };

float ThreeDSoundManager::CalculateDoppler(
    ThreeDSound *sound, const Transform &xfm, float f3, float f4, float f5
) {
    Vector3 sub;
    Subtract(xfm.v, unk18.v, sub);
    Vector3 velocity;
    sound->GetVelocity(velocity);
    Vector3 subSound;
    Subtract(xfm.v, sound->WorldXfm().v, subSound);
    float f7 = 0;
    if (f5 != 0) {
        f7 = 1 / f5;
    }
    f7 *= f4;
    float dotSub = Dot(subSound, sub);
    float dotVel = Dot(subSound, velocity);
    float numerator = -(dotSub * f7);
    numerator += sDopplerFloats[0];
    float mantissa = numerator / (dotVel * f7 + sDopplerFloats[0]);
    float powed = pow(mantissa, mDopplerPower);
    return Clamp(sDopplerFloats[2], sDopplerFloats[1], powed);
}

static const int sMaxNumSounds = 100;

void ThreeDSoundManager::Poll() {
    START_AUTO_TIMER("sound_mgr_poll");
    RndTransformable *trans;
    if (mListener) {
        trans = mListener.Ptr();
    } else {
        trans = mParent->Cam();
    }
    if (trans) {
        const Transform &worldXfm = trans->WorldXfm();
        bool xfmEq = worldXfm != unk18;
        float deltaSecs = TheTaskMgr.DeltaSeconds();
        float invDeltaSecs = deltaSecs ? 1 / deltaSecs : 0;
        int numSounds = 0;
        FOREACH (it, mSounds) {
            if ((xfmEq || (*it)->HasMoved() || (*it)->StartedPlaying())
                && (*it)->IsPlaying()) {
                float f1, f2;
                CalculateDistance(*it, worldXfm, f1, f2);
                if ((*it)->Loop() && f1 <= (*it)->GetSilenceDistance()) {
                    if (numSounds == 100) {
                        MILO_NOTIFY_ONCE(
                            "Over %d looping 3D sounds are currently trying to play - ignoring some",
                            sMaxNumSounds
                        );
                        (*it)->SetDistance(FLT_MAX, FLT_MAX);
                    } else {
                        (*it)->SetDistance(f1, f2);
                        numSounds++;
                    }
                } else {
                    (*it)->SetDistance(f1, f2);
                }
                if ((*it)->PanEnabled()) {
                    float angle = CalculateAngle(*it, worldXfm);
                    (*it)->SetAngle(angle);
                }
                if ((*it)->DopplerEnabled() && !unk6c) {
                    float doppler =
                        CalculateDoppler(*it, worldXfm, deltaSecs, invDeltaSecs, f1);
                    (*it)->SetDoppler(doppler);
                }
            }
            (*it)->SaveWorldXfm();
        }
        unk6c = false;
        unk18 = worldXfm;
    }
}
