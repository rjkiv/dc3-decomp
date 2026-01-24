#include "math/Interp.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include <cmath>

void ATanInterpolator::Sync() {
    float run = mX.x - mY.x;
    float absRun = std::fabs(run);
    float slope;
    if (absRun < 0.000001f) {
        slope = 0;
    } else {
        slope = mSeverity / run * 2.0f;
    }
    float b_val = -(mY.x * slope);
    mSlope = slope;
    mB = b_val - mSeverity;
    if (!(mSeverity > 0.001f)) {
        MILO_FAIL("ATanInterpolator: severity (%f) too small.", mSeverity);
    }
    float tanned = atan(-mSeverity);
    float rise = mX.y - mY.y;
    float neg_tanned = -tanned;
    mOffset = rise * 0.5f + mY.y;
    mScale = rise / (neg_tanned - tanned);
}

float ATanInterpolator::Eval(float f1) const {
    float tanned = atan(mSlope * f1 + mB);
    return mScale * tanned + mOffset;
}

ATanInterpolator::ATanInterpolator(const char *, const char *) : Interpolator() {
    mSeverity = 2.0;
    Sync();
}

void ATanInterpolator::Reset(const Vector2 &y, const Vector2 &x, float sev) {
    mY = y;
    mX = x;
    mSeverity = sev;
    Sync();
}

void ATanInterpolator::Reset(const DataArray *a) {
    float sev = a->Size() > 5 ? a->Float(5) : 10.0f;
    float f2 = a->Float(2);
    float f4 = a->Float(4);
    Vector2 vecX(f4, f2);
    float f1 = a->Float(1);
    float f3 = a->Float(3);
    Vector2 vecY(f3, f1);
    mSeverity = sev;
    mY = vecY;
    mX = vecX;
    Sync();
}
