#include "rndobj/Part.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

PartOverride gNoPartOverride;
ParticleCommonPool *gParticlePool = nullptr;

namespace {
    int ParticlePoolSize() {
        return SystemConfig("rnd", "particlesys", "global_limit")->Int(1);
    }

    DataNode PrintParticlePoolSize(DataArray *) {
        MILO_LOG("Particle Pool Size:\n");
        if (gParticlePool) {
            int size = ParticlePoolSize();
            MILO_LOG(
                "   %d particles can be allocated, %.1f KB.\n",
                size,
                size * sizeof(RndFancyParticle) / 1024.0f
            );
            MILO_LOG(
                "   %d particles active, %d is the high water mark.\n",
                gParticlePool->NumUsed(),
                gParticlePool->HighWaterMark()
            );
            MILO_LOG(
                "   Adding 30%%, suggesting a particle global limit of %d (set in default.dta).\n",
                (int)(gParticlePool->HighWaterMark() * 1.3f)
            );
        }
        return 0;
    }
}

void InitParticleSystem() {
    if (!gParticlePool) {
        gParticlePool = new ParticleCommonPool();
    }
    if (gParticlePool) {
        gParticlePool->InitPool();
    }
    DataRegisterFunc("print_particle_pool_size", PrintParticlePoolSize);
}

BinStream &operator<<(BinStream &bs, const RndParticle &p) {
    bs << p.pos << p.col << p.size;
    return bs;
}

BinStream &operator>>(BinStream &bs, RndParticle &p) {
    bs >> p.pos >> p.col >> p.size;
    return bs;
}

bool RndParticleSys::Burst::Set(float f1, float f2) {
    if (f2 > 0) {
        mPeak = f1;
        mHalfLength = f2 / 2;
        mNorm = 1 / mHalfLength;
        mTimeToLive = f2;
        return true;
    } else {
        return false;
    }
}

float RndParticleSys::Burst::Emit(float f1) {
    mTimeToLive -= f1;
    if (mTimeToLive < 0) {
        return -1;
    }
    float fvar1 = mTimeToLive;
    if (fvar1 > mHalfLength) {
        fvar1 = mHalfLength * 2.0f - fvar1;
    }
    fvar1 *= mNorm;
    return (fvar1 * fvar1 * 3.0f - fvar1 * fvar1 * fvar1 * 2.0f) * mPeak * f1;
}

void ParticleCommonPool::InitPool() {
    int size = ParticlePoolSize();
    mPoolParticles = new RndFancyParticle[size];
    for (int i = 0; i < size - 1; i++) {
        mPoolParticles[i].prev = nullptr;
        mPoolParticles[i].next = &mPoolParticles[i + 1];
    }
    mPoolParticles[size - 1].prev = nullptr;
    mPoolParticles[size - 1].next = nullptr;
    mPoolFreeParticles = mPoolParticles;
}

RndParticle *ParticleCommonPool::AllocateParticle() {
    RndParticle *cur = mPoolFreeParticles;
    RndParticle *ret = nullptr;
    if (mPoolFreeParticles) {
        mPoolFreeParticles = (RndFancyParticle *)mPoolFreeParticles->next;
        cur->prev = cur;
        mNumUsed++;
        ret = cur;
        if (mNumUsed > mHighWaterMark) {
            mHighWaterMark = mNumUsed;
        }
    }
    return ret;
}

RndParticle *ParticleCommonPool::FreeParticle(RndParticle *p) {
    if (!p)
        return nullptr;
    else {
        RndParticle *ret = p->next;
        p->next = mPoolFreeParticles;
        p->prev = nullptr;
        mPoolFreeParticles = (RndFancyParticle *)p;
        mNumUsed--;
        return ret;
    }
}

BEGIN_CUSTOM_PROPSYNC(Attractor)
    SYNC_PROP(attractor, o.mAttractor)
    SYNC_PROP(strength, o.mStrength)
END_CUSTOM_PROPSYNC

BinStream &operator<<(BinStream &bs, const Attractor &a) {
    a.Save(bs);
    return bs;
}

void Attractor::Save(BinStream &bs) const {
    bs << mAttractor;
    bs << mStrength;
}

BinStreamRev &operator>>(BinStreamRev &d, Attractor &a) {
    a.Load(d);
    return d;
}

void Attractor::Load(BinStreamRev &d) {
    d >> mAttractor;
    d >> mStrength;
}

#pragma region RndParticleSys

RndParticleSys::RndParticleSys()
    : mType(kBasic), mMaxParticles(0), mPersistentParticles(nullptr),
      mFreeParticles(nullptr), mActiveParticles(nullptr), mNumActive(0), mEmitCount(0),
      mFrameDrive(0), mLastFrame(0), mPollsSinceDraw(0), mPauseOffscreen(0),
      mSleepTime(0), mBubblePeriod(10, 10), mBubbleSize(1, 1), mLife(100, 100),
      mPosLow(0, 0, 0), mPosHigh(0, 0, 0), mSpeed(1, 1), mPitch(0, 0), mYaw(0, 0),
      mEmitRate(1, 1), mStartSize(gUnitsPerMeter / 4, gUnitsPerMeter / 4),
      mDeltaSize(0, 0), mStartColorLow(1, 1, 1), mStartColorHigh(1, 1, 1),
      mEndColorLow(1, 1, 1), mEndColorHigh(1, 1, 1), mMesh(this), mMat(this),
      mPreserveParticles(0), mMotionParent(this), mBounce(this), mForce(0, 0, 0),
      mDrag(0), mBubble(0), mFastForward(0), mNeedForward(0), mRotate(0), mRotSpeed(0, 0),
      mRotDrag(0), mRotRandomDir(1), mSwingArmStart(0, 0), mSwingArmEnd(0, 0),
      mAlignWithVelocity(0), mStretchWithVelocity(0), mConstantArea(0),
      mPerspectiveStretch(0), mStretchScale(1), mHeightRatio(1), mSubSamples(0),
      mGrowRatio(0), mShrinkRatio(1), mMidColorRatio(0.5), mMidColorLow(1, 1, 1),
      mMidColorHigh(1, 1, 1), mBirthMomentum(0), mBirthMomentumAmount(1), mMaxBurst(0),
      mTimeTillBurst(0), mBurstInterval(15, 35), mBurstPeak(4, 8), mBurstLength(20, 30),
      mExplicitParts(0), mElapsedTime(0), mAnimatedUVs(0), mLoopAnim(1), mRandomStart(0),
      mTileAnimSpeed(0), mTilesAcross(1), mTilesDown(1), mNumTiles(1), mFirstUsedTile(0),
      mLoopSpeed(1), mInvLoopSpeed(1), mAttractors(this) {
    SetRelativeMotion(0, this);
    SetSubSamples(0);
}

RndParticleSys::~RndParticleSys() {
    if (mPreserveParticles) {
        if (mPersistentParticles)
            delete[] mPersistentParticles;
    } else if (mActiveParticles) {
        for (RndParticle *p = mActiveParticles; p != nullptr; p = FreeParticle(p))
            ;
    }
}

