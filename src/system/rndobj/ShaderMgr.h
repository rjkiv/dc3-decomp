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
// names and values are from the PDB
enum VShaderConstant {
    kVShader_DiffuseCol = 0,
    kVShader_AmbientCol = 1,
    kVShader_SpecularCol = 2,
    kVShader_Reserved0 = 3,
    kVShader_ViewProj = 4,
    kVShader_Camera = 16,
    kVShader_SpecularColor2 = 19,
    kVShader_TexCoord = 20,
    kVShader_AOStrength = 24,
    kVShader_SplineParams = 25,
    kVShader_SplinePulseParams = 26,
    kVShader_SyncTrackChargeEffectParams = 27,
    kVShader_SyncTrackChargeEffectParams2 = 28,
    kVShader_SyncTrackChargeEffectAxis = 29,
    kVShader_ShockwaveCenter = 30,
    kVShader_ShockwaveAxis = 31,
    kVShader_ShockwaveParams = 32,
    kVShader_WorldToLightProj = 40,
    kVShader_ExtrudePos = 44,
    kVShader_ExtrudeDir = 45,
    kVShader_ExtrudePlane = 46,
    kVShader_ParticleRight = 47,
    kVShader_ParticleUp = 48,
    kVShader_ParticleRot = 49,
    kVShader_ParticleTilesData = 50,
    kVShader_kUVAnimParticle = 51,
    // fur, seems to share registers with the particle ones above?
    kVShader_Stretch = 50,
    kVShader_Shell = 51,
    // fade out
    kVShader_FadeLeft = 53,
    kVShader_FadeRight = 54,
    kVShader_FadeRange = 55,
    kVShader_RimCol = 61,
    kVShader_Light0 = 62,
    kVShader_Light1 = 63,
    kVShader_Light2 = 64,
    kVShader_Light3 = 65,
    kVShader_LightCol0 = 66,
    kVShader_LightCol1 = 67,
    kVShader_LightCol2 = 68,
    kVShader_LightCol3 = 69,
    kVShader_NumLights = 4,
    kVShader_ScreenOffset = 70,
    kVShader_Reserved1 = 79,
    kVShader_BoxLightApprox = 80,
    kVShader_MultiMeshIndices = 86,
    kVShader_FogRange = 91,
    // world/bone transforms, xfms are 4x3 so they get 3 registers each. 43 of them fill
    // 92-220, so the end slot is 221
    kVShader_TransformNumSlots = 3,
    kVShader_TransformList0 = 92,
    kVShader_TransformListCount = 43,
    kVShader_TransformEndSlot = 221,
    // splines take over 173-220 to get 12 control points (4 registers each)
    // which leaves room for 27 xfms
    kVShader_SplineCtrlPointNumSlots = 4,
    kVShader_SplineMaxCtrlPoints = 12,
    kVShader_SplineCtrlPointEnd = 221,
    kVShader_SplineCtrlPointStart = 173,
    kVShader_SplineTransformListEnd = 173,
    kVShader_SplineTransformListCount = 27,
    kVShader_LightTransform0 = 221
};

