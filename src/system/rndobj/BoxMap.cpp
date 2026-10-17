#include "rndobj/BoxMap.h"
#include "math/Vec.inl"
#include "rndobj/Lit.h"

BoxMapLighting::BoxMapLighting() { Clear(); }

void BoxMapLighting::Clear() {
    mQueued_Directional.clear();
    mQueued_Point.clear();
    mQueued_Spot.clear();
}

bool BoxMapLighting::QueueLight(RndLight *light, float colorScale) {
    if (light->Showing()) {
        Hmx::Color lightColor(light->GetColor());
        lightColor.red *= colorScale;
        lightColor.green *= colorScale;
        lightColor.blue *= colorScale;
        switch (light->LightType()) {
        case RndLight::kDirectional:
        case RndLight::kFakeSpot:
            LightParams_Directional *paramsDirectional;
            if (QueueLight(paramsDirectional)) {
                paramsDirectional->mColor = lightColor;
                Negate(light->WorldXfm().m.y, paramsDirectional->mDirection);
                return true;
            }
            break;
        case RndLight::kPoint:
            LightParams_Point *paramsPoint;
            if (QueueLight(paramsPoint)) {
                paramsPoint->mLightPos = light->WorldXfm().v;
                paramsPoint->mColor = lightColor;
                paramsPoint->mRange = light->Range();
                paramsPoint->mFalloffStart = light->FalloffStart();
                return true;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

bool BoxMapLighting::CacheData(LightParams_Spot &spot) {
    if (spot.mRange > 0 && spot.mRadiusBottom >= spot.mRadiusTop
        && (spot.mColor.red > (1 / 255.0f) || spot.mColor.green > (1 / 255.0f)
            || spot.mColor.blue > (1 / 255.0f))) {
        float f3 =
            (spot.mRadiusTop * spot.mRange) / (spot.mRadiusBottom - spot.mRadiusTop);
        Vector3 v58;
        Scale(spot.mDirection, f3, v58);
        Vector3 v4c;
        Subtract(spot.mLightPos, v58, v4c);
        float f1 = spot.mRadiusBottom / (spot.mRange + f3);
        f1 *= f1;
        float f2 = 1.0f / (spot.mRange * 2.0f);
        f1 = (1.0f - f1) / (f1 + 1.0f);
        spot.mCachedData.mTipPosition = v4c;
        spot.mCachedData.mCosTheta = f1;
        spot.mCachedData.mOneOverOneSubCos = 1.0f / (1.0f - f1);
        spot.mCachedData.mOneOverRange2x = f2;
        spot.mCachedData.mTipOverRange2x = f3 * f2;
        return true;
    } else {
        mQueued_Spot.pop();
        return false;
    }
}

static unsigned int s_arraySize = 0;
static Vector3 s_arrayDir[150];
static Hmx::Color s_arrayCol[150];

void BoxMapLighting::ApplyLight(
    const BoxLightArray<BoxMapLighting::LightParams_Directional, 50> &boxArray
) const {
    for (int i = 0; i < boxArray.size(); i++) {
        s_arrayDir[s_arraySize] = boxArray[i].mDirection;
        s_arrayCol[s_arraySize++] = boxArray[i].mColor;
    }
}

void BoxMapLighting::ApplyLight(
    const BoxLightArray<BoxMapLighting::LightParams_Point, 50> &boxArray,
    const Vector3 &vec
) const {
    for (int i = 0; i < boxArray.size(); i++) {
        if (boxArray[i].mRange > boxArray[i].mFalloffStart) {
            Subtract(boxArray[i].mLightPos, vec, s_arrayDir[s_arraySize]);
            float lensq = LengthSquared(s_arrayDir[s_arraySize]);
            if (lensq > 0) {
                float inv = __frsqrte(lensq);
                float f3 = Min(0.0f, inv * lensq - boxArray[i].mFalloffStart);
                float f4 =
                    Min(0.0f, 1 - f3 / (boxArray[i].mRange - boxArray[i].mFalloffStart));
                s_arrayCol[s_arraySize].red = boxArray[i].mColor.red * f4;
                s_arrayCol[s_arraySize].green = boxArray[i].mColor.green * f4;
                s_arrayCol[s_arraySize].blue = boxArray[i].mColor.blue * f4;
                s_arrayDir[s_arraySize++] *= inv;
            }
        }
    }
}
