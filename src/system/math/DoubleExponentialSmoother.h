#pragma once
#include "math/Vec.h"

// https://en.wikipedia.org/wiki/Exponential_smoothing#Double_exponential_smoothing_(Holt_linear)
class DoubleExponentialSmoother {
    friend class Vector2DESmoother;
    friend class Vector3DESmoother;

public:
    DoubleExponentialSmoother();
    DoubleExponentialSmoother(float, float, float);
    void Smooth(float, float);
    void Reset() {
        mPrevLevel = 0;
        mLevel = 0;
        mTrend = 0;
    }
    void SetCoeffs(float alpha, float beta) {
        mAlpha = alpha;
        mBeta = beta;
    }
    void SetParams(float prevLevel, float level, float trend) {
        mPrevLevel = prevLevel;
        mLevel = level;
        mTrend = trend;
    }
    float Level() const { return mLevel; }

protected:
    float mLevel; // 0x0
    float mPrevLevel; // 0x4
    float mTrend; // 0x8
    float mAlpha; // 0xc
    float mBeta; // 0x10
};

class Vector2DESmoother {
public:
    void SetSmoothParameters(float, float);
    void Smooth(Vector2, float, bool);
    Vector2 Value() const;
    void ForceValue(Vector2);

protected:
    DoubleExponentialSmoother mSmootherX; // 0x0
    DoubleExponentialSmoother mSmootherY; // 0x14
};

class Vector3DESmoother {
public:
    Vector3DESmoother() {}
    Vector3DESmoother(Vector3, float, float);
    void SetSmoothParameters(float, float);
    void Smooth(Vector3, float, bool);
    Vector3 Value() const;
    void ForceValue(Vector3);

protected:
    DoubleExponentialSmoother mSmootherX;
    DoubleExponentialSmoother mSmootherY;
    DoubleExponentialSmoother mSmootherZ;
};
