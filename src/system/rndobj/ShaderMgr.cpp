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
        mHasAOCalc = 0;
        unk6d = true;
        mNumBones = 0;
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
    // i put this here because the asm indexes by increments of 0x30
    struct ShaderCache {
        float xx, yx, zx, vx;
        float xy, yy, zy, vy;
        float xz, yz, zz, vz;
    };
    ShaderCache *cacheArr = (ShaderCache *)mConstantCache;
    ShaderCache *cacheIdx = &cacheArr[idx];

    // yeah i hate this too don't you worry
    float vx = xfm.v.x;
    float xx = xfm.m.x.x;
    float yx = xfm.m.y.x;
    float zx = xfm.m.z.x;
    float vy = xfm.v.y;
    float xy = xfm.m.x.y;
    float yy = xfm.m.y.y;
    float zy = xfm.m.z.y;
    float vz = xfm.v.z;
    float xz = xfm.m.x.z;
    float yz = xfm.m.y.z;
    float zz = xfm.m.z.z;

    cacheIdx->xx = xx;
    cacheIdx->yx = yx;
    cacheIdx->zx = zx;
    cacheIdx->vx = vx;
    cacheIdx->xy = xy;
    cacheIdx->yy = yy;
    cacheIdx->zy = zy;
    cacheIdx->vy = vy;
    cacheIdx->xz = xz;
    cacheIdx->yz = yz;
    cacheIdx->zz = zz;
    cacheIdx->vz = vz;
}

void RndShaderMgr::ShaderPoolAlloc(int i) { unk5c = i; }

void RndShaderMgr::SetMeshInfo(int i, bool b) {
    mNumBones = i;
    mHasAOCalc = b;
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
    mNumBones = 0;
    SetVConstant4x3((VShaderConstant)0x5c, Hmx::Matrix4(xfm));
}

void RndShaderMgr::Invalidate(ShaderType t) {
    bool invalid = t == kMaxShaderTypes;
    for (std::list<ShaderTree>::iterator it = mShaderTrees.begin();
         it != mShaderTrees.end();) {
        if (!invalid && it->shaderType != t) {
            ++it;
        } else {
            delete it->tree;
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
        int alloc; // prolly not the best var name
        fs >> alloc;
        unk5c = alloc;
        while (alloc--) {
            u64 shaderFlags;
            fs >> shaderFlags;
            RndShaderProgram &program =
                FindShader(shaderType, ShaderOptions(shaderFlags));
            int bufferSize;
            fs >> bufferSize;
            RndShaderBuffer *vertexBuffer;
            program.LoadShaderBuffer(fs, bufferSize, vertexBuffer);
            fs >> bufferSize;
            RndShaderBuffer *pixelBuffer;
            program.LoadShaderBuffer(fs, bufferSize, pixelBuffer);
            program.Cache(
                shaderType, ShaderOptions(shaderFlags), vertexBuffer, pixelBuffer
            );
            delete vertexBuffer;
            delete pixelBuffer;
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
    u64 flags = opts.flags;
    FOREACH (it, mShaderTrees) {
        // we found the shader, traverse through its tree
        if (it->shaderType == t) {
            RndShaderProgram *p = it->tree;
            while (true) {
                if (flags < p->mFlags) {
                    if (p->mLeft) {
                        p = p->mLeft;
                    } else {
                        RndShaderProgram *ret = NewShaderProgram();
                        p->mLeft = ret;
                        ret->mFlags = flags;
                        return *ret;
                    }
                } else if (flags > p->mFlags) {
                    if (p->mRight) {
                        p = p->mRight;
                    } else {
                        RndShaderProgram *ret = NewShaderProgram();
                        p->mRight = ret;
                        ret->mFlags = flags;
                        return *ret;
                    }
                } else {
                    return *p;
                }
            }
        }
    }
    // we did not find the shader, create a tree entry for it
    ShaderTree tree;
    tree.shaderType = t;
    RndShaderProgram *p = NewShaderProgram();
    p->mFlags = flags;
    tree.tree = p;
    // we wanna prioritize standard shaders
    if (t == kStandardShader) {
        mShaderTrees.push_front(tree);
    } else {
        mShaderTrees.push_back(tree);
    }
    return *p;
}
