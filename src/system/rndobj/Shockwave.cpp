#include "rndobj/Shockwave.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

RndShockwave *RndShockwave::sSelected = nullptr;
static const float sWavelength = 10;

RndShockwave::RndShockwave()
    : mAutoSelect(0), mRadius(0), mAmplitude(0), mWavelength(sWavelength) {}

BEGIN_HANDLERS(RndShockwave)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndShockwave)
    SYNC_PROP_SET(selected, IsSelected(), SetSelected(_val.Int()))
    SYNC_PROP(auto_select, mAutoSelect)
    SYNC_PROP(radius, mRadius)
    SYNC_PROP(amplitude, mAmplitude)
    SYNC_PROP(wavelength, mWavelength)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndShockwave)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mAutoSelect;
    bs << mRadius;
    bs << mAmplitude;
    bs << mWavelength;
END_SAVES

BEGIN_COPYS(RndShockwave)
    if (this != o) {
        COPY_SUPERCLASS(Hmx::Object)
        COPY_SUPERCLASS(RndTransformable)
        COPY_SUPERCLASS(RndPollable)
        CREATE_COPY(RndShockwave)
        BEGIN_COPYING_MEMBERS
            COPY_MEMBER(mAutoSelect)
            COPY_MEMBER(mRadius)
            COPY_MEMBER(mAmplitude)
            COPY_MEMBER(mWavelength)
        END_COPYING_MEMBERS
    }
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndShockwave)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    LOAD_SUPERCLASS(RndPollable)
    d >> mAutoSelect;
    d >> mRadius;
    d >> mAmplitude;
    d >> mWavelength;
END_LOADS

void RndShockwave::Enter() {
    if (mAutoSelect) {
        sSelected = this;
    }
}

void RndShockwave::Exit() {
    if (sSelected == this) {
        sSelected = nullptr;
    }
}

void RndShockwave::PrepareShader(float iAmplitudeMult) {
    const Vector3 &v = WorldXfm().v;
    Vector4 v1;
    v1.x = v.x;
    v1.y = v.y;
    v1.z = v.z;
    v1.w = 0;
    TheShaderMgr.SetVConstant(kVShader_ShockwaveCenter, v1);
    const Vector3 &mz = WorldXfm().m.z;
    Vector4 v2;
    v2.x = mz.x;
    v2.y = mz.y;
    v2.z = mz.z;
    v2.w = 0;
    TheShaderMgr.SetVConstant(kVShader_ShockwaveAxis, v2);
    TheShaderMgr.SetVConstant(
        kVShader_ShockwaveParams,
        Vector4(mRadius, mAmplitude * iAmplitudeMult, 1.0f / mWavelength, 0)
    );
}
