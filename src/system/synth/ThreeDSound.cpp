#include "synth/ThreeDSound.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"

void ThreeDSound::SaveWorldXfm() { unk1cc = WorldXfm(); }

bool ThreeDSound::HasMoved() { return WorldXfm() != unk1cc; }

ThreeDSound::ThreeDSound()
    : unk194(0), unk195(0), unk198(0), unk19c(0), unk1a0(0), unk1a4(0), unk1a8(0),
      unk1ac(0), unk1b0(2), unk1b4(10), unk1b8(100), unk1bc(1), unk1bd(1), mShape(0),
      mRadius(10), unk20c(100), unk210(0), mDopplerPower(1), unk218(0) {
    Fader *fader = static_cast<Fader *>(Fader::NewObject());
    unk1c8 = fader;
    mFaders.Add(unk1c8);
    CalculateFaderVolume();
    Vector3 v(unk1b4, unk1b8, 1);
    SetLocalScale(this, v);
}

BEGIN_PROPSYNCS(ThreeDSound)
    SYNC_PROP_SET(enable_doppler, unk1bc, bool doppler = _val.Int();
                  unk1c8->SetTranspose(0);
                  unk1bc = doppler)
    SYNC_PROP_SET(enable_pan, unk1bd, EnablePan(_val.Int()))
    SYNC_PROP_SET(falloff_type, unk1ac, unk1ac = _val.Int(); CalculateFaderVolume();)
    SYNC_PROP_SET(falloff_parameter, unk1a8, unk1a8 = _val.Float();
                  CalculateFaderVolume();)
    SYNC_PROP(min_falloff_distance, unk1b0)
    SYNC_PROP(silence_distance, unk1b0)
    SYNC_PROP_SET(shape, mShape, mShape = _val.Int(); CalculateFaderVolume();)
    SYNC_PROP_SET(radius, mRadius, mRadius = _val.Float(); CalculateFaderVolume())
    SYNC_PROP(doppler_power, mDopplerPower)
    SYNC_SUPERCLASS(Sound)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_HANDLERS(ThreeDSound)
    HANDLE_SUPERCLASS(Sound)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_SAVES(ThreeDSound)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(Hmx::Object)

    SAVE_SUPERCLASS(RndTransformable)
    mFaders.Remove(unk1c8);
    SAVE_SUPERCLASS(Sound)
    mFaders.Add(unk1c8);
    bs << unk1ac;
    bs << unk1b0;
    bs << unk1b4;
    bs << unk1b8;

    bs << unk1bc;
    bs << unk1bd;
    bs << mShape;
    bs << mRadius;
    bs << unk195;
    bs << mDopplerPower;

END_SAVES

void ThreeDSound::Highlight() {
    if (mShape < 1) {
        if (mShape != 1) {
            MILO_FAIL("Trying to drawn unknown sound shape %d\n", mShape);
            return;
        }
        WorldXfm();
    } else {
        Hmx::Color fuck;
        UtilDrawSphere(WorldXfm().v, unk1b8, fuck, nullptr);
    }
}

BEGIN_LOADS(ThreeDSound)
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev >= 2) {
        LOAD_SUPERCLASS(RndTransformable)
    }
    LOAD_SUPERCLASS(Sound)
    d >> unk1ac;
    d >> unk1b0;
    d >> unk1b4;
    d >> unk1b8;

    if (d.rev >= 1) {
        d >> unk1bc;
    }
    if (d.rev >= 3) {
        d >> unk1bd;
    }
    if (d.rev >= 4) {
        d >> mShape;
        d >> mRadius;
    }
    if (d.rev >= 5) {
        d >> unk195;
    }
    if (d.rev >= 6) {
        d >> mDopplerPower;
    }
    SetLocalScale(this, Vector3(unk1b4, unk1b8, mRadius));
    CalculateFaderVolume();
END_LOADS
