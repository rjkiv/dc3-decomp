#pragma once
#include "math/Mtx.h"
#include "rndobj/Mat.h"
#include <list>

class RndShaderProgram;

enum VShaderConstant {
};

enum PShaderConstant {
};

class RndShaderMgr {
public:
    struct ShaderTree {};
    RndShaderMgr();
    virtual ~RndShaderMgr();
    virtual void PreInit();
    virtual void Init();
    virtual void Terminate();
    virtual RndMat *GetWork() { return mWorkMat; }
    virtual RndMat *GetPostProcMat() { return mPostProcMat; }
    virtual void SetVConstant(VShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual void SetVConstant(VShaderConstant, RndTex *) = 0;
    virtual void SetVConstant(VShaderConstant, const float *, unsigned int) = 0;
    virtual void SetVConstant(VShaderConstant, const Vector4 &) = 0;
    virtual void SetVConstant(VShaderConstant, int) = 0;
    virtual void SetVConstant(VShaderConstant, bool) = 0;
    virtual void SetVConstant4x3(VShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual void SetPConstant(PShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual void SetPConstant(PShaderConstant, RndCubeTex *) = 0;
    virtual void SetPConstant(PShaderConstant, RndTex *) = 0;
    virtual void SetPConstant(PShaderConstant, const Vector4 &) = 0;
    virtual void SetPConstant(PShaderConstant, int) = 0;
    virtual void SetPConstant(PShaderConstant, bool) = 0;
    virtual void SetPConstant4x3(PShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual RndMat *DrawHighlightMat() { return mDrawHighlightMat; }
    virtual RndMat *DrawRectMat() { return mDrawRectMat; }

    bool CacheShaders() const { return mCacheShaders; }
    void UpdateCache(const Transform &, int);
    void SetMeshInfo(int, bool);
    void SetShaderErrorDisplay(bool);
    bool GetShaderErrorDisplay();
    unsigned long InitShaders();
    void SetTransform(const Transform &);

protected:
    virtual void LoadShaders(const char *);
    virtual void LoadShaderFile(FileStream &);
    virtual RndShaderProgram *NewShaderProgram() = 0;

    void ShaderPoolAlloc();

    std::list<ShaderTree> mShaderTrees; // 0x4
    bool unkc;
    int unk10;
    int unk14;
    bool unk18;
    int unk1c;
    int unk20;
    bool unk24;
    bool unk25;
    bool unk26;
    bool unk27;
    bool unk28;
    bool unk29;
    bool unk2a;
    bool unk2b;
    bool unk2c;
    bool unk2d;
    bool unk2e;
    bool unk2f;
    bool unk30;
    bool unk31;
    int unk34;
    bool unk38;
    bool unk39;
    bool unk3a;
    bool unk3b;
    bool unk3c;
    bool unk3d;
    bool unk3e;
    bool unk3f;
    bool unk40;
    bool unk41;
    bool mDisplayShaderError; // 0x42
    RndMat *mWorkMat; // 0x44
    RndMat *mPostProcMat; // 0x48
    RndMat *mDrawHighlightMat; // 0x4c
    RndMat *mDrawRectMat; // 0x50
    void *unk54;
    int mShaderPoolCount; // 0x58
    int unk5c; // 0x5c - shader pool alloc
    int unk60;
    float *mConstantCache; // 0x64
    int unk68;
    bool mCacheShaders; // 0x6c
    bool unk6d;
    bool unk6e;
    bool unk6f;
};

extern RndShaderMgr &TheShaderMgr;
