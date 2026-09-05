#include "rndobj/ColorXfm.h"
#include "math/Trig.h"
#include "utl/BinStream.h"

int ModChan(int chan) {
    int i = chan % 3;
    if (i < 0) {
        i += 3;
    }
    return i;
}

void RndColorXfm::Reset() { mColorXfm.Reset(); }

void RndColorXfm::AdjustBrightness() {
    Transform tf;
    tf.Reset();
    float set = (mBrightness + 100.0f) / 200.0f + -0.5f;
    tf.v.Set(set, set, set);
    Multiply(mColorXfm, tf, mColorXfm);
}

void RndColorXfm::Save(BinStream &bs) const {
    bs << 0;
    bs << mColorXfm;
    bs << mHue << mSaturation << mLightness;
    bs << mContrast << mBrightness;
    bs << mLevelInLo << mLevelInHi;
    bs << mLevelOutLo << mLevelOutHi;
}

bool RndColorXfm::Load(BinStream &bs) {
    int rev;
    bs >> rev;
    if (rev > 0)
        return false;
    else {
        bs >> mColorXfm;
        bs >> mHue >> mSaturation >> mLightness >> mContrast >> mBrightness;
        bs >> mLevelInLo >> mLevelInHi;
        bs >> mLevelOutLo >> mLevelOutHi;
        return true;
    }
}

RndColorXfm::RndColorXfm()
    : mHue(0), mSaturation(0), mLightness(0), mContrast(0), mBrightness(0),
      mLevelInLo(0, 0, 0), mLevelInHi(1, 1, 1), mLevelOutLo(0, 0, 0),
      mLevelOutHi(1, 1, 1) {
    Reset();
}

void RndColorXfm::AdjustColorXfm() {
    mColorXfm.Reset();
    AdjustHue();
    AdjustSaturation();
    AdjustLightness();
    AdjustContrast();
    AdjustBrightness();
    AdjustLevels();
}

void RndColorXfm::AdjustHue() {
    Transform xfm;
    xfm.Reset();
    if (mHue >= 120.0f) {
        float hue = ((mHue - 120.0f) / 120.0f) * PI / 2;
        float cosHue = std::cos(hue);
        float sinHue = std::sin(hue);
        for (int i = 0; i < 3; i++) {
            xfm.m[i][i] = 0;
            xfm.m[ModChan(i + 1)][i] = cosHue;
            xfm.m[ModChan(i + 2)][i] = sinHue;
        }
    } else if (mHue > 0) {
        float hue = (mHue / 120.0f) * PI / 2;
        float cosHue = std::cos(hue);
        float sinHue = std::sin(hue);
        for (int i = 0; i < 3; i++) {
            xfm.m[i][i] = cosHue;
            xfm.m[ModChan(i + 1)][i] = sinHue;
            xfm.m[ModChan(i + 2)][i] = 0;
        }
    } else if (mHue <= -120.0f) {
        float hue = ((-mHue - 120.0f) / 120.0f) * PI / 2;
        float cosHue = std::cos(hue);
        float sinHue = std::sin(hue);
        for (int i = 0; i < 3; i++) {
            xfm.m[i][i] = 0;
            xfm.m[ModChan(i + 1)][i] = sinHue;
            xfm.m[ModChan(i + 2)][i] = cosHue;
        }
    } else if (mHue < 0) {
        float hue = (-mHue / 120.0f) * PI / 2;
        float cosHue = std::cos(hue);
        float sinHue = std::sin(hue);
        for (int i = 0; i < 3; i++) {
            xfm.m[i][i] = cosHue;
            xfm.m[ModChan(i + 1)][i] = 0;
            xfm.m[ModChan(i + 2)][i] = sinHue;
        }
    }
    Multiply(mColorXfm, xfm, mColorXfm);
}

void RndColorXfm::AdjustSaturation() {
    Transform xfm;
    xfm.Reset();
    float sat = mSaturation / 100.0f;
    if (sat > 0) {
        sat += 1.0f;
    } else {
        sat = -(sat * -0.6666666f - 1);
    }
    float f2 = (1.0f - sat) / 2;
    for (int i = 0; i < 3; i++) {
        xfm.m[i][i] = sat;
        xfm.m[i][ModChan(i + 1)] = f2;
        xfm.m[i][ModChan(i + 2)] = f2;
    }
    Multiply(mColorXfm, xfm, mColorXfm);
}

void RndColorXfm::AdjustLightness() {
    Transform tf58;
    tf58.Reset();
    float lit = mLightness / 100.0f;
    float f1 = 0;
    float f3;
    if (lit >= 0) {
        f3 = 1.0f - lit;
        f1 = lit;
    } else {
        f3 = lit + 1.0f;
    }
    tf58.m[2][2] = f3;
    tf58.m[1][1] = f3;
    tf58.m[0][0] = f3;
    tf58.v.Set(f1, f1, f1);
    Multiply(mColorXfm, tf58, mColorXfm);
}

void RndColorXfm::AdjustContrast() {
    Transform tf58;
    tf58.Reset();
    float contrast = mContrast / 100.0f;
    if (contrast > 0) {
        contrast = 1.0f / (contrast * -0.9921875f + 1.0f);
    } else {
        contrast = -(contrast * -0.992126f - 1);
    }
    float f2 = (1.0f - contrast) / 2;
    tf58.m[2][2] = contrast;
    tf58.m[1][1] = contrast;
    tf58.m[0][0] = contrast;
    tf58.v.Set(f2, f2, f2);
    Multiply(mColorXfm, tf58, mColorXfm);
}

void RndColorXfm::AdjustLevels() {
    Vector3 levelDeltas(
        mLevelInHi.red - mLevelInLo.red,
        mLevelInHi.green - mLevelInLo.green,
        mLevelInHi.blue - mLevelInLo.blue
    );
    float blueDiff;
    if (levelDeltas.z != 0) {
        blueDiff =
            (mLevelOutHi.blue - mLevelOutLo.blue) / (mLevelInHi.blue - mLevelInLo.blue);
    } else {
        blueDiff = 0;
    }
    float greenDiff;
    if (levelDeltas.y != 0) {
        greenDiff = (mLevelOutHi.green - mLevelOutLo.green)
            / (mLevelInHi.green - mLevelInLo.green);
    } else {
        greenDiff = 0;
    }
    float redDiff;
    if (levelDeltas.x != 0) {
        redDiff = (mLevelOutHi.red - mLevelOutLo.red) / (mLevelInHi.red - mLevelInLo.red);
    } else {
        redDiff = 0;
    }
    Vector3 v5c(redDiff, greenDiff, blueDiff);
    Vector3 v68(
        v5c.x * -mLevelInLo.red + mLevelOutLo.red,
        v5c.y * -mLevelInLo.green + mLevelOutLo.green,
        v5c.z * -mLevelInLo.blue + mLevelOutLo.blue
    );
    Transform tf40;
    tf40.m.x.Set(v5c.x, 0, 0);
    tf40.m.y.Set(0, v5c.y, 0);
    tf40.m.z.Set(0, 0, v5c.z);
    tf40.v = v68;
    Multiply(mColorXfm, tf40, mColorXfm);
}
