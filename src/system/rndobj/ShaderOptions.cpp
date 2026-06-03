#include "ShaderOptions.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "rndobj/ShaderOptions.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Loader.h"
#include "utl/Std.h"
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
    static const char *sNumbers[] = { "0", "1",  "2",  "3",  "4",  "5",  "6",  "7", "8",
                                      "9", "10", "11", "12", "13", "14", "15", "16" };
    Platform plat = TheLoadMgr.GetPlatform();
    IsPostProcShaderType(t);
    macros.clear();
    macros.reserve(64);
    macros.push_back(ShaderMacro("PIXEL_SHADER", nullptr));
    if (plat == kPlatformXBox) {
        macros.push_back(ShaderMacro("HX_XBOX", "1"));
        macros.push_back(ShaderMacro("HX_WIN32", "1"));
        macros.push_back(ShaderMacro("SHOW_SHADER_COST", sNumbers[(flags >> 50) & 1]));
    } else if (plat == kPlatformPS3) {
        macros.push_back(ShaderMacro("HX_PS3", "1"));
    } else {
        MILO_ASSERT(plat == kPlatformPC, 0x111);
        macros.push_back(ShaderMacro("HX_PC", "1"));
        macros.push_back(ShaderMacro("HX_WIN32", "1"));
    }
    macros.push_back(ShaderMacro("ENABLE_DIFFUSE_MAP", sNumbers[(flags >> 4) & 1]));
    macros.push_back(ShaderMacro("ENABLE_NORMAL_MAP", sNumbers[(flags >> 5) & 1]));
    macros.push_back(ShaderMacro("NORM_DETAIL", sNumbers[(flags >> 24) & 1]));
    macros.push_back(ShaderMacro("FLIP_NORMAL", sNumbers[(flags >> 54) & 1]));
    macros.push_back(ShaderMacro("ENABLE_SPECULAR", sNumbers[(flags >> 2) & 1]));
    macros.push_back(ShaderMacro("ENABLE_SPECULAR_MAP", sNumbers[(flags >> 1) & 1]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT", sNumbers[(flags >> 37) & 1]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT_UNDER", sNumbers[(flags >> 14) & 1]));
    macros.push_back(ShaderMacro("ENABLE_RIMLIGHT_MAP", sNumbers[(flags >> 15) & 1]));
    macros.push_back(ShaderMacro("ENABLE_ENVIRON_MAP", sNumbers[(flags >> 3) & 1]));
    macros.push_back(
        ShaderMacro("ENABLE_ENVIRON_MAP_FALLOFF", sNumbers[(flags >> 43) & 1])
    );
    macros.push_back(
        ShaderMacro("ENABLE_ENVIRON_MAP_SPECMASK", sNumbers[(flags >> 49) & 1])
    );
    macros.push_back(ShaderMacro("ENABLE_GLOW_MAP", sNumbers[(flags >> 7) & 1]));
    macros.push_back(ShaderMacro("ENABLE_MOVIE_GRAYSCALE", sNumbers[(flags >> 1) & 1]));
    macros.push_back(ShaderMacro("ENABLE_MOVIE_ALPHA", sNumbers[(flags >> 5) & 1]));
    macros.push_back(ShaderMacro("PER_PIXEL_LIGHTING", sNumbers[flags & 1]));
    macros.push_back(ShaderMacro("PRELIT", sNumbers[(flags >> 8) & 1]));
    macros.push_back(ShaderMacro("TEX_GEN", sNumbers[(flags >> 10) & 3]));
    macros.push_back(ShaderMacro("HAS_REAL_LIGHTS", sNumbers[(flags >> 16) & 1]));
    macros.push_back(ShaderMacro("HAS_APPROX_LIGHTS", sNumbers[(flags >> 17) & 1]));
    macros.push_back(ShaderMacro("NUM_PROJ", sNumbers[(flags >> 28) & 3]));
    macros.push_back(ShaderMacro("PROJ_LIGHT_MULTIPLY", sNumbers[(flags >> 44) & 1]));
    macros.push_back(ShaderMacro("NUM_POINT", sNumbers[(flags >> 40) & 3]));
    macros.push_back(ShaderMacro("RESAMP", sNumbers[(flags >> 14) & 1]));
    macros.push_back(ShaderMacro("SKINNED", sNumbers[(flags >> 12) & 1]));
    macros.push_back(ShaderMacro("FOG", sNumbers[(flags >> 18) & 1]));
    macros.push_back(ShaderMacro("SPOTLIGHT", sNumbers[(flags >> 51) & 1]));
    macros.push_back(ShaderMacro("FADE_OUT", sNumbers[(flags >> 26) & 3]));
    macros.push_back(ShaderMacro("SHADOW_BUFFER", sNumbers[(flags >> 19) & 1]));
    macros.push_back(ShaderMacro("ANISOTROPIC", sNumbers[(flags >> 20) & 1]));
    macros.push_back(ShaderMacro("PSEUDO_HDR", sNumbers[(flags >> 22) & 1]));
    macros.push_back(ShaderMacro("POSTERIZE", sNumbers[(flags >> 1) & 1]));
    macros.push_back(ShaderMacro("BILLBOARD", sNumbers[(flags >> 25) & 1]));
    macros.push_back(ShaderMacro("NUM_TAPS", sNumbers[(flags >> 14) & 15]));
    macros.push_back(ShaderMacro("PARTICLES", t == kParticlesShader ? "1" : "0"));
    macros.push_back(ShaderMacro("SCREEN_ALIGNED", sNumbers[(flags >> 13) & 1]));
    macros.push_back(ShaderMacro("COLORXFM", sNumbers[(flags >> 21) & 1]));
    macros.push_back(ShaderMacro("HUECONVERGE", sNumbers[(flags >> 62) & 1]));
    macros.push_back(ShaderMacro("NOISE", sNumbers[(flags >> 2) & 1]));
    macros.push_back(ShaderMacro("NOISE_MIDTONE", sNumbers[(flags >> 47) & 1]));
    macros.push_back(ShaderMacro("DOF", sNumbers[(flags >> 3) & 1]));
    macros.push_back(ShaderMacro("BLENDPREVIOUS", sNumbers[(flags >> 5) & 1]));
    macros.push_back(ShaderMacro("COPYPREVIOUS", sNumbers[(flags >> 6) & 1]));
    macros.push_back(ShaderMacro("BLOOM", sNumbers[(flags >> 4) & 1]));
    macros.push_back(ShaderMacro("GLARE", sNumbers[(flags >> 37) & 1]));
    macros.push_back(ShaderMacro("KALEIDOSCOPE", sNumbers[(flags >> 7) & 1]));
    macros.push_back(ShaderMacro("HALLOFTIME", sNumbers[(flags >> 22) & 3]));
    macros.push_back(ShaderMacro("MOTIONBLUR", sNumbers[(flags >> 24) & 1]));
    macros.push_back(ShaderMacro("VELOCITY", sNumbers[(flags >> 42) & 1]));
    macros.push_back(ShaderMacro("GRADIENTMAP", sNumbers[(flags >> 25) & 1]));
    macros.push_back(ShaderMacro("REFRACT", sNumbers[(flags >> 8) & 1]));
    macros.push_back(ShaderMacro("REFRACT_WORLD", sNumbers[(flags >> 46) & 1]));
    macros.push_back(ShaderMacro("EXTRUDE", sNumbers[(flags >> 23) & 1]));
    macros.push_back(ShaderMacro("SHAPE", sNumbers[(flags >> 1) & 3]));
    macros.push_back(ShaderMacro("CUSTOM_VARIATION", sNumbers[(flags >> 30) & 3]));
    macros.push_back(ShaderMacro("CHROMATIC_ABERRATION", sNumbers[(flags >> 15) & 1]));
    macros.push_back(ShaderMacro("CHROMATIC_SHARPEN", sNumbers[(flags >> 43) & 1]));
    macros.push_back(ShaderMacro("COLOR_MOD", sNumbers[(flags >> 32) & 3]));
    macros.push_back(ShaderMacro("FUR_DETAIL", sNumbers[(flags >> 34) & 1]));
    macros.push_back(ShaderMacro("DISPLAY_ERROR", sNumbers[(flags >> 35) & 1]));
    macros.push_back(ShaderMacro("VIGNETTE", sNumbers[(flags >> 36) & 1]));
    macros.push_back(ShaderMacro("ENABLE_AO", sNumbers[(flags >> 38) & 1]));
    macros.push_back(ShaderMacro("TONE_MAPPING", sNumbers[(flags >> 39) & 1]));
    macros.push_back(ShaderMacro("SOFT_DEPTH_BLEND", sNumbers[(flags >> 45) & 1]));
    macros.push_back(ShaderMacro(
        "ENABLE_POINT_CUBE_TEX", sNumbers[*(const unsigned short *)&flags & 1]
    ));
    macros.push_back(ShaderMacro("HI_RES_SCREEN", sNumbers[(flags >> 52) & 1]));
    macros.push_back(ShaderMacro("INTENSIFY", sNumbers[(flags >> 53) & 1]));
    macros.push_back(ShaderMacro("FIT_TO_SPLINE", sNumbers[(flags >> 55) & 1]));
    macros.push_back(
        ShaderMacro("SPLINE_PULSE", sNumbers[*(const unsigned char *)&flags & 1])
    );
    macros.push_back(ShaderMacro("SYNC_TRACK_CHARGE_EFFECT", sNumbers[(flags >> 59) & 1]));
    macros.push_back(ShaderMacro("SHOCKWAVE", sNumbers[(flags >> 60) & 1]));
    macros.push_back(ShaderMacro("FAST_CHEAP_LIGHTING", sNumbers[(flags >> 61) & 1]));
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
