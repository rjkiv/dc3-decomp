#pragma once
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"

// size 0x1b584
class RndXfmCache {
    friend class RndVelocityBuffer;

    RndXfmCache() : unk0(), unk1f40(), unk19640(), unk1b580(0) {}

    bool GetXfms(
        const RndMesh *__restrict mesh, unsigned int, unsigned int, const float *&
    ) const;

    bool CacheXfms(
        const RndMesh *__restrict mesh,
        const float *__restrict floats,
        unsigned int,
        unsigned int &
    );

    RndMesh *unk0[2000]; // 0x0
    float unk1f40[2000][12]; // 0x1f40
    int unk19640[2000]; // 0x19640
    unsigned int unk1b580; // 0x1b580
};

// size 0x36c84
class RndVelocityBuffer {
public:
    virtual ~RndVelocityBuffer() { FreeData(); }

    void CacheCameraSettings(RndCam *);
    void AllocateData(unsigned int, unsigned int, unsigned int);
    void FreeData();
    void ResetFrame();
    bool Draw(RndCam *, ObjPtrList<RndDrawable> &);
    void DrawMesh(RndMesh *) const;
    void CacheTransform(class RndMesh *__restrict, float const *__restrict, unsigned int);

    static RndVelocityBuffer &Singleton() { return sSingleton; }

    float GetUnk36be8() { return unk36be8; };

private:
    RndVelocityBuffer();

    bool AdvanceFrame(RndCam *);

    static RndVelocityBuffer sSingleton;

    Hmx::Matrix4 unk8; // 0x8
    Vector4 unk48; // 0x48
    Vector3 unk58; // 0x58
    Vector3 unk68[4]; // 0x68
    RndCam *mCam; // 0xa8
    RndXfmCache mXfmCaches[2]; // 0xac
    Timer mTimer; // 0x36bb8
    float unk36be8; // 0x36be8
    Hmx::Matrix4 unk36bec[2]; // 0x36bec
    int unk36c6c; // 0x36c6c - which xfmcache
    int mFrame; // 0x36c70 - frame
    RndTex *mVelocityTex; // 0x36c74
    RndMat *mMat; // 0x36c78
    RndCam *unk36c7c; // 0x36c7c
    bool unk36c80; // 0x36c80
};