bool RndParticleSys::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mMotionParent) {
        SetRelativeMotion(mRelativeMotion, dynamic_cast<RndTransformable *>(to));
        return true;
    } else {
        return RndTransformable::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndParticleSys)
    HANDLE_EXPR(hi_emit_rate, Max(mEmitRate.x, mEmitRate.y))
    HANDLE(set_start_color, OnSetStartColor)
    HANDLE(set_end_color, OnSetEndColor)
    HANDLE(set_start_color_int, OnSetStartColorInt)
    HANDLE(set_end_color_int, OnSetEndColorInt)
    HANDLE(set_emit_rate, OnSetEmitRate)
    HANDLE(set_burst_interval, OnSetBurstInterval)
    HANDLE(set_burst_peak, OnSetBurstPeak)
    HANDLE(set_burst_length, OnSetBurstLength)
    HANDLE(add_emit_rate, OnAddEmitRate)
    HANDLE(launch_part, OnExplicitPart)
    HANDLE(launch_parts, OnExplicitParts)
    HANDLE(set_life, OnSetLife)
    HANDLE(set_speed, OnSetSpeed)
    HANDLE(set_rotate, OnSetRotate)
    HANDLE(set_swing_arm, OnSetSwingArm)
    HANDLE(set_drag, OnSetDrag)
    HANDLE(set_alignment, OnSetAlignment)
    HANDLE(set_start_size, OnSetStartSize)
    HANDLE(set_mat, OnSetMat)
    HANDLE(set_pos, OnSetPos)
    HANDLE_ACTION(set_mesh, SetMesh(_msg->Obj<RndMesh>(2)))
    HANDLE(active_particles, OnActiveParticles)
    HANDLE_EXPR(max_particles, mMaxParticles)
    HANDLE_ACTION(
        set_relative_parent,
        SetRelativeMotion(mRelativeMotion, _msg->Obj<RndTransformable>(2))
    )
    HANDLE_ACTION(clear_all_particles, FreeAllParticles())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool AngleVectorSync(Vector2 &vec, DataNode &_val, DataArray *_prop, int _i, PropOp _op) {
    if (_i == _prop->Size())
        return true;
    else {
        Symbol sym = _prop->Sym(_i);
        static Symbol x("x");
        static Symbol y("y");
        float *coord = nullptr;
        if (sym == x) {
            coord = &vec.x;
        } else if (sym == y) {
            coord = &vec.y;
        } else {
            return false;
        }
        if (_op == kPropSet)
            *coord = DegreesToRadians(_val.Float());
        else if (_op == kPropGet)
            _val = RadiansToDegrees(*coord);
        else
            return false;
    }
    return true;
}

