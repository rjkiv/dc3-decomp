#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "rndobj/ShaderOptions.h"
#include <list>

class RndShaderProgram;

#define PS3_SHADERS_TYPE 'PS3S'
#define PS3_SHADERS_VERSION 1
#define XBOX_SHADERS_TYPE 'XBOX'
#define XBOX_SHADERS_VERSION 1

// vertex shader constant
// each VShaderConstant directly corresponds to a D3DConstants' VertexShaderX
// 256 possible float values (VertexShaderF)
// 4 possible bool values (VertexShaderB)
// 16 possible int values (VertexShaderI)
enum VShaderConstant {
    // floats
    kVShader_EnvAmbientColor = 1,
    // ints
    kVShader_SplineMaxCtrlPoints = 12,
};

// pixel shader constant
// each PShaderConstant directly corresponds to a D3DConstants' PixelShaderX
// 256 possible float values (PixelShaderF)
// 4 possible bool values (PixelShaderB)
// 16 possible int values (PixelShaderI)
enum PShaderConstant {
    // floats
    kPShader_EnvAmbientColor = 1,
};

class RndShaderMgr {
public:
    // A binary search tree for a particular shader type.
    struct ShaderTree {
        ShaderType shaderType; // the shader type
        RndShaderProgram *tree; // the entrypoint for the BST
    };
    RndShaderMgr();
    virtual ~RndShaderMgr() {}
    virtual void PreInit();
    virtual void Init();
    virtual void Terminate();
    virtual RndMat *GetWork() { return mWorkMat; }
    virtual RndMat *GetPostProcMat() { return mPostProcMat; }
    virtual void SetVConstant(VShaderConstant, RndTex *) = 0;
    virtual void SetVConstant(VShaderConstant, const float *__restrict, unsigned int) = 0;
    virtual void SetVConstant(VShaderConstant, const Vector4 &) = 0; // 0x24
    virtual void SetVConstant(VShaderConstant, int) = 0;
    virtual void SetVConstant(VShaderConstant, bool) = 0;
    virtual void SetVConstant(VShaderConstant, const Hmx::Matrix4 &) = 0; // 0x18
    virtual void SetVConstant4x3(VShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual void SetPConstant(PShaderConstant, const Hmx::Matrix4 &) = 0;
    virtual void SetPConstant(PShaderConstant, RndCubeTex *) = 0;
    virtual void SetPConstant(PShaderConstant, const Vector4 &) = 0; // 0x40
    virtual void SetPConstant(PShaderConstant, RndTex *) = 0; // 0x3c
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
    void SetAllowPerPixel(bool allow) { mAllowPerPixel = allow; }
    void Invalidate(ShaderType);
    void ToggleShowMetaMatErrors() { mShowMetaMatErrors = !mShowMetaMatErrors; }
    void ToggleShowShaderErrors() { mShowShaderErrors = !mShowShaderErrors; }
    RndShaderProgram &FindShader(ShaderType, const ShaderOptions &);
    void *AllocShader();
    int Unk20() const { return unk20; }
    bool ShowShaderErrors() const { return mShowShaderErrors; }
    bool Unk18() const { return unk18; }
    bool ShowMetaMatErrors() const { return mShowMetaMatErrors; }
    int Unk10() const { return mNumBones; }
    bool UseAO() const { return mHasAOCalc; }
    bool Unk24() const { return unk24; }
    bool AllowPerPixel() const { return mAllowPerPixel; }
    bool Unk3a() const { return unk3a; }
    void SetUnk3a(bool b) { unk3a = b; }
    void SetUnk3e(bool b) { unk3e = b; }
    void SetUnk38(bool b) { unk38 = b; }
    void SetUnk39(bool b) { unk39 = b; }
    void SetUnk34(int i) { unk34 = i; }
    void SetUnk29(bool b) { unk29 = b; }
    void SetUnk2f(bool b) { unk2f = b; }

protected:
    virtual void LoadShaders(const char *filename);
    virtual void LoadShaderFile(FileStream &);
    virtual RndShaderProgram *NewShaderProgram() = 0;

    void ShaderPoolAlloc(int);

    // The collection of ShaderTrees; 1 ShaderTree per ShaderType.
    std::list<ShaderTree> mShaderTrees; // 0x4
    bool mHasAOCalc; // 0xc
    int mNumBones; // 0x10
    int unk14;
    bool unk18;
    int unk1c;
    int unk20; // 0x20 - some sort of enum
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
    bool unk38; // mMotionBlurChecked?
    bool unk39;
    bool unk3a; // mGradientMapChecked?
    bool unk3b;
    bool unk3c;
    bool unk3d;
    bool unk3e; // mVignetteChecked?
    bool unk3f;
    bool mAllowPerPixel; // 0x40
    bool unk41;
    bool mDisplayShaderError; // 0x42
    RndMat *mWorkMat; // 0x44
    RndMat *mPostProcMat; // 0x48
    RndMat *mDrawHighlightMat; // 0x4c
    RndMat *mDrawRectMat; // 0x50
    void *mShaderPool; // 0x54
    int mShaderPoolCount; // 0x58
    int unk5c; // 0x5c - shader pool alloc
    int unk60;
    float *mConstantCache; // 0x64
    int unk68;
    bool mCacheShaders; // 0x6c
    bool unk6d;
    bool mShowShaderErrors; // 0x6e
    bool mShowMetaMatErrors; // 0x6f
};

extern RndShaderMgr &TheShaderMgr;
