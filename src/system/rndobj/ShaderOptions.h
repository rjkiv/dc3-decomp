#pragma once
#include "types.h"
#include "utl/Str.h"
#include <vector>

enum ShaderType {
    kBloomShader = 0,
    kBlurShader = 1,
    kDepthVolumeShader = 2,
    kDownsampleShader = 3,
    kDownsample4xShader = 4,
    kDownsampleDepthShader = 5,
    kDrawRectShader = 6,
    kErrorShader = 7,
    kFurShader = 8,
    kLineNozShader = 9,
    kLineShader = 10,
    kMovieShader = 11,
    kMultimeshShader = 12,
    kMultimeshBBShader = 13,
    kParticlesShader = 14,
    kPostprocessErrorShader = 15,
    kPostprocessShader = 16,
    kShadowmapShader = 17,
    kStandardShader = 18,
    kStandardBBShader = 19,
    kSyncTrackShader = 20,
    kSyncTrackChargeEffectShader = 21,
    kUnwrapUVShader = 22,
    kVelocityCameraShader = 23,
    kVelocityObjectShader = 24,
    kPlayerDepthVisShader = 25,
    kPlayerDepthShellShader = 26,
    kBloomGlareShader = 27,
    kPlayerDepthShell2Shader = 28,
    kDepthBuffer3DShader = 29,
    kYUVtoRGBShader = 30,
    kYUVtoBlackAndWhiteShader = 31,
    kPlayerGreenScreenShader = 32,
    kPlayerDepthGreenScreenShader = 33,
    kCrewPhotoShader = 34,
    kTwirlShader = 35,
    kKillAlphaShader = 36,
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
    ShaderOptions(u64 u) : flags(u) {}

    /** Generate ShaderMacros both from our flags and from the given ShaderType. */
    void GenerateMacros(ShaderType t, std::vector<ShaderMacro> &macros) const;

    u64 flags; // 0x0

    // Bits and (supposedly) what they represent:
    // 0: PER_PIXEL_LIGHTING, ENABLE_POINT_CUBE_TEX, SPLINE_PULSE
    // 1-2: SHAPE
    // 1: ENABLE_SPECULAR_MAP, ENABLE_MOVIE_GRAYSCALE, POSTERIZE
    // 2: ENABLE_SPECULAR, NOISE
    // 3: ENABLE_ENVIRON_MAP, DOF
    // 4: ENABLE_DIFFUSE_MAP, BLOOM
    // 5: ENABLE_NORMAL_MAP, ENABLE_MOVIE_ALPHA, BLENDPREVIOUS
    // 6: COPYPREVIOUS
    // 7: ENABLE_GLOW_MAP, KALEIDOSCOPE
    // 8: PRELIT, REFRACT
    // 10-11: TEX_GEN
    // 12: SKINNED
    // 13: SCREEN_ALIGNED
    // 14-17: NUM_TAPS
    // 14: ENABLE_RIMLIGHT_UNDER, RESAMP
    // 15: ENABLE_RIMLIGHT_MAP, CHROMATIC_ABERRATION
    // 16: HAS_REAL_LIGHTS
    // 17: HAS_APPROX_LIGHTS
    // 18: FOG
    // 19: SHADOW_BUFFER
    // 20: ANISOTROPIC
    // 21: COLORXFM
    // 22-23: HALLOFTIME
    // 22: PSEUDO_HDR
    // 23: EXTRUDE
    // 24: NORM_DETAIL, MOTIONBLUR
    // 25: BILLBOARD, GRADIENTMAP
    // 26-27: FADE_OUT
    // 28-29: NUM_PROJ
    // 30-31: CUSTOM_VARIATION
    // 32-33: COLOR_MOD
    // 34: FUR_DETAIL
    // 35: DISPLAY_ERROR
    // 36: VIGNETTE
    // 37: ENABLE_RIMLIGHT, GLARE
    // 38: ENABLE_AO
    // 39: TONE_MAPPING
    // 40-41: NUM_POINT
    // 42: VELOCITY
    // 43: ENABLE_ENVIRON_MAP_FALLOFF, CHROMATIC_SHARPEN
    // 44: PROJ_LIGHT_MULTIPLY
    // 45: SOFT_DEPTH_BLEND
    // 46: REFRACT_WORLD
    // 47: NOISE_MIDTONE
    // 49: ENABLE_ENVIRON_MAP_SPECMASK
    // 50: SHOW_SHADER_COST
    // 51: SPOTLIGHT
    // 52: HI_RES_SCREEN
    // 53: INTENSIFY
    // 54: FLIP_NORMAL
    // 55: FIT_TO_SPLINE
    // 59: SYNC_TRACK_CHARGE_EFFECT
    // 60: SHOCKWAVE
    // 61: FAST_CHEAP_LIGHTING
    // 62: HUECONVERGE
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
