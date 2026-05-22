#pragma once
#include "obj/Data.h"
#include "utl/MemMgr.h"
#include "math/Vec.h"

class Interpolator {
public:
    virtual float Eval(float) const = 0;
    virtual float ClampEval(float f) const { return Eval(f); }
    virtual void Reset(const DataArray *) = 0;
    virtual ~Interpolator(); // generic dtor

    MEM_OVERLOAD(Interpolator, 0x28);

protected:
};

class ATanInterpolator : public Interpolator {
public:
    ATanInterpolator(const char *, const char *);
    virtual float Eval(float) const;
    virtual void Reset(const DataArray *);

    void Reset(const Vector2 &, const Vector2 &, float);

protected:
    void Sync();

    Vector2 mP0; // 0x4
    Vector2 mP1; // 0xc
    float mSeverity; // 0x14
    float mSlope; // 0x18
    float mB; // 0x1c
    float mScale; // 0x20
    float mOffset; // 0x24
};
