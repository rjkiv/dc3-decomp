#include "math/DoubleExponentialSmoother.h"

DoubleExponentialSmoother::DoubleExponentialSmoother()
    : mCurrent(0), mA(0), mB(0), mSmooth(0), mTrend(0) {}

DoubleExponentialSmoother::DoubleExponentialSmoother(
    float start, float smooth, float trend
)
    : mCurrent(start), mA(start), mB(0), mSmooth(smooth), mTrend(trend) {}

void DoubleExponentialSmoother::Smooth(float target, float dt) {
    float newSmooth = Clamp(0.0f, 1.0f, mSmooth * dt);
    float newTrend = Clamp(0.0f, 1.0f, mTrend * dt);
    float oldA = mA;
    mA = Interp(mCurrent, target, newSmooth);
    mB = Interp(mB, mA - oldA, newTrend);
    mCurrent = mB + mA;
}

void Vector2DESmoother::SetSmoothParameters(float smooth, float trend) {
    mX.SetSmoothParameters(smooth, trend);
    mY.SetSmoothParameters(smooth, trend);
}

void Vector2DESmoother::ForceValue(Vector2 value) {
    mX.ForceValue(value.x);
    mY.ForceValue(value.y);
}

Vector2 Vector2DESmoother::Value() const { return Vector2(mX.Value(), mY.Value()); }

Vector3DESmoother::Vector3DESmoother(Vector3 v, float alpha, float beta)
    : mX(v.x, alpha, beta), mY(v.y, alpha, beta), mZ(v.z, alpha, beta) {}

void Vector3DESmoother::SetSmoothParameters(float smooth, float trend) {
    mX.SetSmoothParameters(smooth, trend);
    mY.SetSmoothParameters(smooth, trend);
    mZ.SetSmoothParameters(smooth, trend);
}

Vector3 Vector3DESmoother::Value() const {
    return Vector3(mX.Value(), mY.Value(), mZ.Value());
}

void Vector3DESmoother::ForceValue(Vector3 v) {
    mX.ForceValue(v.x);
    mY.ForceValue(v.y);
    mZ.ForceValue(v.z);
}
