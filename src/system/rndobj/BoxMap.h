#pragma once
#include "math/Vec.h"
#include "math/Color.h"

template <class T1, int T2>
class BoxLightArray {
public:
    BoxLightArray() : mNumElements(0) {}
    const T1 &operator[](int idx) const { return mArray[idx]; }
    void clear() { mNumElements = 0; }
    unsigned int size() const { return mNumElements; }
    bool CanAdd() const { return mNumElements < T2; }
    void push_back(const T1 &item) { mArray[mNumElements++] = item; }
    void pop() { mNumElements--; }
    const T1 *end() const { return &mArray[mNumElements]; }
    T1 *Insert() { return &mArray[mNumElements++]; }

private:
    T1 mArray[T2]; // 0x0
    unsigned int mNumElements;
};

class BoxMapLighting {
public:
    // size 0x20
    struct LightParams_Directional {
        Vector3 mDirection; // 0x0
        Hmx::Color mColor; // 0x10
    };

    // size 0x28
    struct LightParams_Point {
        Vector3 mLightPos; // 0x0
        Hmx::Color mColor; // 0x10
        float mRange; // 0x20
        float mFalloffStart; // 0x24
    };

    // size 0x5c
    struct LightParams_Spot {
        struct CachedData {
            Vector3 mTipPosition; // 0x0
            float mCosTheta; // 0x10
            float mOneOverOneSubCos; // 0x14
            float mOneOverRange2x; // 0x18
            float mTipOverRange2x; // 0x1c
        };
        Vector3 mDirection; // 0x0
        Hmx::Color mColor; // 0x10
        CachedData mCachedData; // 0x20
        Vector3 mLightPos; // 0x40
        float mRange; // 0x50
        float mRadiusTop; // 0x54
        float mRadiusBottom; // 0x58
    };

    BoxMapLighting();
    void Clear();
    bool QueueLight(class RndLight *light, float scale);
    bool CacheData(LightParams_Spot &params);
    void ApplyQueuedLights(Hmx::Color *__restrict results, const Vector3 *position) const;

    unsigned int NumQueuedLights() const {
        return mQueued_Directional.size() + mQueued_Point.size() + mQueued_Spot.size();
    }

    bool QueueLight(LightParams_Directional *&pd) {
        if (mQueued_Directional.CanAdd()) {
            pd = mQueued_Directional.Insert();
            return true;
        } else
            return false;
    }
    bool QueueLight(LightParams_Point *&pt) {
        if (mQueued_Point.CanAdd()) {
            pt = mQueued_Point.Insert();
            return true;
        } else
            return false;
    }
    bool QueueLight(LightParams_Spot *&ps) {
        if (mQueued_Spot.CanAdd()) {
            ps = mQueued_Spot.Insert();
            return true;
        } else
            return false;
    }

private:
    void ApplyLight(const BoxLightArray<LightParams_Directional, 50> &paramArray) const;
    void ApplyLight(
        const BoxLightArray<LightParams_Point, 50> &paramArray, const Vector3 &position
    ) const;
    void ApplyLight(
        const BoxLightArray<LightParams_Spot, 50> &paramArray, const Vector3 &position
    ) const;

    BoxLightArray<LightParams_Directional, 50> mQueued_Directional; // 0x0
    BoxLightArray<LightParams_Point, 50> mQueued_Point; // 0x644
    BoxLightArray<LightParams_Spot, 50> mQueued_Spot; // 0xe18
};