// pixel shader constant
// each PShaderConstant directly corresponds to a D3DConstants' PixelShaderX
// 256 possible float values (PixelShaderF)
// 4 possible bool values (PixelShaderB)
// 16 possible int values (PixelShaderI)
// names and values are from the PDB
enum PShaderConstant {
    kPShader_DiffuseCol = 0,
    kPShader_AmbientCol = 1,
    kPShader_SpecularCol = 2,
    kPShader_ShaderCost = 4,
    kPShader_Multipliers = 5,
    kPShader_BloomIntensity = 6,
    kPShader_BloomColor = 7,
    kPShader_PosterLevels = 8,
    kPShader_DepthHalf = 9,
    kPShader_CamPos = 10,
    kPShader_FurRemap = 11,
    kPShader_FurTint = 12,
    kPShader_Anisotropy = 13,
    kPShader_DeNormal = 14,
    kPShader_TexelOffset = 15,
    kPShader_SpecularColor2 = 19,
    kPShader_TexCoord = 20,
    kPShader_DepthOfField = 24,
    kPShader_SpotPos = 25,
    kPShader_SpotDir = 26,
    kPShader_SpotToCam = 27,
    kPShader_Const_u = 28,
    kPShader_Radii = 29,
    kPShader_CamDir = 30,
    // env fade out
    kPShader_FadeLeft = 53,
    kPShader_FadeRight = 54,
    kPShader_FadeRange = 55,
    kPShader_RimCol = 61,
    kPShader_Light0 = 62,
    kPShader_Light1 = 63,
    kPShader_Light2 = 64,
    kPShader_Light3 = 65,
    kPShader_LightCol0 = 66,
    kPShader_LightCol1 = 67,
    kPShader_LightCol2 = 68,
    kPShader_LightCol3 = 69,
    kPShader_NumLights = 4,
    kPShader_ScreenOffset = 70,
    kPShader_Reserved1 = 71,
    kPShader_Reserved2 = 79,
    kPShader_BoxLightApprox = 80,
    kPShader_XSectionStrength = 86,
    kPShader_PlaneRight = 87,
    kPShader_PlaneLeft = 88,
    kPShader_ZRange = 89,
    kPShader_FogColor = 90,
    kPShader_FogRange = 91,
    kPShader_ColorXfm = 92,
    kPShader_ProjXfm = 95,
    kPShader_FadeColor = 104,
    kPShader_MotionBlurParams = 105,
    kPShader_Normal2Parm = 106,
    kPShader_ShadowColor = 107,
    kPShader_ShadowDir = 108,
    kPShader_ColorXfmEnv = 109,
    // all the postproc stuff
    kPShader_NoisePhase = 112,
    kPShader_NoiseScale = 113,
    kPShader_Resamp = 114,
    kPShader_HallOfTimeParams = 115,
    kPShader_HallOfTimeColor = 116,
    kPShader_KaleidoscopeParams = 117,
    kPShader_GradientMapParams = 118,
    kPShader_RefractParams = 119,
    kPShader_RefractScaleOffset = 120,
    kPShader_ChromaticAberration = 121,
    kPShader_VelocityIntenity = 122,
    kPShader_VignetteParams = 123,
    kPShader_ToneMapParams = 124,
    kPShader_PreviousSelect = 125,
    kPShader_FogRamp = 126,
    kPShader_FogDensity = 127,
    kPShader_FogXfm = 128,
    kPShader_FogAmbient = 129,
    kPShader_ScreenSize = 130,
    kPShader_ColorMod = 131,
    kPShader_ViewProj_Prev = 134,
    kPShader_BlurSampleOffsets = 138,
    kPShader_BlurSampleWeights = 154,
    kPShader_WorldProjection = 220,
    kPShader_LightTransform0 = 221,
    kPShader_HueConvergeParams = 222,
    // material samplers
    kPShader_DiffuseMap = 0,
    kPShader_NormalMap = 1,
    kPShader_SpecularMap = 2,
    kPShader_GlowMap = 3,
    kPShader_EnvironMap = 4,
    kPShader_ShadowMap = 5,
    kPShader_DiffuseMap2 = 6,
    kPShader_FloorSpot = 10,
    kPShader_FurDetail = 12,
    kPShader_CubeMap = 13,
    kPShader_NormDetail = 14,
    kPShader_RimMap = 15,
    // postproc samplers, which just reuse the material slots
    kPShader_RefractionMap = 1,
    kPShader_GradientMap = 3,
    kPShader_SoftDepthMap = 4,
    kPShader_FogDensityMap = 5,
    kPShader_FocalBlurHiRes = 6,
    kPShader_BloomSource = 7,
    kPShader_BloomMap0 = 7,
    kPShader_FocalBlurLoRes = 8,
    kPShader_FrontBufferDepth = 9,
    kPShader_VelocityBuffer = 10,
    kPShader_SpotXSectionMap = 11,
    kPShader_BloomMap1 = 11,
    kPShader_FogDepth = 12,
    kPShader_NoiseMap = 13,
    kPShader_PreviousFrame = 14,
    kPShader_BloomMap2 = 15
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
    virtual RndMat *GetWork() { return mWork; }
    virtual RndMat *GetPostProcMat() { return mPostProc; }
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
    virtual RndMat *DrawHighlightMat() { return mHighlight; }
    virtual RndMat *DrawRectMat() { return mDrawRect; }

