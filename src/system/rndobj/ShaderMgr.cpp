#include "rndobj/ShaderMgr.h"
#include "Shader.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/System.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"

void RndShaderMgr::Init() {
    PreInit();
    LoadShaders("%s_shaders");
}

void RndShaderMgr::UpdateCache(const Transform &xfm, int idx) {
    float *cacheIdx = &mConstantCache[idx];
    cacheIdx[0] = xfm.m.x.x;
    cacheIdx[1] = xfm.m.y.x;
    cacheIdx[2] = xfm.m.z.x;
    cacheIdx[3] = xfm.v.x;
    cacheIdx[4] = xfm.m.x.y;
    cacheIdx[5] = xfm.m.y.y;
    cacheIdx[6] = xfm.m.z.y;
    cacheIdx[7] = xfm.v.y;
    cacheIdx[8] = xfm.m.x.z;
    cacheIdx[9] = xfm.m.y.z;
    cacheIdx[10] = xfm.m.z.z;
    cacheIdx[11] = xfm.v.z;
}

void RndShaderMgr::SetMeshInfo(int i, bool b) {
    unk10 = i;
    unkc = b;
}

void RndShaderMgr::SetShaderErrorDisplay(bool disp) { mDisplayShaderError = disp; }
bool RndShaderMgr::GetShaderErrorDisplay() { return mDisplayShaderError; }

unsigned long RndShaderMgr::InitShaders() {
    if (UsingCD() || GetGfxMode() == kOldGfx)
        mCacheShaders = false;
    else {
        DataArray *cfg = SystemConfig("rnd", "cache_shaders");
        mCacheShaders = cfg->Int(1);
    }
    RndShader::Init();
    return 0;
}

void RndShaderMgr::LoadShaders(const char *cc) {
    unsigned long shaders = InitShaders();
    if (TheLoadMgr.GetPlatform() != kPlatformNone) {
        String str(MakeString(cc, PlatformSymbol(TheLoadMgr.GetPlatform())));
        FileStat stat;
        if (!mCacheShaders || !FileGetStat(str.c_str(), &stat)) {
            if (stat.st_mtime > shaders || strstr(cc, "preinit")) {
                FileStream stream(str.c_str(), FileStream::kRead, true);
                if (!stream.Fail()) {
                    LoadShaderFile(stream);
                } else {
                    if (UsingCD() && GetGfxMode() == kNewGfx) {
                        MILO_NOTIFY("Can't load shader file %s!!", str.c_str());
                    }
                }
            }
        }
    }
}

void RndShaderMgr::SetTransform(const Transform &xfm) {
    unk10 = 0;
    SetVConstant4x3((VShaderConstant)0x5c, Hmx::Matrix4(xfm));
}

// void __thiscall RndShaderMgr::SetTransform(RndShaderMgr *this,Transform *param_1)

// {
//   int iVar1;
//   undefined8 uVar2;
//   Matrix4 aMStack_60 [72];

//   iVar1 = *this;
//   *(this + 0x10) = 0;
//   uVar2 = Hmx::Matrix4::Matrix4(aMStack_60,param_1);
//   (**(iVar1 + 0x30))(this,0x5c,uVar2);
//   return;
// }

RndShaderMgr::RndShaderMgr()
    : mShaderPoolCount(0), unk5c(0), mConstantCache(0), unk68(0), unk6d(0), unk6e(1),
      unk6f(0) {}
