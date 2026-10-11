#include "ShaderOptions.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "rndobj/ShaderOptions.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

Symbol sShaderTypes[kMaxShaderTypes];

void InitShaderOptions() {
    sShaderTypes[kBloomShader] = "bloom";
    sShaderTypes[kBloomGlareShader] = "bloom_glare";
    sShaderTypes[kBlurShader] = "blur";
    sShaderTypes[kDepthVolumeShader] = "depthvolume";
    sShaderTypes[kDownsampleDepthShader] = "downsample_depth";
    sShaderTypes[kDownsampleShader] = "downsample";
    sShaderTypes[kDownsample4xShader] = "downsample_4x";
    sShaderTypes[kDrawRectShader] = "drawrect";
    sShaderTypes[kErrorShader] = "error";
    sShaderTypes[kFurShader] = "fur";
    sShaderTypes[kLineNozShader] = "line_noz";
    sShaderTypes[kLineShader] = "line";
    sShaderTypes[kMovieShader] = "movie";
    sShaderTypes[kMultimeshBBShader] = "multimesh_bb";
    sShaderTypes[kMultimeshShader] = "multimesh";
    sShaderTypes[kParticlesShader] = "particles";
    sShaderTypes[kPostprocessErrorShader] = "postproc_error";
    sShaderTypes[kPostprocessShader] = "postprocess";
    sShaderTypes[kShadowmapShader] = "shadowmap";
    sShaderTypes[kStandardShader] = "standard";
    sShaderTypes[kSyncTrackShader] = "sync_track";
    sShaderTypes[kUnwrapUVShader] = "unwrapuv";
    sShaderTypes[kVelocityCameraShader] = "velocity_camera";
    sShaderTypes[kVelocityObjectShader] = "velocity_object";
    sShaderTypes[kPlayerDepthVisShader] = "playerdepth_vis";
    sShaderTypes[kPlayerDepthShellShader] = "playerdepth_shell";
    sShaderTypes[kPlayerDepthShell2Shader] = "playerdepth_shell2";
    sShaderTypes[kDepthBuffer3DShader] = "depthbuffer_3d";
    sShaderTypes[kYUVtoRGBShader] = "yuv_to_rgb";
    sShaderTypes[kYUVtoBlackAndWhiteShader] = "yuv_to_black_and_white";
    sShaderTypes[kPlayerGreenScreenShader] = "player_greenscreen";
    sShaderTypes[kPlayerDepthGreenScreenShader] = "player_depthgreenscreen";
    sShaderTypes[kCrewPhotoShader] = "crew_photo";
    sShaderTypes[kTwirlShader] = "twirl";
    sShaderTypes[kKillAlphaShader] = "killalpha";
    sShaderTypes[kAllWhiteShader] = "allwhite";
}

const char *ShaderTypeName(ShaderType shader) {
    MILO_ASSERT(shader >= ShaderType(0) && shader < kMaxShaderTypes, 0x41);
    return sShaderTypes[shader].Str();
}

const char *sShaderErrors[] = { "pulse" };

ShaderType ShaderTypeFromName(const char *name) {
    for (int i = 0; i < kMaxShaderTypes; i++) {
        if (streq(name, sShaderTypes[i].Str())) {
            return (ShaderType)i;
        }
    }
    for (int i = 0; i < DIM(sShaderErrors); i++) {
        if (streq(name, sShaderErrors[i])) {
            return kErrorShader;
        }
    }
    MILO_FAIL("Shader type name %s not found\n", name);
    return (ShaderType)-1;
}

const char *ShaderSourcePath(const char *file) {
    if (file) {
        return FileMakePath(
            FileSystemRoot(), MakeString("shaders/%s.fx", FileGetBase(file))
        );
    } else {
        return FileMakePath(FileSystemRoot(), "shaders");
    }
}

const char *ShaderCachedPath(const char *file, u64 flags, bool pixelShader) {
    Platform plat = TheLoadMgr.GetPlatform();
    return MakeString(
        "%s/gen/%s_%llx_%s.%s_%s",
        FileGetPath(file),
        FileGetBase(file),
        flags,
        pixelShader ? "ps" : "vs",
        FileGetExt(file),
        PlatformSymbol(plat)
    );
}