BEGIN_PROPSYNCS(RndParticleSys)
    SYNC_PROP(mat, mMat)
    SYNC_PROP_SET(animate_uvs, mAnimatedUVs, SetAnimatedUV(_val.Int()))
    SYNC_PROP(loop_uv_anim, mLoopAnim)
    SYNC_PROP(random_anim_start, mRandomStart)
    SYNC_PROP_SET(tile_hold_time, mTileAnimSpeed, SetTileHoldTime(_val.Float()))
    SYNC_PROP_SET(num_tiles_across, mTilesAcross, mTilesAcross = Max(_val.Int(), 1))
    SYNC_PROP_SET(num_tiles_down, mTilesDown, mTilesDown = Max(_val.Int(), 1))
    SYNC_PROP_SET(num_tiles_total, mNumTiles, SetNumTiles(_val.Int()))
    SYNC_PROP(starting_tile, mFirstUsedTile)
    SYNC_PROP_SET(max_parts, mMaxParticles, SetPool(_val.Int(), mType))
    SYNC_PROP(emit_rate, mEmitRate)
    SYNC_PROP(screen_aspect, mHeightRatio)
    SYNC_PROP(life, mLife)
    SYNC_PROP(speed, mSpeed)
    SYNC_PROP(start_size, mStartSize)
    SYNC_PROP(delta_size, mDeltaSize)
    SYNC_PROP(force_dir, mForce)
    SYNC_PROP(bounce, mBounce)
    SYNC_PROP(start_color_low, mStartColorLow)
    SYNC_PROP(start_color_high, mStartColorHigh)
    SYNC_PROP(start_alpha_low, mStartColorLow.alpha)
    SYNC_PROP(start_alpha_high, mStartColorHigh.alpha)
    SYNC_PROP(end_color_low, mEndColorLow)
    SYNC_PROP(end_color_high, mEndColorHigh)
    SYNC_PROP(end_alpha_low, mEndColorLow.alpha)
    SYNC_PROP(end_alpha_high, mEndColorHigh.alpha)
    SYNC_PROP(preserve, mPreserveParticles)
    SYNC_PROP_SET(fancy, mType, SetPool(mMaxParticles, (Type)_val.Int()))
    // SYNC_PROP_SET(grow_ratio, mGrowRatio,SetGrowRatio(_val.Float()))
    {
        static Symbol _s("grow_ratio");
        if (sym == _s) {
            if (_op == kPropSet) {
                float f = _val.Float();
                if (f >= 0 && f <= mShrinkRatio) {
                    mGrowRatio = f;
                }
            } else {
                if (_op == (PropOp)0x40)
                    return false;
                _val = mGrowRatio;
            }
            return true;
        }
    }
    SYNC_PROP_SET(shrink_ratio, mShrinkRatio, SetShrinkRatio(_val.Float()))
    SYNC_PROP(drag, mDrag)
    SYNC_PROP(mid_color_ratio, mMidColorRatio)
    SYNC_PROP(mid_color_low, mMidColorLow)
    SYNC_PROP(mid_color_high, mMidColorHigh)
    SYNC_PROP(mid_alpha_low, mMidColorLow.alpha)
    SYNC_PROP(mid_alpha_high, mMidColorHigh.alpha)
    SYNC_PROP(bubble, mBubble)
    SYNC_PROP(bubble_period, mBubblePeriod)
    SYNC_PROP(bubble_size, mBubbleSize)
    SYNC_PROP(max_burst, mMaxBurst)
    SYNC_PROP(time_between, mBurstInterval)
    SYNC_PROP(peak_rate, mBurstPeak)
    SYNC_PROP(duration, mBurstLength)
    SYNC_PROP(spin, mRotate)
    SYNC_PROP(rpm, mRotSpeed)
    SYNC_PROP(rpm_drag, mRotDrag)
    SYNC_PROP(start_offset, mSwingArmStart)
    SYNC_PROP(end_offset, mSwingArmEnd)
    SYNC_PROP(random_direction, mRotRandomDir)
    SYNC_PROP(velocity_align, mAlignWithVelocity)
    SYNC_PROP(stretch_with_velocity, mStretchWithVelocity)
    SYNC_PROP(stretch_scale, mStretchScale)
    SYNC_PROP(constant_area, mConstantArea)
    SYNC_PROP(perspective, mPerspectiveStretch)
    SYNC_PROP_SET(mesh_emitter, mMesh.Ptr(), SetMesh(_val.Obj<RndMesh>()))
    SYNC_PROP(box_extent_1, mPosLow)
    SYNC_PROP(box_extent_2, mPosHigh) {
        static Symbol _s("pitch");
        if (sym == _s) {
            AngleVectorSync(mPitch, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    {
        static Symbol _s("yaw");
        if (sym == _s) {
            AngleVectorSync(mYaw, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    SYNC_PROP_SET(
        motion_parent,
        mMotionParent.Ptr(),
        SetRelativeMotion(mRelativeMotion, _val.Obj<RndTransformable>())
    )
    SYNC_PROP_SET(
        relative_motion, mRelativeMotion, SetRelativeMotion(_val.Float(), mMotionParent)
    )
    SYNC_PROP_SET(subsamples, mSubSamples, SetSubSamples(_val.Int()))
    SYNC_PROP_SET(frame_drive, mFrameDrive, SetFrameDrive(_val.Int()))
    SYNC_PROP(pre_spawn, mFastForward)
    SYNC_PROP_SET(pause_offscreen, mPauseOffscreen, SetPauseOffscreen(_val.Int()))
    SYNC_PROP(attractors, mAttractors)
    SYNC_PROP(birth_momentum, mBirthMomentum)
    SYNC_PROP(birth_momentum_amount, mBirthMomentumAmount)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndParticleSys)
    SAVE_REVS(0x29, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndPollable)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mLife;
    bs << mHeightRatio;
    bs << mPosLow;
    bs << mPosHigh;
    bs << mSpeed;
    bs << mPitch;
    bs << mYaw;
    bs << mEmitRate;
    bs << mMaxBurst;
    bs << mBurstInterval;
    bs << mBurstPeak;
    bs << mBurstLength;
    bs << mStartSize;
    bs << mDeltaSize;
    bs << mStartColorLow;
    bs << mStartColorHigh;
    bs << mEndColorLow;
    bs << mEndColorHigh;
    bs << mBounce;
    bs << mForce;
    bs << mMat;
    bs << mType;
    bs << mGrowRatio;
    bs << mShrinkRatio;
    bs << mMidColorRatio;
    bs << mMidColorLow;
    bs << mMidColorHigh;
    bs << mMaxParticles;
    bs << mBubblePeriod;
    bs << mBubbleSize;
    bs << mBubble;
    bs << mRotate;
    bs << mRotSpeed;
    bs << mRotDrag;
    bs << mRotRandomDir;
    bs << mDrag;
    bs << mSwingArmStart;
    bs << mSwingArmEnd;
    bs << mAlignWithVelocity;
    bs << mStretchWithVelocity;
    bs << mConstantArea;
    bs << mStretchScale;
    bs << mPerspectiveStretch;
    bs << mRelativeMotion;
    bs << mMotionParent;
    bs << mMesh;
    bs << mSubSamples;
    bs << mFrameDrive;
    bs << mPauseOffscreen;
    bs << mFastForward;
    bs << mAnimatedUVs;
    bs << mTileAnimSpeed;
    bs << mTilesAcross;
    bs << mTilesDown;
    bs << mNumTiles;
    bs << mFirstUsedTile;
    bs << mLoopAnim;
    bs << mRandomStart;
    bs << mAttractors;
    bs << mBirthMomentum;
    bs << mBirthMomentumAmount;
    bs << mPreserveParticles;
    mNeedForward = mFastForward;
    if (mPreserveParticles) {
        bs << mNumActive;
        for (RndParticle *p = mActiveParticles; p != nullptr; p = p->next) {
            bs << *p;
        }
    }
END_SAVES

BEGIN_COPYS(RndParticleSys)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndPollable)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndParticleSys)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPreserveParticles)
        if (mPreserveParticles) {
            SetPool(c->mMaxParticles, c->mType);
            for (RndParticle *p = c->mActiveParticles; p != nullptr; p = p->next) {
                RndParticle *alloced = AllocParticle();
                if (!alloced)
                    break;
                RndParticle *next = alloced->next;
                RndParticle *prev = alloced->prev;
                *alloced = *p;
                alloced->next = next;
                alloced->prev = prev;
            }
        }
        COPY_MEMBER(mNumActive)
        mLastFrame = Frame();
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mLife)
            COPY_MEMBER(mHeightRatio)
            COPY_MEMBER(mPosLow)
            COPY_MEMBER(mPosHigh)
            COPY_MEMBER(mSpeed)
            COPY_MEMBER(mPitch)
            COPY_MEMBER(mYaw)
            COPY_MEMBER(mEmitRate)
            COPY_MEMBER(mMaxBurst)
            COPY_MEMBER(mBurstInterval)
            COPY_MEMBER(mBurstPeak)
            COPY_MEMBER(mBurstLength)
            COPY_MEMBER(mStartSize)
            COPY_MEMBER(mDeltaSize)
            COPY_MEMBER(mStartColorLow)
            COPY_MEMBER(mStartColorHigh)
            COPY_MEMBER(mEndColorLow)
            COPY_MEMBER(mEndColorHigh)
            COPY_MEMBER(mBounce)
            COPY_MEMBER(mForce)
            COPY_MEMBER(mMat)
            COPY_MEMBER(mBubblePeriod)
            COPY_MEMBER(mBubbleSize)
            COPY_MEMBER(mBubble)
            COPY_MEMBER(mRotate)
            COPY_MEMBER(mRotSpeed)
            COPY_MEMBER(mRotDrag)
            COPY_MEMBER(mRotRandomDir)
            COPY_MEMBER(mDrag)
            COPY_MEMBER(mSwingArmStart)
            COPY_MEMBER(mSwingArmEnd)
            COPY_MEMBER(mAlignWithVelocity)
            COPY_MEMBER(mStretchWithVelocity)
            COPY_MEMBER(mConstantArea)
            COPY_MEMBER(mPerspectiveStretch)
            COPY_MEMBER(mStretchScale)
            COPY_MEMBER(mFastForward)
            mNeedForward = mFastForward;
            COPY_MEMBER(mGrowRatio)
            COPY_MEMBER(mShrinkRatio)
            COPY_MEMBER(mMidColorRatio)
            COPY_MEMBER(mMidColorLow)
            COPY_MEMBER(mMidColorHigh)
            COPY_MEMBER(mMesh)
            COPY_MEMBER(mFrameDrive)
            COPY_MEMBER(mPauseOffscreen)
            mElapsedTime = mSleepTime = 0;
            COPY_MEMBER(mAnimatedUVs)
            COPY_MEMBER(mLoopAnim)
            COPY_MEMBER(mRandomStart)
            COPY_MEMBER(mTileAnimSpeed)
            COPY_MEMBER(mTilesAcross)
            COPY_MEMBER(mTilesDown)
            COPY_MEMBER(mNumTiles)
            COPY_MEMBER(mFirstUsedTile)
            COPY_MEMBER(mLoopSpeed)
            COPY_MEMBER(mInvLoopSpeed)
            COPY_MEMBER(mBirthMomentum)
            COPY_MEMBER(mBirthMomentumAmount)
            mAttractors.clear();
            for (unsigned int i = 0; i != c->mAttractors.size(); i++) {
                mAttractors.push_back(Attractor(c->mAttractors[i], this));
            }
            if (!mPreserveParticles) {
                SetPool(c->mMaxParticles, c->mType);
            }
            SetRelativeMotion(
                c->mRelativeMotion,
                (Hmx::Object *)c->mMotionParent.Ptr() == c ? this : c->mMotionParent.Ptr()
            );
            SetSubSamples(c->mSubSamples);
        }
    END_COPYING_MEMBERS
END_COPYS

static const float sFloat = 4;
INIT_REVS(0x29, 0)

float unusedpartlol() { return sFloat; }

