#include "rndobj/PartAnim.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Part.h"

#pragma region Hmx::Object

RndParticleSysAnim::RndParticleSysAnim() : mParticleSys(this), mKeysOwner(this, this) {}

bool RndParticleSysAnim::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mParticleSys == from) {
        if (mKeysOwner != this) {
            mKeysOwner = this;
        } else {
            RndParticleSysAnim *sysTo = dynamic_cast<RndParticleSysAnim *>(to);
            if (sysTo) {
                mKeysOwner = sysTo->mKeysOwner;
            }
        }
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndParticleSysAnim)
    HANDLE_ACTION(set_particle_sys, SetParticleSys(_msg->Obj<RndParticleSys>(2)))
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndParticleSysAnim)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndParticleSysAnim)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mParticleSys << mStartColorKeys << mEndColorKeys << mEmitRateKeys;
    bs << mKeysOwner << mSpeedKeys << mLifeKeys << mStartSizeKeys;
END_SAVES

BEGIN_COPYS(RndParticleSysAnim)
    CREATE_COPY_AS(RndParticleSysAnim, l)
    MILO_ASSERT(l, 0x7E);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(l, mParticleSys)
    if (ty == kCopyShallow || ty == kCopyFromMax && l->mKeysOwner != l) {
        COPY_MEMBER_FROM(l, mKeysOwner)
    } else {
        mKeysOwner = this;
        mStartColorKeys = l->mKeysOwner->mStartColorKeys;
        mEndColorKeys = l->mKeysOwner->mEndColorKeys;
        mEmitRateKeys = l->mKeysOwner->mEmitRateKeys;
        mSpeedKeys = l->mKeysOwner->mSpeedKeys;
        mLifeKeys = l->mKeysOwner->mLifeKeys;
        mStartSizeKeys = l->mKeysOwner->mStartSizeKeys;
    }
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(RndParticleSysAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    if (d.rev > 2) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndAnimatable)
    d >> mParticleSys >> mStartColorKeys >> mEndColorKeys;
    if (d.rev < 2) {
        float scale = 1;
        Keys<float, float> floatKeys;
        d >> floatKeys >> mKeysOwner;
        if (d.rev == 1) {
            d >> scale;
        }
        mEmitRateKeys.clear();
        mEmitRateKeys.resize(floatKeys.size());
        FOREACH (it, floatKeys) {
            Key<Vector2> vecKey;
            vecKey.value = Vector2(it->value, it->value * scale);
            mEmitRateKeys.push_back(vecKey);
        }
    } else {
        d >> mEmitRateKeys >> mKeysOwner;
    }
    if (!mKeysOwner)
        mKeysOwner = this;
    if (d.rev > 1) {
        d >> mSpeedKeys >> mLifeKeys >> mStartSizeKeys;
    }
END_LOADS

void RndParticleSysAnim::Print() {
    TheDebug << "   particleSys: " << mParticleSys << "\n";
    TheDebug << "   framesOwner: " << mKeysOwner << "\n";
    TheDebug << "   startColorKeys: " << mStartColorKeys << "\n";
    TheDebug << "   endColorKeys: " << mEndColorKeys << "\n";
    TheDebug << "   emitRateKeys: " << mEmitRateKeys << "\n";
    TheDebug << "   speedKeys: " << mSpeedKeys << "\n";
    TheDebug << "   startSizeKeys: " << mStartSizeKeys << "\n";
    TheDebug << "   lifeKeys: " << mLifeKeys << "\n";
}

#pragma endregion
#pragma region RndAnimatable

void RndParticleSysAnim::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mParticleSys) {
        if (!StartColorKeys().empty()) {
            Hmx::Color colorlow(mParticleSys->StartColorLow());
            Hmx::Color colorhigh(mParticleSys->StartColorHigh());
            StartColorKeys().AtFrame(frame, colorlow);
            Add(colorlow, mParticleSys->StartColorHigh(), colorhigh);
            Subtract(colorhigh, mParticleSys->StartColorLow(), colorhigh);
            if (blend != 1.0f) {
                Interp(mParticleSys->StartColorLow(), colorlow, blend, colorlow);
                Interp(mParticleSys->StartColorHigh(), colorhigh, blend, colorhigh);
            }
            mParticleSys->SetStartColor(colorlow, colorhigh);
        }
        if (!EndColorKeys().empty()) {
            Hmx::Color colorlow(mParticleSys->EndColorLow());
            Hmx::Color colorhigh(mParticleSys->EndColorHigh());
            EndColorKeys().AtFrame(frame, colorlow);
            Add(colorlow, mParticleSys->EndColorHigh(), colorhigh);
            Subtract(colorhigh, mParticleSys->EndColorLow(), colorhigh);
            if (blend != 1.0f) {
                Interp(mParticleSys->StartColorLow(), colorlow, blend, colorlow);
                Interp(mParticleSys->StartColorHigh(), colorhigh, blend, colorhigh);
            }
            mParticleSys->SetEndColor(colorlow, colorhigh);
        }
        if (!EmitRateKeys().empty()) {
            Vector2 rate(mParticleSys->EmitRate());
            EmitRateKeys().AtFrame(frame, rate);
            if (blend != 1.0f) {
                Interp(mParticleSys->EmitRate(), rate, blend, rate);
            }
            mParticleSys->SetEmitRate(rate.x, rate.y);
        }
        if (!SpeedKeys().empty()) {
            Vector2 speed(mParticleSys->Speed());
            SpeedKeys().AtFrame(frame, speed);
            if (blend != 1.0f) {
                Interp(mParticleSys->Speed(), speed, blend, speed);
            }
            mParticleSys->SetSpeed(speed.x, speed.y);
        }
        if (!LifeKeys().empty()) {
            Vector2 life(mParticleSys->Life());
            LifeKeys().AtFrame(frame, life);
            if (blend != 1.0f) {
                Interp(mParticleSys->Life(), life, blend, life);
            }
            mParticleSys->SetLife(life.x, life.y);
        }
        if (!StartSizeKeys().empty()) {
            Vector2 startsize(mParticleSys->StartSize());
            StartSizeKeys().AtFrame(frame, startsize);
            if (blend != 1.0f) {
                Interp(mParticleSys->StartSize(), startsize, blend, startsize);
            }
            mParticleSys->SetStartSize(startsize.x, startsize.y);
        }
    }
}

float RndParticleSysAnim::EndFrame() {
    float last =
        Max(StartColorKeys().LastFrame(),
            EndColorKeys().LastFrame(),
            EmitRateKeys().LastFrame());
    last = Max(last, SpeedKeys().LastFrame(), LifeKeys().LastFrame());
    last = Max(last, StartSizeKeys().LastFrame());
    return last;
}

void RndParticleSysAnim::SetKey(float frame) {
    if (mParticleSys) {
        StartColorKeys().Add(mParticleSys->StartColorLow(), frame, true);
        EndColorKeys().Add(mParticleSys->EndColorLow(), frame, true);
        EmitRateKeys().Add(mParticleSys->EmitRate(), frame, true);
        SpeedKeys().Add(mParticleSys->Speed(), frame, true);
        LifeKeys().Add(mParticleSys->Life(), frame, true);
        StartSizeKeys().Add(mParticleSys->StartSize(), frame, true);
    }
}

#pragma endregion
#pragma region RndParticleSysAnim

void RndParticleSysAnim::SetParticleSys(RndParticleSys *sys) { mParticleSys = sys; }
