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

    void GenerateMacros(ShaderType, std::vector<ShaderMacro> &) const;

    u64 flags; // 0x0
};

void InitShaderOptions();
const char *ShaderTypeName(ShaderType shader);
ShaderType ShaderTypeFromName(const char *name);
const char *ShaderSourcePath(const char *file);
const char *ShaderCachedPath(const char *file, u64 flags, bool pixelShader);
bool IsPostProcShaderType(ShaderType shader);
void ShaderMakeOptionsString(ShaderType shader, const ShaderOptions &options, String &str);
