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
    : mShaderPoolCount(0), mPendingShaderPoolCount(0), mConstantCache(0), mCacheSize(0),
      mPreInitialized(0), mShowShaderErrors(1), mShowMetaMatErrors(0) {}

void RndShaderMgr::PreInit() {
    if (!mPreInitialized) {
        mUseAO = false;
        mPreInitialized = true;
        mNumBones = 0;
        mNumTaps = 1;
        mDepthVolume = 0;
        mDepthShape = 0;
        mCull = kUnForced;
        mSubtract = 0;
        mPostSpotlight = 0;
        mPostDOF = 0;
        mPostBloom = 0;
        mPostGlare = 0;
        mPostColorXfm = 0;
        mPostPosterize = 0;
        mPostKaleidoscope = 0;
        mPostNoise = 0;
        mPostNoiseMidtone = 0;
        mPostBlendPrevious = 0;
        mPostCopyPrevious = 0;
        mPostResamp = 0;
        mPostHallOfTime = 0;
        mPostMotionBlur = false;
        mPostVelocity = 0;
        mPostGradientMap = false;
        mPostHueConverge = 0;
        mPostRefract = 0;
        mPostChromaticAberration = 0;
        mPostChromaticSharpen = 0;
        mPostVignette = 0;
        mPostSoftDepth = 0;
        mAllowPerPixel = 1;
        mPrecacheOffscreen = 1;
        mShaderErrorDisplay = true;
        RELEASE(mWork);
        RELEASE(mPostProc);
        RELEASE(mHighlight);
        RELEASE(mDrawRect);
        mWork = Hmx::Object::New<RndMat>();
        mPostProc = Hmx::Object::New<RndMat>();
        mHighlight = Hmx::Object::New<RndMat>();
        mDrawRect = Hmx::Object::New<RndMat>();
        CreateAndSetMetaMat(mWork);
        CreateAndSetMetaMat(mPostProc);
        CreateAndSetMetaMat(mHighlight);
        CreateAndSetMetaMat(mDrawRect);
        MILO_ASSERT(mConstantCache == NULL, 104);
        mCacheSize = 516;
        {
            MemDoTempAllocations tmp;
            mConstantCache = new float[mCacheSize];
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
    mCacheSize = 0;
}

void RndShaderMgr::UpdateCache(const Transform &xfm, int idx) {
    const Vector3 x(xfm.m.x.x, xfm.m.y.x, xfm.m.z.x);
    const Vector3 y(xfm.m.x.y, xfm.m.y.y, xfm.m.z.y);
    const Vector3 z(xfm.m.x.z, xfm.m.y.z, xfm.m.z.z);
    const Vector3 v(xfm.v.x, xfm.v.y, xfm.v.z);

    mConstantCache[idx * 12 + 0] = x.x;
    mConstantCache[idx * 12 + 1] = x.y;
    mConstantCache[idx * 12 + 2] = x.z;
    mConstantCache[idx * 12 + 3] = v.x;
    mConstantCache[idx * 12 + 4] = y.x;
    mConstantCache[idx * 12 + 5] = y.y;
    mConstantCache[idx * 12 + 6] = y.z;
    mConstantCache[idx * 12 + 7] = v.y;
    mConstantCache[idx * 12 + 8] = z.x;
    mConstantCache[idx * 12 + 9] = z.y;
    mConstantCache[idx * 12 + 10] = z.z;
    mConstantCache[idx * 12 + 11] = v.z;
}

void RndShaderMgr::ShaderPoolAlloc(int i) { mPendingShaderPoolCount = i; }

void RndShaderMgr::SetMeshInfo(int i, bool b) {
    mNumBones = i;
    mUseAO = b;
}

void RndShaderMgr::SetShaderErrorDisplay(bool disp) { mShaderErrorDisplay = disp; }
bool RndShaderMgr::GetShaderErrorDisplay() { return mShaderErrorDisplay; }

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
    Platform plat = TheLoadMgr.GetPlatform();
    if (plat != kPlatformNone) {
        String str(MakeString(filename, PlatformSymbol(plat)));
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
        mPendingShaderPoolCount = alloc;
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
    if (mShaderPoolCount == 0 && mPendingShaderPoolCount > 0) {
        mShaderPoolCount = mPendingShaderPoolCount;
        mPendingShaderPoolCount = 0;
        mShaderPool =
            MemAlloc(mShaderSize * mShaderPoolCount, __FILE__, 0x11c, "ShaderPool");
    }
    if (mShaderPoolCount <= 0) {
        if (UsingCD()) {
            MILO_NOTIFY_ONCE("Shader Pool is allocating dynamically");
        }
        mPendingShaderPoolCount = 0;
        mShaderPoolCount = 0x100;
        mShaderPool = MemAlloc(mShaderSize << 8, __FILE__, 0x127, "ShaderPool");
    }
    MILO_ASSERT(mShaderPoolCount-- > 0, 0x12A);
    // increment mShaderPool by mShaderSize
    void *old = mShaderPool;
    char *pool = (char *)mShaderPool;
    pool += mShaderSize;
    mShaderPool = pool;
    mPendingShaderPoolCount--;
    return old;
}

RndShaderProgram &RndShaderMgr::FindShader(ShaderType t, const ShaderOptions &opts) {
    u64 flags = opts.value;
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