BEGIN_LOADS(RndParticleSys)
    LOAD_REVS(bs)
    ASSERT_REVS(0x29, 0)
    if (d.rev > 0x16) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    if (d.rev > 0x1B) {
        LOAD_SUPERCLASS(RndPollable)
    }
    if (d.rev > 0) {
        LOAD_SUPERCLASS(RndAnimatable)
        LOAD_SUPERCLASS(RndTransformable)
        LOAD_SUPERCLASS(RndDrawable)
    }
    d >> mLife;
    if (d.rev > 0x23) {
        d >> mHeightRatio;
    }
    d >> mPosLow;
    d >> mPosHigh;
    d >> mSpeed;
    d >> mPitch;
    d >> mYaw;
    d >> mEmitRate;
    if (d.rev > 0x20) {
        d >> mMaxBurst;
        d >> mBurstInterval >> mBurstPeak >> mBurstLength;
    }
    d >> mStartSize;
    if (d.rev > 0xF)
        d >> mDeltaSize;
    d >> mStartColorLow;
    d >> mStartColorHigh;
    d >> mEndColorLow;
    d >> mEndColorHigh;
    if (d.rev > 0x19)
        d >> mBounce;
    else if (d.rev > 1) {
        bool ba7;
        d >> ba7;
        Plane p;
        if (d.rev > 0xB) {
            d >> p;
        } else {
            Vector3 v1;
            d >> v1 >> p.a >> p.b >> p.c;
            p.d = -Dot(reinterpret_cast<Vector3 &>(p), v1);
        }
        if (ba7) {
            bool old = TheLoadMgr.EditMode();
            TheLoadMgr.SetEditMode(true);
            const char *bounceName = MakeString("%s_bounce.trans", FileGetBase(Name()));
            mBounce = Dir()->New<RndTransformable>(bounceName);
            TheLoadMgr.SetEditMode(old);
            Transform worldXfm;
            worldXfm.v = p.On();
            worldXfm.m.z = reinterpret_cast<Vector3 &>(p);
            Cross(Vector3(0, 1, 0), worldXfm.m.z, worldXfm.m.x);
            Cross(worldXfm.m.z, worldXfm.m.x, worldXfm.m.y);
            Normalize(worldXfm.m.x, worldXfm.m.x);
            Normalize(worldXfm.m.y, worldXfm.m.y);
            mBounce->SetWorldXfm(worldXfm);
        }
    } else {
        std::list<Plane> planes;
        d >> planes;
    }
    d >> mForce;
    d >> mMat;
    if (d.rev > 0x17 && d.rev < 0x19) {
        char buf[0x80];
        d.ReadString(buf, 0x80);
        if (!mMat && buf[0] != '\0') {
            mMat = LookupOrCreateMat(buf, Dir());
        }
    }
    if (d.rev > 0x11) {
        d >> (int &)mType >> mGrowRatio >> mShrinkRatio >> mMidColorRatio;
        d >> mMidColorLow >> mMidColorHigh;
    } else if (d.rev < 0xD) {
        int i94;
        d >> i94;
    }
    d >> mMaxParticles;

    if (d.rev > 2) {
        if (d.rev < 7) {
            int i98;
            d >> i98;
        } else if (d.rev < 0xD) {
            int i9c;
            d >> i9c;
        }
    }
    if (d.rev > 3) {
        d >> mBubblePeriod >> mBubbleSize >> mBubble;
    }
    if (d.rev > 0x1D) {
        d >> mRotate >> mRotSpeed >> mRotDrag;
        if (d.rev > 0x24) {
            d >> mRotRandomDir;
        }
        d >> mDrag;
    }
    if (d.rev > 0x1F) {
        d >> mSwingArmStart >> mSwingArmEnd;
        d >> mAlignWithVelocity >> mStretchWithVelocity >> mConstantArea >> mStretchScale;
    }
    if (d.rev > 0x21) {
        d >> mPerspectiveStretch;
    }

    if (d.rev > 4 && d.rev < 0xF) {
        bool baf;
        d >> baf;
        BaseMaterial::ZMode z = baf ? BaseMaterial::kTransparent : BaseMaterial::kDisable;
        if (mMat)
            mMat->SetZMode(z);
    }
    if (d.rev > 5 && d.rev < 0x11) {
        String str;
        d >> str;
    }
    if (d.rev == 8) {
        bool b1b0;
        d >> b1b0;
    }
    if (d.rev > 0xC && d.rev < 0xE) {
        int i1a0;
        d >> i1a0;
    }
    if (d.rev > 0x13) {
        d >> mRelativeMotion;
    } else if (d.rev > 0xC) {
        bool b;
        d >> b;
        mRelativeMotion = b;
    }
    if (d.rev > 0x1A) {
        d >> mMotionParent;
    }
    SetRelativeMotion(mRelativeMotion, mMotionParent);
    if (d.rev > 0x12) {
        d >> mMesh;
    }
    if (d.rev > 0x1E || d.rev == 0x15) {
        d >> mSubSamples;
    }
    SetSubSamples(mSubSamples);
    if (d.rev > 0x1B) {
        d >> mFrameDrive;
    } else {
        mFrameDrive = true;
    }
    if (d.rev > 0x22) {
        d >> mPauseOffscreen;
    } else {
        mPauseOffscreen = false;
    }
    if (d.rev > 0x1C) {
        d >> mFastForward;
    } else {
        mFastForward = false;
    }
    mNeedForward = mFastForward;
    if (d.rev > 0x26) {
        d >> mAnimatedUVs;
        float tileHoldTime;
        d >> tileHoldTime;
        d >> mTilesAcross;
        d >> mTilesDown;
        d >> mNumTiles;
        d >> mFirstUsedTile;
        d >> mLoopAnim;
        d >> mRandomStart;
        SetTileHoldTime(tileHoldTime);
    }
    if (d.rev > 0x27) {
        d >> mAttractors;
    }
    if (d.rev > 0x28) {
        d >> mBirthMomentum >> mBirthMomentumAmount;
    }
    if (d.rev > 0xA) {
        d >> mPreserveParticles;
        if (mPreserveParticles) {
            int count;
            d >> count;
            SetPool(mMaxParticles, mType);
            for (int i = 0; i < count; i++) {
                RndParticle *p = AllocParticle();
                if (p) {
                    p->angle = 0;
                    p->swingArm = 0;
                    p->vel.Set(0, 0, 0, 0);
                    d >> *p;
                } else {
                    MILO_NOTIFY_ONCE(
                        "Unable to allocate all particles for %s\n", PathName(this)
                    );
                    RndParticle pp;
                    d >> pp;
                }
            }
        } else {
            SetPool(mMaxParticles, mType);
        }
    } else {
        SetPool(mMaxParticles, mType);
    }
    mSleepTime = 0;
    mLastFrame = Frame();
END_LOADS

void RndParticleSys::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mFrameDrive) {
        UpdateParticles();
        mLastFrame = frame;
        mSleepTime = 0;
    }
}

float RndParticleSys::EndFrame() {
    if (mFrameDrive) {
        return Max(mLife.x, mLife.y);
    } else
        return 0;
}

void RndParticleSys::Poll() {
    if (!mFrameDrive) {
        mElapsedTime += (GetRate() == k30_fps_ui ? TheTaskMgr.DeltaUISeconds()
                                                 : TheTaskMgr.DeltaSeconds())
            * 30.0f;
        if (mPollsSinceDraw == 0) {
            if (Showing()
                && (mActiveParticles || mExplicitParts || mEmitRate.x > 0
                    || mEmitRate.y > 0 || mMaxBurst > 0)) {
                UpdateRelativeXfm();
                UpdateParticles();
            } else
                mLastFrame = CalcFrame();
        } else if (mActiveParticles && mPollsSinceDraw % 60 == 0 && !mPreserveParticles) {
            float calced = CalcFrame();
            RndParticle *p = mActiveParticles;
            while (p) {
                if (CheckParticleLife(calced, p)) {
                    p = FreeParticle(p);
                } else
                    p = p->next;
            }
        }
        if (mSubSamples > 0 && Dirty()) {
            MakeLocToRel(mSubSampleXfm);
        }
        mPollsSinceDraw++;
    }
}

