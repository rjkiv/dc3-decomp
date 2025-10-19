#include "rndobj/PartLauncher.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Poll.h"

RndPartLauncher::RndPartLauncher()
    : mPart(this), mTrans(this), mMeshEmitter(this), mNumParts(0), mEmitRate(0, 0),
      mEmitCount(0) {}

BEGIN_HANDLERS(RndPartLauncher)
    HANDLE_ACTION(launch_particles, LaunchParticles())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndPartLauncher)
    SYNC_PROP_MODIFY(part, mPart, CopyPropsFromPart())
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(num_parts, mNumParts)
    SYNC_PROP(emit_rate, mEmitRate)
    SYNC_PROP_SET(override_life, int(mPartOverride.mask & 1), SetBit(1, _val.Int()))
    SYNC_PROP(life, mPartOverride.life)
    SYNC_PROP_SET(override_speed, int(mPartOverride.mask >> 1 & 1), SetBit(2, _val.Int()))
    SYNC_PROP(speed, mPartOverride.speed)
    SYNC_PROP_SET(override_size, int(mPartOverride.mask >> 2 & 1), SetBit(4, _val.Int()))
    SYNC_PROP(size, mPartOverride.size)
    SYNC_PROP_SET(
        override_delta_size, int(mPartOverride.mask >> 3 & 1), SetBit(8, _val.Int())
    )
    SYNC_PROP(delta_size, mPartOverride.deltaSize)
    SYNC_PROP_SET(
        override_start_color, int(mPartOverride.mask >> 4 & 1), SetBit(0x10, _val.Int())
    )
    SYNC_PROP(start_color, mPartOverride.startColor)
    SYNC_PROP(start_alpha, mPartOverride.startColor.alpha)
    SYNC_PROP_SET(
        override_mid_color, int(mPartOverride.mask >> 5 & 1), SetBit(0x20, _val.Int())
    )
    SYNC_PROP(mid_color, mPartOverride.midColor)
    SYNC_PROP(mid_alpha, mPartOverride.midColor.alpha)
    SYNC_PROP_SET(
        override_end_color, int(mPartOverride.mask >> 6 & 1), SetBit(0x40, _val.Int())
    )
    SYNC_PROP(end_color, mPartOverride.endColor)
    SYNC_PROP(end_alpha, mPartOverride.endColor.alpha)
    SYNC_PROP_SET(
        override_emit_direction,
        int(mPartOverride.mask >> 7 & 1),
        SetBit(0x80, _val.Int())
    )
    SYNC_PROP(pitch_low, mPartOverride.pitch.x)
    SYNC_PROP(pitch_high, mPartOverride.pitch.y)
    SYNC_PROP(yaw_low, mPartOverride.yaw.x)
    SYNC_PROP(yaw_high, mPartOverride.yaw.y)
    SYNC_PROP_SET(
        override_box_emitter, int(mPartOverride.mask >> 9 & 1), SetBit(0x200, _val.Int())
    )
    SYNC_PROP(box_extent_1, mPartOverride.box.mMin)
    SYNC_PROP(box_extent_2, mPartOverride.box.mMax)
    SYNC_PROP_SET(
        override_mesh_emitter, int(mPartOverride.mask >> 8 & 1), SetBit(0x100, _val.Int())
    )
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

BEGIN_LOADS(RndPartLauncher)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 2) {
        LOAD_SUPERCLASS(RndPollable)
        ObjPtr<RndMultiMesh> multiMesh(this);
        bs >> multiMesh;
        if (d.rev > 0) {
            bs >> mPart;
            bs >> mTrans;
            bs >> mNumParts;

            bool bit1;
            d >> bit1;
            SetBit(1, bit1);
            bs >> mPartOverride.life;

            bool bit2;
            d >> bit2;
            SetBit(2, bit2);
            bs >> mPartOverride.speed;

            bool bit4;
            d >> bit4;
            SetBit(4, bit4);
            bs >> mPartOverride.size;

            bool bit8;
            d >> bit8;
            SetBit(8, bit8);
            bs >> mPartOverride.deltaSize;

            bool bit10;
            d >> bit10;
            SetBit(0x10, bit10);
            bs >> mPartOverride.startColor;

            bool bit20;
            d >> bit20;
            SetBit(0x20, bit20);
            bs >> mPartOverride.midColor;

            bool bit40;
            d >> bit40;
            SetBit(0x40, bit40);
            bs >> mPartOverride.endColor;

            bool bit80;
            d >> bit80;
            SetBit(0x80, bit80);
            bs >> mPartOverride.pitch;
            bs >> mPartOverride.yaw;
        }
    } else {
        bs >> mPart;
        bs >> mTrans;
        bs >> mNumParts;
        bs >> mPartOverride.mask;
        bs >> mPartOverride.life;
        bs >> mPartOverride.speed;
        bs >> mPartOverride.size;
        bs >> mPartOverride.deltaSize;
        bs >> mPartOverride.startColor;
        bs >> mPartOverride.midColor;
        bs >> mPartOverride.endColor;
        bs >> mPartOverride.pitch;
        bs >> mPartOverride.yaw;
        if (d.rev > 2) {
            bs >> mPartOverride.box;
            bs >> mMeshEmitter;
        }
        if (d.rev > 3) {
            bs >> mEmitRate;
        }
    }
END_LOADS

void RndPartLauncher::Poll() {
    if (mEmitRate.x > 0.0f || mEmitRate.y > 0.0f) {
        float delta = TheTaskMgr.DeltaSeconds();
        if (delta > 0.0f) {
            float random = RandomFloat(mEmitRate.x, mEmitRate.y);
            mEmitCount += delta * random * 30.0f;
            if (mEmitCount >= 1.0f) {
                int parts = mNumParts;
                // something here with modf
                LaunchParticles();
                mNumParts = parts;
            }
        }
    }
}

void RndPartLauncher::CopyPropsFromPart() {
    if (mPart) {
        if (!(mPartOverride.mask & 1)) {
            mPartOverride.life = Average(mPart->Life());
        }
        if (!(mPartOverride.mask & 2)) {
            mPartOverride.speed = Average(mPart->Speed());
        }
        if (!(mPartOverride.mask & 4)) {
            mPartOverride.size = Average(mPart->StartSize());
        }
        if (!(mPartOverride.mask & 8)) {
            mPartOverride.deltaSize = Average(mPart->DeltaSize());
        }
        if (!(mPartOverride.mask & 0x10)) {
            mPartOverride.startColor = Average(
                mPartOverride.startColor, mPart->StartColorLow(), mPart->StartColorHigh()
            );
        }
        if (!(mPartOverride.mask & 0x20)) {
            mPartOverride.midColor = Average(
                mPartOverride.midColor, mPart->MidColorLow(), mPart->MidColorHigh()
            );
        }
        if (!(mPartOverride.mask & 0x40)) {
            mPartOverride.endColor = Average(
                mPartOverride.endColor, mPart->EndColorLow(), mPart->EndColorHigh()
            );
        }
        if (!(mPartOverride.mask & 0x80)) {
            mPartOverride.pitch = mPart->Pitch();
            mPartOverride.yaw = mPart->Yaw();
        }
        if (!(mPartOverride.mask & 0x100)) {
            mPartOverride.mesh = mPart->GetMesh();
        }
        if (!(mPartOverride.mask & 0x200)) {
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
        mPartOverride.mesh = 0;

        if (mTrans) {
            mPart->SetBoxExtent(box1, box2);
        }
    }
}
