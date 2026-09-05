#include "rndobj/Wind.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

float gUnitsPerMeter = 39.370079f;
static Rand *sRand = nullptr;
static float sWhiteField[0x400] = { 0 };
static float sWindField[0x401] = { 0 };

void SetWind(int ai, int bi, float av, float bv, float scale) {
    sWindField[ai] = av;
    while (bi - ai >= 2) {
        int mi = (ai + bi) / 2;
        float mv = (av + bv) / 2;

        mv += sRand->Gaussian() * scale;
        scale /= sqrtf(2.0);

        SetWind(ai, mi, av, mv, scale);
        sWindField[mi] = mv;
        ai = mi;
        av = mv;
    }
}

RndWind::RndWind()
    : mPrevailing(0, 0, 0), mRandom(0, 0, 0), mTimeLoop(100),
      mSpaceLoop(gUnitsPerMeter * 10), mTrans(this), mAboutZ(false),
      mMaxSpeed(kHugeFloat), mMinSpeed(0), mWindOwner(this, this) {
    SyncLoops();
}

RndWind::~RndWind() {}

bool RndWind::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mWindOwner == from) {
        if (mWindOwner == this) {
            mWindOwner = this;
        } else {
            RndWind *windTo = dynamic_cast<RndWind *>(to);
            if (windTo) {
                mWindOwner = windTo->mWindOwner.Ptr();
            } else {
                mWindOwner = this;
            }
        }

        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

BEGIN_HANDLERS(RndWind)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_ACTION(set_defaults, SetDefaults())
    HANDLE_ACTION(set_zero, Zero())
END_HANDLERS

BEGIN_PROPSYNCS(RndWind)
    SYNC_PROP(prevailing, mPrevailing)
    SYNC_PROP(random, mRandom)
    SYNC_PROP(max_speed, mMaxSpeed)
    SYNC_PROP(min_speed, mMinSpeed)
    SYNC_PROP_SET(wind_owner, mWindOwner.Ptr(), SetWindOwner(_val.Obj<RndWind>()))
    SYNC_PROP_MODIFY(time_loop, mTimeLoop, SyncLoops())
    SYNC_PROP_MODIFY(space_loop, mSpaceLoop, SyncLoops())
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(about_z, mAboutZ)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndWind)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPrevailing;
    bs << mRandom;
    bs << mTimeLoop;
    bs << mSpaceLoop;
    bs << mWindOwner;
    bs << mTrans;
    bs << mAboutZ;
    bs << mMinSpeed;
    bs << mMaxSpeed;
END_SAVES

BEGIN_COPYS(RndWind)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndWind)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyShallow) {
            mWindOwner = c->mWindOwner.Ptr();
        } else {
            mWindOwner = this;
            mWindOwner = c->mWindOwner.Ptr();
            COPY_MEMBER(mPrevailing)
            COPY_MEMBER(mRandom)
            COPY_MEMBER(mTimeLoop)
            COPY_MEMBER(mSpaceLoop)
            COPY_MEMBER(mTrans)
            COPY_MEMBER(mAboutZ)
            COPY_MEMBER(mMinSpeed)
            COPY_MEMBER(mMaxSpeed)
            SyncLoops();
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(4, 0)

BEGIN_LOADS(RndWind)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(RndHighlightable)
    d >> mPrevailing;
    d >> mRandom;
    d >> mTimeLoop;
    d >> mSpaceLoop;
    if (d.rev > 1) {
        d >> mWindOwner;
        SetWindOwner(mWindOwner);
    }
    if (d.rev > 2) {
        d >> mTrans;
        d >> mAboutZ;
    }
    if (d.rev > 3) {
        d >> mMinSpeed;
        d >> mMaxSpeed;
    }
    SyncLoops();
END_LOADS

void RndWind::SyncLoops() {
    float f1 = (mTimeLoop == 0.0f) ? 0.0f : 1.0f / mTimeLoop;
    mTimeRate.Set(f1, f1 * 0.773437f, f1 * 1.38484f);
    f1 = (mSpaceLoop == 0.0f) ? 0.0f : 1.0f / mSpaceLoop;
    mSpaceRate.Set(f1, f1 * 0.773437f, f1 * 1.38484f);
}

void RndWind::Zero() {
    mRandom.Set(0.0f, 0.0f, 0.0f);
    mPrevailing.Set(0.0f, 0.0f, 0.0f);
}

void RndWind::SetDefaults() {
    mPrevailing.Set(0.0f, 0.0f, 0.0f);
    mRandom.Set(17.0f, 17.0f, 0.0f);
    mTimeLoop = 100.0f;
    mSpaceLoop = gUnitsPerMeter * 10;
}

void RndWind::SetWindOwner(RndWind *wind) { mWindOwner = wind ? wind : this; }

void RndWind::Init() {
    REGISTER_OBJ_FACTORY(RndWind);
    sRand = new Rand(0x7FEF8A);
    SetWind(0, 0x400, 0, 0, 0.5f);
    sWindField[0x400] = sWindField[0];
    int i = 0;
    do {
        sWhiteField[i] = RandomFloat(0, 1);
        i++;
    } while (i < 0x400);
    RELEASE(sRand);
}