void RndParticleSys::Enter() {
    mNeedForward = mFastForward;
    mElapsedTime = 0;
    RndPollable::Enter();
}

void RndParticleSys::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    Transform tf;
    FastInvert(WorldXfm(), tf);
    Multiply(s, tf, s);
    SetSphere(s);
}

bool RndParticleSys::MakeWorldSphere(Sphere &s, bool z) {
    if (z) {
        s.Zero();
        for (RndParticle *p = mActiveParticles; p != nullptr; p = p->next) {
            Sphere curSphere;
            Multiply(p->Pos(), mRelativeXfm, curSphere.center);
            curSphere.radius = p->size / 2;
            s.GrowToContain(curSphere);
        }
        return true;
    } else {
        if (GetSphere().radius) {
            Multiply(GetSphere(), WorldXfm(), s);
            return true;
        } else {
            return false;
        }
    }
}

void RndParticleSys::Mats(std::list<class RndMat *> &mats, bool) {
    if (mMat) {
        MatShaderOptions opts = GetDefaultMatShaderOpts(this, mMat);
        mMat->SetShaderOpts(opts);
        mats.push_back(mMat);
    }
}

void RndParticleSys::DrawShowing() {
    if (mFrameDrive) {
        UpdateRelativeXfm();
    } else {
        if (mPollsSinceDraw > 1) {
            UpdateRelativeXfm();
            UpdateParticles();
        } else if (mRelativeMotion == 1) {
            UpdateRelativeXfm();
        }
        mPollsSinceDraw = 0;
    }
}

void RndParticleSys::SetPool(int max, Type ty) {
    if (mPreserveParticles) {
        SetPersistentPool(max, ty);
    } else {
        for (RndParticle *p = mActiveParticles; p != nullptr; p = FreeParticle(p))
            ;
        mType = ty;
        mMaxParticles = max;
        int limit = SystemConfig()
            ? SystemConfig("rnd", "particlesys", "local_limit")->Int(1)
            : mMaxParticles;
        if (mMaxParticles > limit) {
            MILO_NOTIFY(
                "Max particles for %s is too high (%d > %d). The max number of particles has been reset to %d.\n",
                PathName(this),
                mMaxParticles,
                limit,
                limit
            );
            mMaxParticles = limit;
        }
        mActiveParticles = nullptr;
        mNumActive = 0;
        mEmitCount = 0;
    }
}

void RndParticleSys::SetPersistentPool(int max, Type ty) {
    delete[] mPersistentParticles;
    mMaxParticles = max;
    mType = ty;
    if (mMaxParticles != 0) {
        if (ty == kFancy) {
            mPersistentParticles = new RndFancyParticle[max];
            RndFancyParticle *fp = (RndFancyParticle *)mPersistentParticles;
            for (int i = 0; i != max; i++) {
                (fp++)->next = fp;
            }
            fp->next = nullptr;
        } else {
            mPersistentParticles = new RndParticle[max];
            RndParticle *p = mPersistentParticles;
            for (int i = 0; i != max; i++) {
                (p++)->next = p;
            }
            p->next = nullptr;
        }
    } else {
        mPersistentParticles = nullptr;
    }
    mActiveParticles = nullptr;
    mNumActive = 0;
    mFreeParticles = mPersistentParticles;
    mEmitCount = 0;
}

void RndParticleSys::SetTileHoldTime(float f1) {
    mTileAnimSpeed = f1;
    mLoopSpeed = mNumTiles * mTileAnimSpeed;
    MaxEq(mLoopSpeed, 0.0001f);
    mInvLoopSpeed = 1.0f / mLoopSpeed;
}

void RndParticleSys::SetNumTiles(int num) {
    mNumTiles = Max(num, 1);
    mLoopSpeed = mNumTiles * mTileAnimSpeed;
    mLoopSpeed = Max(mLoopSpeed, 0.0001f);
    mInvLoopSpeed = 1.0f / mLoopSpeed;
}

void RndParticleSys::SetGrowRatio(float f) {
    if (f >= 0 && f <= mGrowRatio)
        mGrowRatio = f;
}

void RndParticleSys::SetShrinkRatio(float f) {
    if (f >= mGrowRatio && f <= 1.0f)
        mShrinkRatio = f;
}

void RndParticleSys::SetFrameDrive(bool b) {
    mFrameDrive = b;
    if (mFrameDrive) {
        mLastFrame = Frame();
    } else
        mPollsSinceDraw = 0;
    mSleepTime = 0;
}

void RndParticleSys::SetPauseOffscreen(bool b) {
    mPauseOffscreen = b;
    mSleepTime = 0;
}

void RndParticleSys::SetAnimatedUV(bool b) {
    if (mAnimatedUVs != b) {
        SetPool(mMaxParticles, mType);
    }
    mAnimatedUVs = b;
}

void RndParticleSys::SetMesh(RndMesh *mesh) {
    if (mesh) {
        SetTransParent(mesh, false);
        SetTransConstraint(RndTransformable::kConstraintParentWorld, 0, false);
        if (!mesh->GetKeepMeshData()) {
            MILO_NOTIFY(
                "keep_mesh_data should be checked for %s.  It's the mesh emitter for %s.\n",
                PathName(mesh),
                PathName(this)
            );
        }
    } else if (mMesh) {
        SetTransParent(0, false);
        SetTransConstraint(RndTransformable::kConstraintNone, 0, false);
    }
    mMesh = mesh;
}

RndParticle *RndParticleSys::AllocParticle() {
    RndParticle *p;
    if (mPreserveParticles) {
        p = mFreeParticles;
        if (!mFreeParticles)
            return nullptr;
        mFreeParticles = p->next;
    } else {
        p = gParticlePool->AllocateParticle();
        if (!p) {
            int size = ParticlePoolSize();
            MILO_NOTIFY_ONCE(
                "Can't allocate more particles for %s.\nGlobal max particle limit reached (%d).\n",
                PathName(this),
                size
            )
            return nullptr;
        }
    }
    p->prev = p;
    if (mActiveParticles) {
        mActiveParticles->prev = p;
    }
    p->next = mActiveParticles;
    mActiveParticles = p;
    mNumActive++;
    return p;
}

RndParticle *RndParticleSys::FreeParticle(RndParticle *p) {
    if (!p)
        return nullptr;
    else {
        if (p == mActiveParticles) {
            mActiveParticles = p->next;
        } else {
            p->prev->next = p->next;
        }
        if (p->next) {
            p->next->prev = p->prev;
        }
        if (!p->prev) {
            MILO_FAIL("Already deallocated particle");
        }
        p->prev = nullptr;
        RndParticle *ret = nullptr;
        if (mPreserveParticles) {
            ret = p->next;
            p->next = mFreeParticles;
            mFreeParticles = p;
        } else {
            ret = gParticlePool->FreeParticle(p);
        }
        mNumActive--;
        return ret;
    }
}

void RndParticleSys::MakeLocToRel(Transform &tf) {
    if (mRelativeMotion == 1) {
        if (mMotionParent == this) {
            tf.Reset();
            return;
        }
    }
    Transpose(mRelativeXfm, tf);
    Multiply(WorldXfm(), tf, tf);
}

void RndParticleSys::SetSubSamples(int num) {
    mSubSamples = num;
    Transpose(mRelativeXfm, mSubSampleXfm);
    Multiply(WorldXfm(), mSubSampleXfm, mSubSampleXfm);
}

