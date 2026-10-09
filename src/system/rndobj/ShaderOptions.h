#pragma once
#include "types.h"
#include "rndobj/Mat.h"
#include "utl/Str.h"
#include <vector>

enum ShaderType {
    // used for bloom
    kBloomShader = 0,
    // blur pass for bloom/dof/shadows/soft particles
    kBlurShader = 1,
    // depth volumes
    kDepthVolumeShader = 2,
    // plain downsampling
    kDownsampleShader = 3,
    // fancy downsampling for bloom and dof
    kDownsample4xShader = 4,
    // downsampling the depth buffer
    kDownsampleDepthShader = 5,
    // any screenspace/engine draw stuff (used by stuff like Rnd::DrawString etc.)
    kDrawRectShader = 6,
    // all black shader used when stuff is broke
    kErrorShader = 7,
    // furry
    kFurShader = 8,
    // debug lines without z test
    kLineNozShader = 9,
    // debug lines 2: now with z test
    kLineShader = 10,
    // used when playing binks
    kMovieShader = 11,
    // instanced multimesh
    kMultimeshShader = 12,
    // instanced multimesh, but billboarded
    kMultimeshBBShader = 13,
    // particles
    kParticlesShader = 14,
    // postproc error shader, equivalent to no postprocessing at all when something broke
    kPostprocessErrorShader = 15,
    // fullscreen postproc
    kPostprocessShader = 16,
    // depth into a shadow map (kDrawShadowDepth)
    kShadowmapShader = 17,
    // basic mesh shader, used by majority of mats
    kStandardShader = 18,
    // standard, but billboarded
    kStandardBBShader = 19,
    // sync track meshes?
    kSyncTrackShader = 20,
    // sync track, but with the charge effect on?
    kSyncTrackChargeEffectShader = 21,
    // draws a mesh in uv space, TexBlender uses it
    kUnwrapUVShader = 22,
    // velocity from just the cam motion alone, for motion blur
    kVelocityCameraShader = 23,
    // per-object velocity for motion blur (kDrawVelocity)
    kVelocityObjectShader = 24,
    // used for kinect depth vis?
    kPlayerDepthVisShader = 25,
    // presumably also used for something kinect related
    kPlayerDepthShellShader = 26,
    // bloom w/ glare
    kBloomGlareShader = 27,
    // dc2 leftover? kinect related
    kPlayerDepthShell2Shader = 28,
    // kinect depth buffer, but as a 3D mesh (DepthBuffer3D)
    kDepthBuffer3DShader = 29,
    // kinect color camera, yuv to rgb
    kYUVtoRGBShader = 30,
    // kinect color camera, yuv to black and white
    kYUVtoBlackAndWhiteShader = 31,
    // kinect color camera, players without the background
    kPlayerGreenScreenShader = 32,
    // kinect color camera, players with the depth buffer
    kPlayerDepthGreenScreenShader = 33,
    // kinect color, edge detected background and blurred players?
    kCrewPhotoShader = 34,
    // twirl distortion, used for TexProc
    kTwirlShader = 35,
    // keeps color but sets alpha to 1, also used for TexProc
    kKillAlphaShader = 36,
    // standard shader but all white
    kAllWhiteShader = 37,
    kMaxShaderTypes = 38
};

// Like https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmacro, but at home
struct ShaderMacro {
    ShaderMacro(const char *n = nullptr, const char *v = nullptr) : Name(n), Value(v) {}

    ShaderMacro &operator=(const ShaderMacro &other) {
        this->Name = other.Name;
        this->Value = other.Value;
        return *this;
    }

    const char *Name; // 0x0
    const char *Value; // 0x4
};

struct ShaderOptions {
    typedef u64 Type;

    ShaderOptions(Type v) : value(v) {}
    ShaderOptions() : value(0) {}

