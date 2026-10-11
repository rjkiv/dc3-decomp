#pragma once
#include "math/Vec.h"

// https://en.wikipedia.org/wiki/Exponential_smoothing#Double_exponential_smoothing_(Holt_linear)
class DoubleExponentialSmoother {
public:
    DoubleExponentialSmoother();
    DoubleExponentialSmoother(float start, float smooth, float trend);
    void Smooth(float, float);

    void SetSmoothParameters(float smooth, float trend) {
        mSmooth = smooth;
        mTrend = trend;
    }
    void ForceValue(float value) {
        mA = value;
        mCurrent = value;
        mB = 0;
    }

    float Value() const { return mCurrent; }
    float GetSmoothParam() const { return mSmooth; }
    float GetTrendParam() const { return mTrend; }

private:
    float mCurrent; // 0x0
    float mA; // 0x4
    float mB; // 0x8
    float mSmooth; // 0xc
    float mTrend; // 0x10
};

class Vector2DESmoother {
public:
    Vector2DESmoother() {}
    Vector2DESmoother(Vector2, float, float);
    void SetSmoothParameters(float smooth, float trend);
    void Smooth(Vector2 target, float dt, bool normalize);
    Vector2 Value() const;
    void ForceValue(Vector2 value);

private:
    DoubleExponentialSmoother mX; // 0x0
    DoubleExponentialSmoother mY; // 0x14
};

class Vector3DESmoother {
public:
    Vector3DESmoother() {}
    Vector3DESmoother(Vector3, float, float);
    void SetSmoothParameters(float, float);
    void Smooth(Vector3, float, bool);
    Vector3 Value() const;
    void ForceValue(Vector3);

private:
    DoubleExponentialSmoother mX; // 0x0
    DoubleExponentialSmoother mY; // 0x14
    DoubleExponentialSmoother mZ; // 0x28
};