void RndParticleSys::FreeAllParticles() {
    for (RndParticle *p = mActiveParticles; p != nullptr; p = FreeParticle(p))
        ;
    mEmitCount = 0;
}

void RndParticleSys::ExplicitParticles(int i1, bool b2, PartOverride &partOverride) {
    if (b2) {
        float frame = CalcFrame();
        Transform tf;
        MakeLocToRel(tf);
        for (int i = 0; i < i1 && mNumActive < mMaxParticles; i++) {
            RndParticle *p = AllocParticle();
            if (!p)
                break;
            InitParticle(frame, p, &tf, partOverride);
        }
    } else {
        mExplicitParts += i1;
    }
}

void RndParticleSys::InitParticle(RndParticle *p, const Transform *t) {
    InitParticle(CalcFrame(), p, t, gNoPartOverride);
}

void RndParticleSys::SetRelativeMotion(float motion, RndTransformable *parent) {
    mMotionParent = parent ? parent : this;
    mRelativeMotion = motion;
    mLastWorldXfm = mMotionParent->WorldXfm();
    if (motion == 1) {
        mRelativeXfm = mMotionParent->WorldXfm();
    } else {
        mRelativeXfm.Reset();
    }
    mMomentum.Zero();
}

void RndParticleSys::UpdateRelativeXfm() {
    if (mRelativeMotion == 1) {
        mRelativeXfm = mMotionParent->WorldXfm();
    } else if (mRelativeMotion != 0) {
        const Transform &parentWorld = mMotionParent->WorldXfm();
        Invert(mLastWorldXfm.m, mLastWorldXfm.m);
        Multiply(mLastWorldXfm.m, parentWorld.m, mLastWorldXfm.m);
        Hmx::Quat q60(0, 0, 0, 1);
        Hmx::Quat q50(mLastWorldXfm.m);
        FastInterp(q60, q50, mRelativeMotion, q60);
        MakeRotMatrix(q60, mLastWorldXfm.m);
        auto &harness_ref_0 = mRelativeXfm.v;
        Subtract(harness_ref_0, mLastWorldXfm.v, mRelativeXfm.v);
        Multiply(mRelativeXfm, mLastWorldXfm.m, mRelativeXfm);
        Normalize(mRelativeXfm.m, mRelativeXfm.m);
        Interp(mLastWorldXfm.v, parentWorld.v, mRelativeMotion, mLastWorldXfm.v);
        Add(mRelativeXfm.v, mLastWorldXfm.v, mRelativeXfm.v);
    }
    Subtract(mMotionParent->WorldXfm().v, mLastWorldXfm.v, mMomentum);
    mLastWorldXfm = mMotionParent->WorldXfm();
}

float RndParticleSys::CheckBursts(float f1) {
    if (f1 > 1) {
        f1 = 1;
    }
    float f5 = 0;
    float f6 = 0;
    for (auto it = mBursts.begin(); it != mBursts.end();) {
        float emit = it->Emit(f1);
        if (f6 > emit) {
            it = mBursts.erase(it);
        } else {
            f5 += emit;
            ++it;
        }
    }
    if (mBursts.size() < mMaxBurst) {
        mTimeTillBurst = mTimeTillBurst - f1;
        if (mTimeTillBurst <= f6) {
            Burst burst;
            if (burst.Set(
                    RandomFloat(mBurstPeak.x, mBurstPeak.y),
                    RandomFloat(mBurstLength.x, mBurstLength.y)
                )) {
                mBursts.push_back(burst);
            }
            mTimeTillBurst = RandomFloat(mBurstInterval.x, mBurstInterval.y);
        }
    }
    return f5;
}

void RndParticleSys::CreateParticles(float f1, float f2, const Transform &tf) {
    if (f2 > 0 && mNumActive < mMaxParticles) {
        mEmitCount += RandomFloat(mEmitRate.x, mEmitRate.y) * f2;
        mEmitCount += CheckBursts(f2) + (float)mExplicitParts;
        mExplicitParts = 0;
        while (mEmitCount >= 1.0f && mNumActive < mMaxParticles) {
            RndParticle *p = AllocParticle();
            if (!p) {
                mEmitCount = 0;
                return;
            }
            InitParticle(f1, p, &tf, gNoPartOverride);
            mEmitCount -= 1;
        }
    }
}

void RndParticleSys::RunFastForward() {
    mNeedForward = false;
    float f1 = (mEmitRate.x + mEmitRate.y) * 0.5f;
    if (f1 < 0.0001f)
        return;
    else {
        float f6 = 1.0f / f1;
        float f3 = Min<float>(f6 * (float)mMaxParticles, (mLife.x + mLife.y) * 0.5f);
        f6 = Max(1.0f, f6);
        float f4 = CalcFrame();
        Transform tf78;
        MakeLocToRel(tf78);
        for (float f5 = f4 - f3; f5 <= f4; f5 += f6) {
            MoveParticles(f5, f6);
            CreateParticles(f5, f6, tf78);
        }
    }
}

