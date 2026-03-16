#pragma once
#include "ShaderOptions.h"
#include "rndobj/Rnd.h"
#include "types.h"
#include "os/Debug.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Mat.h"
#include "rndobj/Mat_NG.h"

class RndShader {
public:
    enum MatFlagErrorType {
    };

    virtual ~RndShader() {}
    virtual bool CheckError(MatFlagErrorType) { return false; }

    static void Init();
    static void SelectConfig(RndMat *, ShaderType, bool);

protected:
    virtual void Select(RndMat *, ShaderType shader_type, bool) = 0;
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool) = 0;

    void CheckForceCull(ShaderType);
    void Cache(ShaderType, ShaderOptions, RndMat *);
    bool RedundantState(const RndMat *, ShaderType, bool, bool, bool);

    static void ShaderWarn(const char *);
    static void WarnMatProp(const char *, NgMat *, class NgEnviron *, ShaderType);
    static bool MatShaderFlagsOK(RndMat *, ShaderType);
    static bool DisplayMatShaderFlagsError(RndMat *, ShaderType);

    static bool sCurrentSkinned;
    static bool sCurrentUseAO;
    static bool sMatShadersOK;
    static ModalCallbackFunc *mModalCallback;
    static ShaderType sCurrentShader;
    static RndShader *sShaders[kMaxShaderTypes];
};

class RndShaderSimple : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderDepthVolume : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderDrawRect : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderParticles : public RndShader {
public:
    virtual bool CheckError(MatFlagErrorType e) {
        return (e == 0 || e == 2) && TheRnd.DrawMode() != 4;
    }

protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderMultimesh : public RndShader {
public:
    virtual bool CheckError(MatFlagErrorType e) { return e == 0 || e == 1 || e == 2; }

protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderStandard : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderPostProc : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderUnwrapUV : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderVelocity : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderVelocityCamera : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderFur : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};

class RndShaderSyncTrack : public RndShader {
protected:
    virtual void Select(RndMat *, ShaderType, bool);
    virtual u64 CalcShaderOpts(NgMat *, ShaderType, bool);
};
