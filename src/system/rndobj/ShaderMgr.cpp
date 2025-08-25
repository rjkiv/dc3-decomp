#include "rndobj/ShaderMgr.h"
#include "Shader.h"
#include "macros.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/System.h"
#include "rndobj/ShaderProgram.h"
#include "rndobj/Utl.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

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

void RndShaderMgr::ShaderPoolAlloc(int i) { unk5c = i; }

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
    return RndShaderProgram::InitModTime();
}

void RndShaderMgr::PreInit() {
    if (!unk6d) {
        unkc = 0;
        unk6d = true;
        unk10 = 0;
        unk14 = 1;
        unk18 = 0;
        unk1c = 0;
        unk20 = 0;
        unk24 = 0;
        unk25 = 0;
        unk26 = 0;
        unk27 = 0;
        unk28 = 0;
        unk29 = 0;
        unk2b = 0;
        unk2c = 0;
        unk2d = 0;
        unk2e = 0;
        unk2f = 0;
        unk30 = 0;
        unk31 = 0;
        unk34 = 0;
        unk38 = 0;
        unk39 = 0;
        unk3a = 0;
        unk2a = 0;
        unk3b = 0;
        unk3c = 0;
        unk3d = 0;
        unk3e = 0;
        unk3f = 0;
        unk40 = 1;
        unk41 = 1;
        mDisplayShaderError = true;
        RELEASE(mWorkMat);
        RELEASE(mPostProcMat);
        RELEASE(mDrawHighlightMat);
        RELEASE(mDrawRectMat);
        mWorkMat = Hmx::Object::New<RndMat>();
        mPostProcMat = Hmx::Object::New<RndMat>();
        mDrawHighlightMat = Hmx::Object::New<RndMat>();
        mDrawRectMat = Hmx::Object::New<RndMat>();
        CreateAndSetMetaMat(mWorkMat);
        CreateAndSetMetaMat(mPostProcMat);
        CreateAndSetMetaMat(mDrawHighlightMat);
        CreateAndSetMetaMat(mDrawRectMat);
        MILO_ASSERT(mConstantCache == NULL, 104);
        unk68 = 0x204;
        MemPushTemp();
        mConstantCache = new float[unk68];
        MemPopTemp();
        LoadShaders("%s_preinit_shaders");
    }
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