void RndParticleSys::InitParticle(
    float f1, RndParticle *particle, const Transform *xfm, PartOverride &partOverride
) {
    particle->birthFrame = f1;
    if (partOverride.mask & 1) {
        particle->deathFrame = particle->birthFrame + partOverride.life;
    } else {
        particle->deathFrame = particle->birthFrame + RandomFloat(mLife.x, mLife.y);
    }
    particle->pos.w = particle->deathFrame > particle->birthFrame
        ? 1.0f / (particle->deathFrame - particle->birthFrame)
        : 0;
    RndMesh *mesh = mMesh;
    if (partOverride.mask & 0x100) {
        mesh = partOverride.mesh;
    }
    if (mesh && !mesh->Faces().empty()) {
        RandomPointOnMesh(mesh, particle->Pos(), particle->Vel());
    } else {
        if (partOverride.mask & 0x200) {
            particle->pos.x =
                RandomFloat(partOverride.box.mMin.x, partOverride.box.mMax.x);
            particle->pos.y =
                RandomFloat(partOverride.box.mMin.y, partOverride.box.mMax.y);
            particle->pos.z =
                RandomFloat(partOverride.box.mMin.z, partOverride.box.mMax.z);
        } else {
            particle->pos.x = RandomFloat(mPosLow.x, mPosHigh.x);
            particle->pos.y = RandomFloat(mPosLow.y, mPosHigh.y);
            particle->pos.z = RandomFloat(mPosLow.z, mPosHigh.z);
        }
        float f8, f9;
        if (partOverride.mask & 0x80) {
            f8 = RandomFloat(partOverride.pitch.x, partOverride.pitch.y);
            f9 = RandomFloat(partOverride.yaw.x, partOverride.yaw.y);
        } else {
            f8 = RandomFloat(mPitch.x, mPitch.y);
            f9 = RandomFloat(mYaw.x, mYaw.y);
        }

        float cosPitch = FastCos(f8);
        float sinPitch = FastSin(f9);
        particle->vel.x = -cosPitch * sinPitch;
        particle->vel.y = cosPitch * FastCos(f9);
        particle->vel.z = FastSin(f8);
    }
    particle->Vel() *=
        partOverride.mask & 2 ? partOverride.speed : RandomFloat(mSpeed.x, mSpeed.y);
    float f11 = particle->deathFrame != particle->birthFrame
        ? 1.0f / (particle->deathFrame - particle->birthFrame)
        : 0;
    if (mRotate) {
        particle->angle = RandomFloat(0, PI * 2);
        particle->swingArm = RandomFloat(mSwingArmStart.x, mSwingArmStart.y);
    } else {
        particle->angle = 0;
        particle->swingArm = 0;
    }
    if (partOverride.mask & 0x10) {
        particle->col = partOverride.startColor;
    } else {
        float lowH = 0, lowS = 0, lowL = 0;
        MakeHSL(mStartColorLow, lowH, lowS, lowL);
        float highH = 0, highS = 0, highL = 0;
        MakeHSL(mStartColorHigh, highH, highS, highL);
        MakeColor(
            RandomFloat(lowH, highH),
            RandomFloat(lowS, highS),
            RandomFloat(lowL, highL),
            particle->col
        );
        particle->col.alpha = RandomFloat(mStartColorLow.alpha, mStartColorHigh.alpha);
    }
    if (partOverride.mask & 4) {
        particle->size = partOverride.size;
    } else {
        particle->size = RandomFloat(mStartSize.x, mStartSize.y);
    }
    if (partOverride.mask & 8) {
        particle->sizeVel = partOverride.deltaSize;
    } else {
        particle->sizeVel = RandomFloat(mDeltaSize.x, mDeltaSize.y);
    }
    if (particle->sizeVel < -particle->size) {
        particle->sizeVel = -particle->size;
    }
    if (partOverride.mask & 0x40) {
        particle->colVel = partOverride.endColor;
    } else {
        float lowH = 0, lowS = 0, lowL = 0;
        MakeHSL(mEndColorLow, lowH, lowS, lowL);
        float highH = 0, highS = 0, highL = 0;
        MakeHSL(mEndColorHigh, highH, highS, highL);
        MakeColor(
            RandomFloat(lowH, highH),
            RandomFloat(lowS, highS),
            RandomFloat(lowL, highL),
            particle->colVel
        );
        particle->colVel.alpha = RandomFloat(mEndColorLow.alpha, mEndColorHigh.alpha);
    }
    if (mType == kFancy) {
        RndFancyParticle *fancyParticle = (RndFancyParticle *)particle;
        fancyParticle->momentum = mMomentum;
        if (mBubble) {
            fancyParticle->bubbleFreq =
                (2 * PI) / RandomFloat(mBubblePeriod.x, mBubblePeriod.y);
            fancyParticle->bubblePhase = RandomFloat(0, 2 * PI);
            float f14 = RandomFloat(0, 2 * PI);
            float f20 = FastCos(f14);
            f14 = FastSin(f14);
            Scale(
                Vector3(f14, 0, f20),
                RandomFloat(mBubbleSize.x, mBubbleSize.y),
                fancyParticle->BubbleDir()
            );
            Vector3 toAdd;
            Scale(fancyParticle->BubbleDir(), FastSin(fancyParticle->bubblePhase), toAdd);
            Add(fancyParticle->Pos(), toAdd, fancyParticle->Pos());
            fancyParticle->bubblePhase =
                -(f1 * fancyParticle->bubbleFreq - fancyParticle->bubblePhase);
        }
        if (mRotate) {
            fancyParticle->RPF = RandomFloat(mRotSpeed.x, mRotSpeed.y) * 0.0034906587f;
            if (mRotRandomDir && (RandomInt() & 0x100000)) {
                fancyParticle->RPF = -fancyParticle->RPF;
            }
            fancyParticle->swingArmVel =
                (RandomFloat(mSwingArmEnd.x, mSwingArmEnd.y) - fancyParticle->swingArm)
                * f11;
        } else {
            fancyParticle->RPF = 0;
            fancyParticle->swingArmVel = 0;
        }
        if (mGrowRatio != 0) {
            fancyParticle->growFrame =
                Interp(fancyParticle->birthFrame, fancyParticle->deathFrame, mGrowRatio);
            fancyParticle->growVel = fancyParticle->growFrame != fancyParticle->birthFrame
                ? fancyParticle->size
                    / (fancyParticle->growFrame - fancyParticle->birthFrame)
                : 0;
        } else {
            fancyParticle->growVel = 0;
            fancyParticle->growFrame = fancyParticle->birthFrame;
        }
        float death = fancyParticle->deathFrame;
        if (mShrinkRatio != 1) {
            fancyParticle->shrinkFrame =
                Interp(fancyParticle->birthFrame, death, mShrinkRatio);
            fancyParticle->shrinkVel = fancyParticle->shrinkFrame != death
                ? (fancyParticle->size + fancyParticle->sizeVel)
                    / (fancyParticle->shrinkFrame - death)
                : 0;
        } else {
            fancyParticle->shrinkVel = 0;
            fancyParticle->shrinkFrame = fancyParticle->birthFrame;
        }
        fancyParticle->beginGrow = fancyParticle->growFrame > fancyParticle->birthFrame
            ? 1.0f / (fancyParticle->growFrame - fancyParticle->birthFrame)
            : 0;
        fancyParticle->midGrow = fancyParticle->shrinkFrame > fancyParticle->growFrame
            ? 1.0f / (fancyParticle->shrinkFrame - fancyParticle->growFrame)
            : 0;
        fancyParticle->endGrow = fancyParticle->deathFrame > fancyParticle->shrinkFrame
            ? 1.0f / (fancyParticle->deathFrame - fancyParticle->shrinkFrame)
            : 0;

        if (mGrowRatio != 0) {
            fancyParticle->size = 0;
        }
        if (fancyParticle->shrinkFrame != fancyParticle->growFrame) {
            f11 = 1.0f / (fancyParticle->shrinkFrame - fancyParticle->growFrame);
        }
        fancyParticle->midcolFrame =
            Interp(fancyParticle->birthFrame, fancyParticle->deathFrame, mMidColorRatio);
        if (partOverride.mask & 0x20) {
            fancyParticle->midcolVel = partOverride.midColor;
        } else {
            fancyParticle->midcolVel.red =
                RandomFloat(mMidColorLow.red, mMidColorHigh.red);
            fancyParticle->midcolVel.green =
                RandomFloat(mMidColorLow.green, mMidColorHigh.green);
            fancyParticle->midcolVel.blue =
                RandomFloat(mMidColorLow.blue, mMidColorHigh.blue);
            fancyParticle->midcolVel.alpha =
                RandomFloat(mMidColorLow.alpha, mMidColorHigh.alpha);
        }
        if (fancyParticle->midcolFrame > fancyParticle->birthFrame) {
            fancyParticle->vel.w =
                1.0f / (fancyParticle->midcolFrame - fancyParticle->birthFrame);
        } else {
            fancyParticle->vel.w = 0;
        }
        fancyParticle->bubbleDir.w =
            fancyParticle->deathFrame > fancyParticle->midcolFrame
            ? 1.0f / (fancyParticle->deathFrame - fancyParticle->midcolFrame)
            : 0;

        Subtract(fancyParticle->colVel, fancyParticle->midcolVel, fancyParticle->colVel);
        if (fancyParticle->deathFrame != fancyParticle->midcolFrame) {
            float scalar =
                1.0f / (fancyParticle->deathFrame - fancyParticle->midcolFrame);
            Multiply(fancyParticle->colVel, scalar, fancyParticle->colVel);
        }
        if (fancyParticle->midcolFrame != fancyParticle->birthFrame) {
            Subtract(
                fancyParticle->midcolVel, fancyParticle->col, fancyParticle->midcolVel
            );
            if (fancyParticle->midcolFrame != fancyParticle->birthFrame) {
                float scalar =
                    1.0f / (fancyParticle->midcolFrame - fancyParticle->birthFrame);
                Multiply(fancyParticle->midcolVel, scalar, fancyParticle->midcolVel);
            }
        }
    } else {
        Subtract(particle->colVel, particle->col, particle->colVel);
        Multiply(particle->colVel, f11, particle->colVel);
    }
    particle->sizeVel *= f11;
    Transform loc;
    if (!xfm) {
        MakeLocToRel(loc);
        xfm = &loc;
    }
    Multiply(particle->Pos(), *xfm, particle->Pos());
    Multiply(particle->Vel(), xfm->m, particle->Vel());
    if (mBubble && mType == kFancy) {
        RndFancyParticle *fancyParticle = (RndFancyParticle *)particle;
        Multiply(fancyParticle->BubbleDir(), xfm->m, fancyParticle->BubbleDir());
    }
    if (mRandomStart) {
        particle->tileIndex = RandomInt(0, mNumTiles);
    } else {
        particle->tileIndex = mFirstUsedTile;
    }
    particle->animTime = 0;
}