    // several fields share bits, depending on which shader the options are for
    union {
        bf<Type, 0, 1> perPixel;
        bf<Type, 1, 1> specularMap;
        bf<Type, 1, 1> posterize;
        bf<Type, 1, 1> movieGrayscale;
        bf<Type, 1, 2> shape;
        bf<Type, 2, 1> specular;
        bf<Type, 2, 1> noise;
        bf<Type, 3, 1> environMap;
        bf<Type, 3, 1> dof;
        bf<Type, 4, 1> diffuseMap;
        bf<Type, 4, 1> bloom;
        bf<Type, 5, 1> normalMap;
        bf<Type, 5, 1> blendPrevious;
        bf<Type, 5, 1> movieAlpha;
        bf<Type, 6, 1> copyPrevious;
        bf<Type, 7, 1> glowMap;
        bf<Type, 7, 1> kaleidoscope;
        bf<Type, 8, 1> prelit;
        bf<Type, 8, 1> refract;
        bf<Type, 10, 2> texGen;
        bf<Type, 12, 1> skinned;
        bf<Type, 13, 1> screenAligned;
        bf<Type, 14, 1> rimlightUnder;
        bf<Type, 14, 4> numTaps;
        bf<Type, 14, 1> resamp;
        bf<Type, 15, 1> rimlightMap;
        bf<Type, 15, 1> chromaticAberration;
        bf<Type, 16, 1> hasLightsReal;
        bf<Type, 17, 1> hasLightsApprox;
        bf<Type, 18, 1> fog;
        bf<Type, 19, 1> shadowMap;
        bf<Type, 20, 1> anisotropic;
        bf<Type, 21, 1> colorXfm;
        bf<Type, 22, 1> hdr;
        bf<Type, 22, 2> hallOfTime;
        bf<Type, 23, 1> extrude;
        bf<Type, 24, 1> normDetail;
        bf<Type, 24, 1> motionBlur;
        bf<Type, 25, 1> billboard;
        bf<Type, 25, 1> gradientMap;
        bf<Type, 26, 2> fade;
        bf<Type, 28, 2> numLightProj;
        bf<Type, 30, 2> customVariation;
        bf<Type, 32, 2> colorMod;
        bf<Type, 34, 1> furDetail;
        bf<Type, 35, 1> displayError;
        bf<Type, 36, 1> vignette;
        bf<Type, 37, 1> rimlightEnabled;
        bf<Type, 37, 1> glare;
        bf<Type, 38, 1> ambientOcclusionOn;
        bf<Type, 39, 1> toneMapping;
        bf<Type, 40, 2> numLightPoint;
        bf<Type, 42, 1> velocity;
        bf<Type, 43, 1> environMapFalloff;
        bf<Type, 43, 1> chromaticSharpen;
        bf<Type, 44, 1> projLightMultiply;
        bf<Type, 45, 1> softDepthBlend;
        bf<Type, 46, 1> refractWorld;
        bf<Type, 47, 1> noiseMidtone;
        bf<Type, 48, 1> enablePointCubeTex;
        bf<Type, 49, 1> environMapSpecMask;
        bf<Type, 50, 1> showShaderCost;
        bf<Type, 51, 1> spotlight;
        bf<Type, 52, 1> hiResScreen;
        bf<Type, 53, 1> intensify;
        bf<Type, 54, 1> normalFlip;
        bf<Type, 55, 1> fitToSpline;
        bf<Type, 56, 1> splinePulse;
        bf<Type, 59, 1> syncTrackChargeEffect;
        bf<Type, 60, 1> shockwave;
        bf<Type, 61, 1> fastAndCheapLighting;
        bf<Type, 62, 1> hueConverge;
        Type value;
    };

    /** Generate ShaderMacros both from our flags and from the given ShaderType. */
    void GenerateMacros(ShaderType t, std::vector<ShaderMacro> &macros) const;
};

/** Initialize the internal shader symbols. */
void InitShaderOptions();
/** Given a ShaderType, get the corresponding name string. */
const char *ShaderTypeName(ShaderType shader);
/** Given a shader name string, get the corresponding ShaderType. */
ShaderType ShaderTypeFromName(const char *name);
const char *ShaderSourcePath(const char *file);
const char *ShaderCachedPath(const char *file, u64 flags, bool pixelShader);
bool IsPostProcShaderType(ShaderType shader);
/** Given a ShaderType, generate ShaderMacros for it,
    and print them all out to a string. */
void ShaderMakeOptionsString(ShaderType shader, const ShaderOptions &options, String &str);