    float *ConstantCache() const { return mConstantCache; }
    bool CacheShaders() const { return mCacheShaders; }
    void UpdateCache(const Transform &, int);
    void SetMeshInfo(int, bool);
    void SetShaderErrorDisplay(bool);
    bool GetShaderErrorDisplay();
    unsigned long InitShaders();
    void SetTransform(const Transform &);
    void Invalidate(ShaderType);
    void ToggleShowMetaMatErrors() { mShowMetaMatErrors = !mShowMetaMatErrors; }
    void ToggleShowShaderErrors() { mShowShaderErrors = !mShowShaderErrors; }
    RndShaderProgram &FindShader(ShaderType, const ShaderOptions &);
    void *AllocShader();
    void SetNumTaps(int taps) { mNumTaps = taps; }
    void SetForceCullMode(int mode) { mCull = mode; }
    void SetDepthVolume(bool b) { mDepthVolume = b; }
    void SetDepthShape(int shape) { mDepthShape = shape; }
    void SetSubtract(bool b) { mSubtract = b; }
    void SetPostSpotlight(bool b) { mPostSpotlight = b; }
    void SetPostDOF(bool b) { mPostDOF = b; }
    void SetPostBloom(bool b) { mPostBloom = b; }
    void SetPostGlare(bool b) { mPostGlare = b; }
    void SetPostColorXfm(bool b) { mPostColorXfm = b; }
    void SetPostHueConverge(bool b) { mPostHueConverge = b; }
    void SetPostPosterize(bool b) { mPostPosterize = b; }
    void SetPostKaleidoscope(bool b) { mPostKaleidoscope = b; }
    void SetPostNoise(bool b) { mPostNoise = b; }
    void SetPostNoiseMidtone(bool b) { mPostNoiseMidtone = b; }
    void SetPostBlendPrevious(bool b) { mPostBlendPrevious = b; }
    void SetPostCopyPrevious(bool b) { mPostCopyPrevious = b; }
    void SetPostResamp(bool b) { mPostResamp = b; }
    void SetPostHallOfTime(int b) { mPostHallOfTime = b; }
    void SetPostMotionBlur(bool b) { mPostMotionBlur = b; }
    void SetPostVelocity(bool b) { mPostVelocity = b; }
    void SetPostGradientMap(bool b) { mPostGradientMap = b; }
    void SetPostRefract(bool b) { mPostRefract = b; }
    void SetPostChromaticAberration(bool b) { mPostChromaticAberration = b; }
    void SetPostChromaticSharpen(bool b) { mPostChromaticSharpen = b; }
    void SetPostVignette(bool b) { mPostVignette = b; }
    void SetPostSoftDepth(bool b) { mPostSoftDepth = b; }
    bool UseAO() const { return mUseAO; }
    int NumBones() const { return mNumBones; }
    int NumTaps() const { return mNumTaps; }
    bool DepthVolume() const { return mDepthVolume; }
    int DepthShape() const { return mDepthShape; }
    int ForceCullMode() const { return mCull; }
    bool Subtract() const { return mSubtract; }
    bool PostSpotlight() const { return mPostSpotlight; }
    bool PostDOF() const { return mPostDOF; }
    bool PostBloom() const { return mPostBloom; }
    bool PostGlare() const { return mPostGlare; }
    bool PostColorXfm() const { return mPostColorXfm; }
    bool PostHueConverge() const { return mPostHueConverge; }
    bool PostPosterize() const { return mPostPosterize; }
    bool PostKaleidoscope() const { return mPostKaleidoscope; }
    bool PostNoise() const { return mPostNoise; }
    bool PostNoiseMidtone() const { return mPostNoiseMidtone; }
    bool PostBlendPrevious() const { return mPostBlendPrevious; }
    bool PostCopyPrevious() const { return mPostCopyPrevious; }
    bool PostResamp() const { return mPostResamp; }
    int PostHallOfTime() const { return mPostHallOfTime; }
    bool PostMotionBlur() const { return mPostMotionBlur; }
    bool PostVelocity() const { return mPostVelocity; }
    bool PostGradientMap() const { return mPostGradientMap; }
    bool PostRefract() const { return mPostRefract; }
    bool PostChromaticAberration() const { return mPostChromaticAberration; }
    bool PostChromaticSharpen() const { return mPostChromaticSharpen; }
    bool PostVignette() const { return mPostVignette; }
    bool PostSoftDepth() const { return mPostSoftDepth; }
    void SetAllowPerPixel(bool allow) { mAllowPerPixel = allow; }
    bool AllowPerPixel() const { return mAllowPerPixel; }
    void SetPrecacheOffscreen(bool b) { mPrecacheOffscreen = b; }
    bool GetPrecacheOffscreen() const { return mPrecacheOffscreen; }
    bool GetShowShaderErrors() const { return mShowShaderErrors; }
    bool GetShowMetaMatErrors() const { return mShowMetaMatErrors; }

protected:
    virtual void LoadShaders(const char *filename);
    virtual void LoadShaderFile(FileStream &);
    virtual RndShaderProgram *NewShaderProgram() = 0;