void RndParticleSys::MoveParticles(float f1, float f2) {
    START_AUTO_TIMER("psysmove");
    if (mActiveParticles && f2) {
        float f12, f13;
        if (mDrag > 0) {
            f12 = 1;
            f13 = pow(1.0f - mDrag, f2 * 0.03333333507180214f);
        } else {
            f12 = 1;
            f13 = 1;
        }
        if (mRotate && mRotDrag > 0) {
            f12 = pow(f12 - mRotDrag, f2 * 0.03333333507180214f);
        }
        bool fancy = mType == kFancy;
        Vector3 vScaled;
        Scale(mForce, f2, vScaled);
        Multiply(mRelativeXfm.m, vScaled, vScaled);
        Plane plane;
        if (mBounce) {
            plane.Set(WorldXfm().v, WorldXfm().m.z);
        }
        int i20 = mNumTiles + mFirstUsedTile;
        for (RndParticle *p = mActiveParticles; p != nullptr;) {
            if (CheckParticleLife(f1, p)) {
                p = FreeParticle(p);
            } else {
                if (mAnimatedUVs) {
                    p->animTime += f2;
                    if (p->tileIndex < i20 && p->animTime > mTileAnimSpeed) {
                        p->tileIndex++;
                        if (p->tileIndex >= i20) {
                            p->tileIndex = mLoopAnim ? mFirstUsedTile : i20 - 1;
                        }
                        p->animTime = fmod(p->animTime, mTileAnimSpeed);
                    }
                }
                if (fancy && mBirthMomentum) {
                    RndFancyParticle *fp = (RndFancyParticle *)p;
                    float f46 = mBirthMomentumAmount * f2 * 0.03333333507180214f;
                    ScaleAddEq(fp->Pos(), fp->momentum, f46);
                }
                ScaleAddEq(p->Pos(), p->Vel(), f2);
                // some more math
                for (int i = 0; i < mAttractors.size(); i++) {
                    Attractor &cur = mAttractors[i];
                    if (cur.mAttractor) {
                    }
                }
                p = p->next;
            }
        }
    }
}

DataNode RndParticleSys::OnSetStartColor(const DataArray *da) {
    DataArray *arr1 = da->Array(2);
    DataArray *arr2 = da->Array(3);
    SetStartColor(
        Hmx::Color(arr1->Float(0), arr1->Float(1), arr1->Float(2), arr1->Float(3)),
        Hmx::Color(arr2->Float(0), arr2->Float(1), arr2->Float(2), arr2->Float(3))
    );
    return 0;
}

DataNode RndParticleSys::OnSetStartColorInt(const DataArray *da) {
    Hmx::Color col1(da->Int(2));
    Hmx::Color col2(da->Int(3));
    col1.alpha = da->Float(4);
    col2.alpha = da->Float(5);
    SetStartColor(col1, col2);
    return 0;
}

DataNode RndParticleSys::OnSetEndColor(const DataArray *da) {
    DataArray *arr1 = da->Array(2);
    DataArray *arr2 = da->Array(3);
    SetEndColor(
        Hmx::Color(arr1->Float(0), arr1->Float(1), arr1->Float(2), arr1->Float(3)),
        Hmx::Color(arr2->Float(0), arr2->Float(1), arr2->Float(2), arr2->Float(3))
    );
    return 0;
}

DataNode RndParticleSys::OnSetEndColorInt(const DataArray *da) {
    Hmx::Color col1(da->Int(2));
    Hmx::Color col2(da->Int(3));
    col1.alpha = da->Float(4);
    col2.alpha = da->Float(5);
    SetEndColor(col1, col2);
    return 0;
}

DataNode RndParticleSys::OnSetEmitRate(const DataArray *da) {
    SetEmitRate(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnAddEmitRate(const DataArray *da) {
    float add = da->Float(2);
    mEmitRate.x = Max(0.0f, mEmitRate.x + add);
    mEmitRate.y = Max(0.0f, mEmitRate.y + add);
    return mEmitRate == Vector2(0, 0);
}

DataNode RndParticleSys::OnSetBurstInterval(const DataArray *da) {
    SetMaxBurst(da->Int(2));
    SetBurstInterval(da->Float(3), da->Float(4));
    return 0;
}

DataNode RndParticleSys::OnSetBurstPeak(const DataArray *da) {
    SetBurstPeak(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnSetBurstLength(const DataArray *da) {
    SetBurstLength(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnSetLife(const DataArray *da) {
    SetLife(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnSetSpeed(const DataArray *da) {
    SetSpeed(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnSetRotate(const DataArray *da) {
    SetRotate(da->Int(2));
    SetAngularSpeed(da->Float(3), da->Float(4));
    SetAngularDrag(da->Float(4));
    return 0;
}

DataNode RndParticleSys::OnSetSwingArm(const DataArray *da) {
    SetSwingArmStart(da->Float(2), da->Float(3));
    SetSwingArmEnd(da->Float(4), da->Float(5));
    return 0;
}

DataNode RndParticleSys::OnSetDrag(const DataArray *da) {
    SetDrag(da->Float(2));
    return 0;
}

DataNode RndParticleSys::OnSetAlignment(const DataArray *da) {
    SetAlignWithVelocity(da->Int(2));
    SetStretchWithVelocity(da->Int(3));
    SetConstantArea(da->Int(4));
    SetStretchScale(da->Float(5));
    return 0;
}

DataNode RndParticleSys::OnSetStartSize(const DataArray *da) {
    SetStartSize(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndParticleSys::OnSetMat(const DataArray *da) {
    SetMat(da->Obj<RndMat>(2));
    return 0;
}

DataNode RndParticleSys::OnSetPos(const DataArray *da) {
    SetPos(
        Vector3(da->Float(2), da->Float(3), da->Float(4)),
        Vector3(da->Float(5), da->Float(6), da->Float(7))
    );
    return 0;
}

DataNode RndParticleSys::OnActiveParticles(const DataArray *da) {
    return mActiveParticles != nullptr;
}

DataNode RndParticleSys::OnExplicitPart(const DataArray *da) {
    ExplicitParticles(1, false, gNoPartOverride);
    return 0;
}

DataNode RndParticleSys::OnExplicitParts(const DataArray *da) {
    bool b = da->Size() >= 4 && da->Int(3);
    ExplicitParticles(da->Int(2), b, gNoPartOverride);
    return 0;
}

#pragma endregion
