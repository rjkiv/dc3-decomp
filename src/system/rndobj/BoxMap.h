#pragma once
#include "math/Vec.h"
#include "math/Color.h"

class RndLight;

template <class T1, int T2>
class BoxLightArray {
public:
    BoxLightArray() : mNumElements(0) {}
    void Clear() { mNumElements = 0; }
    bool CanAddEntry() const { return mNumElements < T2; }
    T1 *AddEntry() { return &mArray[mNumElements++]; }
    void RemoveEntry() { mNumElements--; }
    unsigned int NumElements() const { return mNumElements; }
    const T1 &operator[](int idx) const { return mArray[idx]; }

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
        Vector3 unk0; // 0x0
        Hmx::Color mColor; // 0x10
        float mRange; // 0x20
        float mFalloffStart; // 0x24
    };

    // size 0x5c
    struct LightParams_Spot {
        Vector3 unk0; // 0x0
        Hmx::Color mColor; // 0x10
        Vector3 unk20; // 0x20
        float unk30; // 0x30
        float unk34; // 0x34
        float unk38; // 0x38
        float unk3c; // 0x3c
        Vector3 unk40; // 0x40
        float unk50; // 0x50 - beam length?
        float unk54; // 0x54 - top beam radius
        float unk58; // 0x58 - bottom beam radius
    };

    BoxMapLighting();
    void Clear();
    bool QueueLight(RndLight *, float);
    bool CacheData(LightParams_Spot &);
    void ApplyQueuedLights(Hmx::Color *, const Vector3 *) const;

    unsigned int NumQueuedLights() const {
        return mQueued_Directional.NumElements() + mQueued_Point.NumElements()
            + mQueued_Spot.NumElements();
    }

    bool ParamsAt(LightParams_Directional *&pd) {
        if (mQueued_Directional.CanAddEntry()) {
            pd = mQueued_Directional.AddEntry();
            return true;
        } else
            return false;
    }
    bool ParamsAt(LightParams_Point *&pt) {
        if (mQueued_Point.CanAddEntry()) {
            pt = mQueued_Point.AddEntry();
            return true;
        } else
            return false;
    }
    bool ParamsAt(LightParams_Spot *&ps) {
        if (mQueued_Spot.CanAddEntry()) {
            ps = mQueued_Spot.AddEntry();
            return true;
        } else
            return false;
    }

private:
    void ApplyLight(const BoxLightArray<LightParams_Directional, 50> &) const;
    void ApplyLight(const BoxLightArray<LightParams_Point, 50> &, const Vector3 &) const;
    void ApplyLight(const BoxLightArray<LightParams_Spot, 50> &, const Vector3 &) const;

    // static Vector3 sAxisDir[6];

    BoxLightArray<LightParams_Directional, 50> mQueued_Directional; // 0x0
    BoxLightArray<LightParams_Point, 50> mQueued_Point; // 0x5644
    BoxLightArray<LightParams_Spot, 50> mQueued_Spot; // 0xe18
};
