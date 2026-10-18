#include "rndobj/PartLauncher.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Part.h"
#include "rndobj/Poll.h"

RndPartLauncher::RndPartLauncher()
    : mPart(this), mTrans(this), mMeshEmitter(this), mNumParts(0), mEmitRate(0, 0),
      mEmitCount(0) {}

BEGIN_HANDLERS(RndPartLauncher)
    HANDLE_ACTION(launch_particles, LaunchParticles())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#define SYNC_PROP_OVERRIDE(name, mask)                                                   \
    SYNC_PROP_SET(name, GetBit(mask), SetBit(mask, _val.Int()))

BEGIN_PROPSYNCS(RndPartLauncher)
    SYNC_PROP_MODIFY(part, mPart, CopyPropsFromPart())
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(num_parts, mNumParts)
    SYNC_PROP(emit_rate, mEmitRate)
    SYNC_PROP_OVERRIDE(override_life, PartOverride::kLife)
    SYNC_PROP(life, mPartOverride.life)
    SYNC_PROP_OVERRIDE(override_speed, PartOverride::kSpeed)
    SYNC_PROP(speed, mPartOverride.speed)
    SYNC_PROP_OVERRIDE(override_size, PartOverride::kSize)
    SYNC_PROP(size, mPartOverride.size)
    SYNC_PROP_OVERRIDE(override_delta_size, PartOverride::kDeltaSize)
    SYNC_PROP(delta_size, mPartOverride.deltaSize)
    SYNC_PROP_OVERRIDE(override_start_color, PartOverride::kStartColor)
    SYNC_PROP(start_color, mPartOverride.startColor)
    SYNC_PROP(start_alpha, mPartOverride.startColor.alpha)
    SYNC_PROP_OVERRIDE(override_mid_color, PartOverride::kMidColor)
    SYNC_PROP(mid_color, mPartOverride.midColor)
    SYNC_PROP(mid_alpha, mPartOverride.midColor.alpha)
    SYNC_PROP_OVERRIDE(override_end_color, PartOverride::kEndColor)
    SYNC_PROP(end_color, mPartOverride.endColor)
    SYNC_PROP(end_alpha, mPartOverride.endColor.alpha)
    SYNC_PROP_OVERRIDE(override_emit_direction, PartOverride::kEmitDirection)
    SYNC_PROP(pitch_low, mPartOverride.pitch.x)
    SYNC_PROP(pitch_high, mPartOverride.pitch.y)
    SYNC_PROP(yaw_low, mPartOverride.yaw.x)
    SYNC_PROP(yaw_high, mPartOverride.yaw.y)
    SYNC_PROP_OVERRIDE(override_box_emitter, PartOverride::kBoxEmitter)
    SYNC_PROP(box_extent_1, mPartOverride.box.mMin)
    SYNC_PROP(box_extent_2, mPartOverride.box.mMax)
    SYNC_PROP_OVERRIDE(override_mesh_emitter, PartOverride::kMeshEmitter)
    SYNC_PROP(mesh, mMeshEmitter)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndPartLauncher)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPart;
    bs << mTrans;
    bs << mNumParts;
    bs << mPartOverride.mask;
    bs << mPartOverride.life;
    bs << mPartOverride.speed;
    bs << mPartOverride.size;
    bs << mPartOverride.deltaSize;
    bs << mPartOverride.startColor;
    bs << mPartOverride.midColor;
    bs << mPartOverride.endColor;
    bs << mPartOverride.pitch;
    bs << mPartOverride.yaw;
    bs << mPartOverride.box;
    bs << mMeshEmitter;
    bs << mEmitRate;
END_SAVES

