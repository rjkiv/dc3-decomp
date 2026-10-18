#pragma once
#include "math/Mtx.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"

// size 0x1b584
class RndXfmCache {
    friend class RndVelocityBuffer;

    RndXfmCache() { Reset(); }

    void Reset() {
        memset(mEntries_Mesh, 0, sizeof(mEntries_Mesh));
        memset(mEntries_Xfm, 0, sizeof(mEntries_Xfm));
        memset(mEntries_Bone, 0, sizeof(mEntries_Bone));
        mEntryCount = 0;
    }

    bool GetXfms(
        const RndMesh *__restrict mesh,
        unsigned int key,
        unsigned int numXfms,
        const float *&xfmArray
    ) const;

    bool CacheXfms(
        const RndMesh *__restrict mesh,
        const float *__restrict xfmArray,
        unsigned int numXfms,
        unsigned int &key
    );

    static const unsigned int kMaxItems = 2000;

    // 1 mesh entry per compacted xfm
    const RndMesh *mEntries_Mesh[kMaxItems]; // 0x0
    // 1 compacted xfm == 12 floats
    float mEntries_Xfm[kMaxItems * 12]; // 0x1f40
    unsigned int mEntries_Bone[kMaxItems]; // 0x19640
    unsigned int mEntryCount; // 0x1b580
};

// size 0x36c84
class RndVelocityBuffer {
public:
    // size 0xa4
    struct CameraSettings {
        Hmx::Matrix4 mViewProj; // 0x0
        Vector4 mZRange; // 0x40
        Vector3 mCamPos; // 0x50
        Vector3 mCamCorner[4]; // 0x60
        RndCam *mCachedCam; // 0xa0
    };

    virtual ~RndVelocityBuffer() { FreeData(); }

    void CacheCameraSettings(RndCam *camera);
    void AllocateData(unsigned int nWidth, unsigned int nHeight, unsigned int nBpp);
    void FreeData();
    void ResetFrame();
    bool Draw(RndCam *camera, ObjPtrList<RndDrawable> &drawList);
    void DrawMesh(RndMesh *mesh) const;
    void CacheTransform(
        RndMesh *__restrict mesh, const float *__restrict xfmArray, unsigned int numXfms
    );

    static RndVelocityBuffer &Singleton() { return sSingleton; }

    float GetIntensity() { return mIntensity; };

private:
    RndVelocityBuffer();

    bool AdvanceFrame(RndCam *camera);

    static RndVelocityBuffer sSingleton;

    CameraSettings mCamSettings; // 0x8
    RndXfmCache mCache[2]; // 0xac
    Timer mTimer; // 0x36bb8
    float mIntensity; // 0x36be8
    Hmx::Matrix4 mViewProj[2]; // 0x36bec
    unsigned int mFrame; // 0x36c6c
    unsigned int mNumFrames; // 0x36c70
    RndTex *mVelocityTex; // 0x36c74
    RndMat *mMat; // 0x36c78
    RndCam *mLastCamera; // 0x36c7c
    bool mReadyToUse; // 0x36c80
};