    void ShaderPoolAlloc(int);

    // The collection of ShaderTrees; 1 ShaderTree per ShaderType.
    std::list<ShaderTree> mShaderTrees; // 0x4
    bool mUseAO; // 0xc
    int mNumBones; // 0x10
    int mNumTaps; // 0x14
    bool mDepthVolume;
    int mDepthShape;
    int mCull; // 0x20 - some sort of enum
    bool mSubtract;

    // all of these are various postproc flags
    bool mPostSpotlight;
    bool mPostDOF; // 0x26
    bool mPostBloom; // 0x27 - bloom related
    bool mPostGlare; // 0x28 - bloom related
    bool mPostColorXfm; // 0x29
    bool mPostHueConverge; // 0x2a
    bool mPostPosterize; // 0x2b
    bool mPostKaleidoscope; // 0x2c
    bool mPostNoise;
    bool mPostNoiseMidtone;
    bool mPostBlendPrevious;
    bool mPostCopyPrevious;
    bool mPostResamp;
    int mPostHallOfTime;
    bool mPostMotionBlur; // 0x38
    bool mPostVelocity;
    bool mPostGradientMap; // 0x3a
    bool mPostRefract; // 0x3b
    bool mPostChromaticAberration; // 0x3c
    bool mPostChromaticSharpen; // 0x3d
    bool mPostVignette; // 0x3e
    bool mPostSoftDepth;

    bool mAllowPerPixel; // 0x40
    bool mPrecacheOffscreen;
    bool mShaderErrorDisplay; // 0x42
    RndMat *mWork; // 0x44
    RndMat *mPostProc; // 0x48
    RndMat *mHighlight; // 0x4c
    RndMat *mDrawRect; // 0x50
    void *mShaderPool; // 0x54
    int mShaderPoolCount; // 0x58
    int mPendingShaderPoolCount; // 0x5c - shader pool alloc
    int mShaderSize;
    float *mConstantCache; // 0x64
    int mCacheSize;
    bool mCacheShaders; // 0x6c
    bool mPreInitialized; // 0x6d
    bool mShowShaderErrors; // 0x6e
    bool mShowMetaMatErrors; // 0x6f
};

extern RndShaderMgr &TheShaderMgr;