BEGIN_COPYS(RndPartLauncher)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndPartLauncher)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPart)
        COPY_MEMBER(mTrans)
        COPY_MEMBER(mNumParts)
        COPY_MEMBER(mEmitRate)
        mPartOverride = c->mPartOverride;
        COPY_MEMBER(mMeshEmitter)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BEGIN_LOADS(RndPartLauncher)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 2) {
        LOAD_SUPERCLASS(RndPollable)
        ObjPtr<RndMultiMesh> multiMesh(this);
        d >> multiMesh;
        if (d.rev > 0) {
            d >> mPart;
            d >> mTrans;
            d >> mNumParts;

            bool bit1;
            d >> bit1;
            SetBit(PartOverride::kLife, bit1);
            d >> mPartOverride.life;

            bool bit2;
            d >> bit2;
            SetBit(PartOverride::kSpeed, bit2);
            d >> mPartOverride.speed;

            bool bit4;
            d >> bit4;
            SetBit(PartOverride::kSize, bit4);
            d >> mPartOverride.size;

            bool bit8;
            d >> bit8;
            SetBit(PartOverride::kDeltaSize, bit8);
            d >> mPartOverride.deltaSize;

            bool bit10;
            d >> bit10;
            SetBit(PartOverride::kStartColor, bit10);
            d >> mPartOverride.startColor;

            bool bit20;
            d >> bit20;
            SetBit(PartOverride::kMidColor, bit20);
            d >> mPartOverride.midColor;

            bool bit40;
            d >> bit40;
            SetBit(PartOverride::kEndColor, bit40);
            d >> mPartOverride.endColor;

            bool bit80;
            d >> bit80;
            SetBit(PartOverride::kEmitDirection, bit80);
            d >> mPartOverride.pitch;
            d >> mPartOverride.yaw;
        }
    } else {
        d >> mPart;
        d >> mTrans;
        d >> mNumParts;
        d >> mPartOverride.mask;
        d >> mPartOverride.life;
        d >> mPartOverride.speed;
        d >> mPartOverride.size;
        d >> mPartOverride.deltaSize;
        d >> mPartOverride.startColor;
        d >> mPartOverride.midColor;
        d >> mPartOverride.endColor;
        d >> mPartOverride.pitch;
        d >> mPartOverride.yaw;
        if (d.rev > 2) {
            d >> mPartOverride.box;
            d >> mMeshEmitter;
        }
        if (d.rev > 3) {
            d >> mEmitRate;
        }
    }
END_LOADS

void RndPartLauncher::Poll() {
    if (mEmitRate.x > 0.0f || mEmitRate.y > 0.0f) {
        float delta = TheTaskMgr.DeltaSeconds();
        if (delta > 0.0f) {
            mEmitCount += delta * RandomFloat(mEmitRate.x, mEmitRate.y) * 30.0f;
            if (mEmitCount >= 1) {
                int parts = mNumParts;
                double x;
                mEmitCount = modf(mEmitCount, &x);
                mNumParts = (float)x;
                LaunchParticles();
                mNumParts = parts;
            }
        }
    }
}

void RndPartLauncher::CopyPropsFromPart() {
    if (mPart) {
        if (!(mPartOverride.mask & PartOverride::kLife)) {
            mPartOverride.life = Average(mPart->Life());
        }
        if (!(mPartOverride.mask & PartOverride::kSpeed)) {
            mPartOverride.speed = Average(mPart->Speed());
        }
        if (!(mPartOverride.mask & PartOverride::kSize)) {
            mPartOverride.size = Average(mPart->StartSize());
        }
        if (!(mPartOverride.mask & PartOverride::kDeltaSize)) {
            mPartOverride.deltaSize = Average(mPart->DeltaSize());
        }
        if (!(mPartOverride.mask & PartOverride::kStartColor)) {
            mPartOverride.startColor = Average(
                mPartOverride.startColor, mPart->StartColorLow(), mPart->StartColorHigh()
            );
        }
        if (!(mPartOverride.mask & PartOverride::kMidColor)) {
            mPartOverride.midColor = Average(
                mPartOverride.midColor, mPart->MidColorLow(), mPart->MidColorHigh()
            );
        }
        if (!(mPartOverride.mask & PartOverride::kEndColor)) {
            mPartOverride.endColor = Average(
                mPartOverride.endColor, mPart->EndColorLow(), mPart->EndColorHigh()
            );
        }
        if (!(mPartOverride.mask & PartOverride::kEmitDirection)) {
            mPartOverride.pitch = mPart->Pitch();
            mPartOverride.yaw = mPart->Yaw();
        }
        if (!(mPartOverride.mask & PartOverride::kMeshEmitter)) {
            mPartOverride.mesh = mPart->GetMesh();
        }
        if (!(mPartOverride.mask & PartOverride::kBoxEmitter)) {
            mPartOverride.box.mMin = mPart->BoxExtent1();
            mPartOverride.box.mMax = mPart->BoxExtent2();
        }
    }
}

void RndPartLauncher::LaunchParticles() {
    if (mPart) {
        Vector3 box1(mPart->BoxExtent1());
        Vector3 box2(mPart->BoxExtent2());
        if (mTrans) {
            Vector3 partvec(mPart->WorldXfm().v);
            Vector3 transvec(mTrans->WorldXfm().v);

            transvec -= partvec;
            Vector3 sumvec(box1);
            Vector3 sumvec2(box2);
            sumvec += transvec;
            sumvec2 += transvec;
            mPart->SetBoxExtent(sumvec, sumvec2);
        }

        mPartOverride.mesh = mMeshEmitter;
        mPart->ExplicitParticles(mNumParts, true, mPartOverride);
        mPartOverride.mesh = nullptr;

        if (mTrans) {
            mPart->SetBoxExtent(box1, box2);
        }
    }
}
