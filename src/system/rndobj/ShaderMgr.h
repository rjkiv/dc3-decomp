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
    // genuinely i do not understand the vtable offset ordering going on here
    virtual void SetVConstant(VShaderConstant, RndTex *) = 0;
    virtual void SetVConstant(VShaderConstant, int) = 0;
    virtual void SetVConstant(VShaderConstant, const Vector4 &) = 0; // 0x24
    virtual void
    SetVConstant(VShaderConstant, const float *__restrict, unsigned int) = 0; // 0x20
    virtual void SetVConstant(VShaderConstant, bool) = 0;
    virtual void SetVConstant(VShaderConstant, const Hmx::Matrix4 &) = 0; // 0x18
    virtual void SetVConstant4x3(VShaderConstant, const Hmx::Matrix4 &) = 0; // 0x30
    virtual void SetPConstant(PShaderConstant, const Hmx::Matrix4 &) = 0; // 0x48
    virtual void SetPConstant(PShaderConstant, int) = 0;
    virtual void SetPConstant(PShaderConstant, const Vector4 &) = 0; // 0x40
    virtual void SetPConstant(PShaderConstant, RndTex *) = 0; // 0x3c
    virtual void SetPConstant(PShaderConstant, RndCubeTex *) = 0; // 0x38
    virtual void SetPConstant(PShaderConstant, bool) = 0;
    virtual void SetPConstant4x3(PShaderConstant, const Hmx::Matrix4 &) = 0; // 0x4c
    virtual RndMat *DrawHighlightMat() { return mDrawHighlightMat; }
    virtual RndMat *DrawRectMat() { return mDrawRectMat; }

    float *ConstantCache() const { return mConstantCache; }
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
    int NumBones() const { return mNumBones; }
    bool UseAO() const { return mHasAOCalc; }
    bool Unk24() const { return unk24; }
    bool AllowPerPixel() const { return mAllowPerPixel; }
    bool GradientMapEnabled() const { return mGradientMapEnabled; }
    void SetGradientMapEnabled(bool b) { mGradientMapEnabled = b; }
    void SetVignetteEnabled(bool b) { mVignetteEnabled = b; }
    void SetMotionBlurEnabled(bool b) { mMotionBlurEnabled = b; }
    void SetUnk39(bool b) { unk39 = b; }
    void SetUnk34(int i) { unk34 = i; }
    void SetColorXfmEnabled(bool b) { mColorXfmEnabled = b; }
    void SetUnk2f(bool b) { unk2f = b; }
    void SetUnk30(bool b) { unk30 = b; }
    int NumTaps() const { return mNumTaps; }
    void SetNumTaps(int i) { mNumTaps = i; }
    void SetPosterizeEnabled(bool b) { mPosterizeEnabled = b; }
    void SetKaleidoscopeEnabled(bool b) { mKaleidoscopeEnabled = b; }
    void SetHueConvergeEnabled(bool b) { mHueConvergeEnabled = b; }
    void SetUnk3b(bool b) { unk3b = b; }
    void SetUnk2d(bool b) { unk2d = b; }
    void SetUnk2e(bool b) { unk2e = b; }
    void SetChromaticAberrationEnabled(bool b) { mChromaticAberrationEnabled = b; }
    void SetChromaticSharpenEnabled(bool b) { mChromaticSharpenEnabled = b; }
    void SetUnk27(bool b) { unk27 = b; }
    void SetUnk28(bool b) { unk28 = b; }
    void SetDOFEnabled(bool b) { mDOFEnabled = b; }
    void SetUnk3f(bool b) { unk3f = b; }

protected:
    virtual void LoadShaders(const char *filename);
    virtual void LoadShaderFile(FileStream &);
    virtual RndShaderProgram *NewShaderProgram() = 0;

    void ShaderPoolAlloc(int);

    // The collection of ShaderTrees; 1 ShaderTree per ShaderType.
    std::list<ShaderTree> mShaderTrees; // 0x4
    bool mHasAOCalc; // 0xc
    int mNumBones; // 0x10
    int mNumTaps; // 0x14
    bool unk18;
    int unk1c;
    int unk20; // 0x20 - some sort of enum
    bool unk24;

    // all of these are various postproc flags
    bool unk25;
    bool mDOFEnabled; // 0x26
    bool unk27; // 0x27 - bloom related
    bool unk28; // 0x28 - bloom related
    bool mColorXfmEnabled; // 0x29
    bool mHueConvergeEnabled; // 0x2a
    bool mPosterizeEnabled; // 0x2b
    bool mKaleidoscopeEnabled; // 0x2c
    bool unk2d;
    bool unk2e;
    bool unk2f;
    bool unk30;
    bool unk31;
    int unk34;
    bool mMotionBlurEnabled; // 0x38
    bool unk39;
    bool mGradientMapEnabled; // 0x3a
    bool unk3b; // 0x3b
    bool mChromaticAberrationEnabled; // 0x3c
    bool mChromaticSharpenEnabled; // 0x3d
    bool mVignetteEnabled; // 0x3e
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
    bool mInitted; // 0x6d
    bool mShowShaderErrors; // 0x6e
    bool mShowMetaMatErrors; // 0x6f
};

extern RndShaderMgr &TheShaderMgr;
