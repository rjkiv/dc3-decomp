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
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShaderProgram.h"
#include "rndobj/Utl.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

RndShaderMgr::RndShaderMgr()
    : mShaderPoolCount(0), unk5c(0), mConstantCache(0), unk68(0), unk6d(0),
      mShowShaderErrors(1), mShowMetaMatErrors(0) {}

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
        mAllowPerPixel = 1;
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
        unk68 = 516;
        {
            MemDoTempAllocations tmp;
            mConstantCache = new float[unk68];
        }
        LoadShaders("%s_preinit_shaders");
    }
}

void RndShaderMgr::Init() {
    PreInit();
    LoadShaders("%s_shaders");
}

void RndShaderMgr::Terminate() {
    Invalidate(kMaxShaderTypes);
    RELEASE(mConstantCache);
    unk68 = 0;
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

void RndShaderMgr::LoadShaders(const char *filename) {
    unsigned long shaders = InitShaders();
    if (TheLoadMgr.GetPlatform() != kPlatformNone) {
        String str(MakeString(filename, PlatformSymbol(TheLoadMgr.GetPlatform())));
        FileStat stat;
        if (!mCacheShaders
            || (!FileGetStat(str.c_str(), &stat) && stat.st_mtime > shaders)
            || strstr(filename, "preinit")) {
            FileStream stream(str.c_str(), FileStream::kRead, true);
            if (!stream.Fail()) {
                if (TheLoadMgr.GetPlatform() == kPlatformXBox) {
                    LoadShaderFile(stream);
                } else {
                    LoadShaderFile(stream);
                }
            } else {
                if (UsingCD() && GetGfxMode() == kNewGfx) {
                    MILO_NOTIFY("Can't load shader file %s!!", str.c_str());
                }
            }
        }
    }
}

void RndShaderMgr::SetTransform(const Transform &xfm) {
    unk10 = 0;
    SetVConstant4x3((VShaderConstant)0x5c, Hmx::Matrix4(xfm));
}

void RndShaderMgr::Invalidate(ShaderType t) {
    for (std::list<ShaderTree>::iterator it = mShaderTrees.begin();
         it != mShaderTrees.end();) {
        if (it == mShaderTrees.begin() && it->shaderType != t) {
            ++it;
        } else {
            delete it->obj;
            it = mShaderTrees.erase(it);
        }
    }
    RndShaderProgram::InitModTime();
}

void RndShaderMgr::LoadShaderFile(FileStream &fs) {
    if (TheLoadMgr.GetPlatform() == kPlatformPS3) {
        RndSplasherResume();
        unsigned int fileType, fileVersion;
        fs >> fileType;
        fs >> fileVersion;
        MILO_ASSERT(fileType == PS3_SHADERS_TYPE, 0xBF);
        MILO_ASSERT(fileVersion == PS3_SHADERS_VERSION, 0xC0);
        RndSplasherSuspend();
    }
    int num;
    fs >> num;
    while (num--) {
        Symbol name;
        fs >> name;
        ShaderType shaderType = ShaderTypeFromName(name.Str());
        int alloc;
        fs >> alloc;
        unk5c = alloc;
        while (alloc--) {
            u64 u50;
            fs >> u50;
            RndShaderProgram &program = FindShader(shaderType, ShaderOptions(u50));
            int i6c;
            fs >> i6c;
            RndShaderBuffer *buf1;
            program.LoadShaderBuffer(fs, i6c, buf1);
            fs >> i6c;
            RndShaderBuffer *buf2;
            program.LoadShaderBuffer(fs, i6c, buf2);
            program.Cache(shaderType, ShaderOptions(u50), buf1, buf2);
            delete buf1;
            delete buf2;
            RndSplasherPoll();
        }
    }
}

void *RndShaderMgr::AllocShader() {
    if (mShaderPoolCount == 0 && unk5c > 0) {
        mShaderPoolCount = unk5c;
        unk5c = 0;
        mShaderPool = MemAlloc(unk60 * mShaderPoolCount, __FILE__, 0x11c, "ShaderPool");
    }
    if (mShaderPoolCount <= 0) {
        if (UsingCD()) {
            MILO_NOTIFY_ONCE("Shader Pool is allocating dynamically");
        }
        unk5c = 0;
        mShaderPoolCount = 0x100;
        mShaderPool = MemAlloc(unk60 << 8, __FILE__, 0x127, "ShaderPool");
    }
    MILO_ASSERT(mShaderPoolCount-- > 0, 0x12A);
    // increment mShaderPool by unk60
    void *old = mShaderPool;
    char *pool = (char *)mShaderPool;
    pool += unk60;
    mShaderPool = pool;
    unk5c--;
    return old;
}

RndShaderProgram &RndShaderMgr::FindShader(ShaderType t, const ShaderOptions &opts) {
    for (std::list<ShaderTree>::iterator it = mShaderTrees.begin();
         it != mShaderTrees.end() && it->shaderType != t;
         ++it) {
        // more stuff here
    }
    ShaderTree tree;
    tree.shaderType = t;
    RndShaderProgram *p = NewShaderProgram();
    p->unk8 = opts.flags;
    tree.obj = p;
    if (t == kStandardShader) {
        mShaderTrees.push_front(tree);
    } else {
        mShaderTrees.push_back(tree);
    }
    return *p;
}