bool IsPostProcShaderType(ShaderType s) {
    switch (s) {
    case kBloomShader:
    case kBlurShader:
    case kDepthVolumeShader:
    case kDownsampleShader:
    case kDownsample4xShader:
    case kDownsampleDepthShader:
    case kDrawRectShader:
    case kFurShader:
    case kLineNozShader:
    case kLineShader:
    case kMovieShader:
    case kPostprocessErrorShader:
    case kPostprocessShader:
    case kShadowmapShader:
    case kUnwrapUVShader:
    case kVelocityCameraShader:
    case kVelocityObjectShader:
    case kPlayerDepthVisShader:
    case kPlayerDepthShellShader:
    case kBloomGlareShader:
    case kPlayerDepthShell2Shader:
    case kDepthBuffer3DShader:
    case kYUVtoRGBShader:
    case kYUVtoBlackAndWhiteShader:
    case kPlayerGreenScreenShader:
    case kPlayerDepthGreenScreenShader:
    case kCrewPhotoShader:
    case kTwirlShader:
    case kKillAlphaShader:
        return false;
    case kErrorShader:
    case kMultimeshShader:
    case kMultimeshBBShader:
    case kParticlesShader:
    case kStandardShader:
    case kStandardBBShader:
    case kSyncTrackShader:
    case kSyncTrackChargeEffectShader:
    case kAllWhiteShader:
        return true;
    default:
        MILO_FAIL("unknown shader type %s", ShaderTypeName(s));
        return false;
    }
}

void ShaderOptions::GenerateMacros(ShaderType t, std::vector<ShaderMacro> &macros) const {
    static const char *str[] = { "0", "1",  "2",  "3",  "4",  "5",  "6",  "7", "8",
                                 "9", "10", "11", "12", "13", "14", "15", "16" };
    Platform plat = TheLoadMgr.GetPlatform();
    IsPostProcShaderType(t);
    macros.clear();
    macros.reserve(64);
    macros.push_back(ShaderMacro("PIXEL_SHADER", nullptr));
    if (plat == kPlatformXBox) {
        macros.push_back(ShaderMacro("HX_XBOX", "1"));
        macros.push_back(ShaderMacro("HX_WIN32", "1"));
        macros.push_back(ShaderMacro("SHOW_SHADER_COST", str[showShaderCost]));
    } else if (plat == kPlatformPS3) {
        macros.push_back(ShaderMacro("HX_PS3", "1"));
    } else {
        MILO_ASSERT(plat == kPlatformPC, 0x111);
        macros.push_back(ShaderMacro("HX_PC", "1"));
        macros.push_back(ShaderMacro("HX_WIN32", "1"));
    }
    macros.push_back(ShaderMacro("ENABLE_DIFFUSE_MAP", str[diffuseMap]));
    macros.push_back(ShaderMacro("ENABLE_NORMAL_MAP", str[normalMap]));
    macros.push_back(ShaderMacro("NORM_DETAIL", str[normDetail]));
    macros.push_back(ShaderMacro("FLIP_NORMAL", str[normalFlip]));
    macros.push_back(ShaderMacro("ENABLE_SPECULAR", str[specular]));
    macros.push_back(ShaderMacro("ENABLE_SPECULAR_MAP", str[specularMap]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT", str[rimlightEnabled]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT_UNDER", str[rimlightUnder]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT_MAP", str[rimlightMap]));
    macros.push_back(ShaderMacro("ENABLE_ENVIRON_MAP", str[environMap]));
    macros.push_back(ShaderMacro("ENABLE_ENVIRON_MAP_FALLOFF", str[environMapFalloff]));
    macros.push_back(ShaderMacro("ENABLE_ENVIRON_MAP_SPECMASK", str[environMapSpecMask]));
    macros.push_back(ShaderMacro("ENABLE_GLOW_MAP", str[glowMap]));
    macros.push_back(ShaderMacro("ENABLE_MOVIE_GRAYSCALE", str[movieGrayscale]));
    macros.push_back(ShaderMacro("ENABLE_MOVIE_ALPHA", str[movieAlpha]));
    macros.push_back(ShaderMacro("PER_PIXEL_LIGHTING", str[perPixel]));
    macros.push_back(ShaderMacro("PRELIT", str[prelit]));
    macros.push_back(ShaderMacro("TEX_GEN", str[texGen]));
    macros.push_back(ShaderMacro("HAS_REAL_LIGHTS", str[hasLightsReal]));
    macros.push_back(ShaderMacro("HAS_APPROX_LIGHTS", str[hasLightsApprox]));
    macros.push_back(ShaderMacro("NUM_PROJ", str[numLightProj]));
    macros.push_back(ShaderMacro("PROJ_LIGHT_MULTIPLY", str[projLightMultiply]));
    macros.push_back(ShaderMacro("NUM_POINT", str[numLightPoint]));
    macros.push_back(ShaderMacro("RESAMP", str[resamp]));
    macros.push_back(ShaderMacro("SKINNED", str[skinned]));
    macros.push_back(ShaderMacro("FOG", str[fog]));
    macros.push_back(ShaderMacro("SPOTLIGHT", str[spotlight]));
    macros.push_back(ShaderMacro("FADE_OUT", str[fade]));
    macros.push_back(ShaderMacro("SHADOW_BUFFER", str[shadowMap]));
    macros.push_back(ShaderMacro("ANISOTROPIC", str[anisotropic]));
    macros.push_back(ShaderMacro("PSEUDO_HDR", str[hdr]));
    macros.push_back(ShaderMacro("POSTERIZE", str[posterize]));
    macros.push_back(ShaderMacro("BILLBOARD", str[billboard]));
    macros.push_back(ShaderMacro("NUM_TAPS", str[numTaps]));
    macros.push_back(ShaderMacro("PARTICLES", t == kParticlesShader ? "1" : "0"));
    macros.push_back(ShaderMacro("SCREEN_ALIGNED", str[screenAligned]));
    macros.push_back(ShaderMacro("COLORXFM", str[colorXfm]));
    macros.push_back(ShaderMacro("HUECONVERGE", str[hueConverge]));
    macros.push_back(ShaderMacro("NOISE", str[noise]));
    macros.push_back(ShaderMacro("NOISE_MIDTONE", str[noiseMidtone]));
    macros.push_back(ShaderMacro("DOF", str[dof]));
    macros.push_back(ShaderMacro("BLENDPREVIOUS", str[blendPrevious]));
    macros.push_back(ShaderMacro("COPYPREVIOUS", str[copyPrevious]));
    macros.push_back(ShaderMacro("BLOOM", str[bloom]));
    macros.push_back(ShaderMacro("GLARE", str[glare]));
    macros.push_back(ShaderMacro("KALEIDOSCOPE", str[kaleidoscope]));
    macros.push_back(ShaderMacro("HALLOFTIME", str[hallOfTime]));
    macros.push_back(ShaderMacro("MOTIONBLUR", str[motionBlur]));
    macros.push_back(ShaderMacro("VELOCITY", str[velocity]));
    macros.push_back(ShaderMacro("GRADIENTMAP", str[gradientMap]));
    macros.push_back(ShaderMacro("REFRACT", str[refract]));
    macros.push_back(ShaderMacro("REFRACT_WORLD", str[refractWorld]));
    macros.push_back(ShaderMacro("EXTRUDE", str[extrude]));
    macros.push_back(ShaderMacro("SHAPE", str[shape]));
    macros.push_back(ShaderMacro("CUSTOM_VARIATION", str[customVariation]));
    macros.push_back(ShaderMacro("CHROMATIC_ABERRATION", str[chromaticAberration]));
    macros.push_back(ShaderMacro("CHROMATIC_SHARPEN", str[chromaticSharpen]));
    macros.push_back(ShaderMacro("COLOR_MOD", str[colorMod]));
    macros.push_back(ShaderMacro("FUR_DETAIL", str[furDetail]));
    macros.push_back(ShaderMacro("DISPLAY_ERROR", str[displayError]));
    macros.push_back(ShaderMacro("VIGNETTE", str[vignette]));
    macros.push_back(ShaderMacro("ENABLE_AO", str[ambientOcclusionOn]));
    macros.push_back(ShaderMacro("TONE_MAPPING", str[toneMapping]));
    macros.push_back(ShaderMacro("SOFT_DEPTH_BLEND", str[softDepthBlend]));
    macros.push_back(ShaderMacro("ENABLE_POINT_CUBE_TEX", str[enablePointCubeTex]));
    macros.push_back(ShaderMacro("HI_RES_SCREEN", str[hiResScreen]));
    macros.push_back(ShaderMacro("INTENSIFY", str[intensify]));
    macros.push_back(ShaderMacro("FIT_TO_SPLINE", str[fitToSpline]));
    macros.push_back(ShaderMacro("SPLINE_PULSE", str[splinePulse]));
    macros.push_back(ShaderMacro("SYNC_TRACK_CHARGE_EFFECT", str[syncTrackChargeEffect]));
    macros.push_back(ShaderMacro("SHOCKWAVE", str[shockwave]));
    macros.push_back(ShaderMacro("FAST_CHEAP_LIGHTING", str[fastAndCheapLighting]));
    macros.push_back(ShaderMacro(nullptr, nullptr));
}

void ShaderMakeOptionsString(ShaderType type, const ShaderOptions &opts, String &str) {
    std::vector<ShaderMacro> macros;
    opts.GenerateMacros(type, macros);
    bool first = true;
    for (int i = 0; i < macros.size(); i++) {
        ShaderMacro &cur = macros[i];
        if (cur.Name && cur.Value && !streq(cur.Value, "0")) {
            if (!first) {
                str += " ";
            }
            first = false;
            str += cur.Name;
            str += "=";
            str += cur.Value;
        }
    }
}
