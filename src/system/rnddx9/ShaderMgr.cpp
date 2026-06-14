
#include "ShaderMgr.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "os/Memory.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Rnd.h"
#include "rnddx9/Shader.h"
#include "rnddx9/ShaderInclude.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/CubeTex.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShaderProgram.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "utl/FileStream.h"
#include "utl/MemTrack.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/XGRAPHICS.h"
#include "xdk/d3dx9/d3dx9mesh.h"
#include "xdk/d3dx9/d3dx9shader.h"
#include "xdk/win_types.h"
#include "xdk/xgraphics/xgraphics.h"

DxShaderMgr TheDxShaderMgr;
RndShaderMgr &TheShaderMgr = TheDxShaderMgr;
DxShaderInclude &TheDxShaderInclude = DxShaderInclude();

#pragma region DxShader

DxShader::~DxShader() {
    if (mPreCreated) {
        MILO_ASSERT(mVShader != NULL, 0x60);
        MILO_ASSERT(mPShader != NULL, 0x61);
        mVShader = nullptr;
        mPShader = nullptr;
    } else {
        DX_RELEASE(mVShader);
        DX_RELEASE(mPShader);
    }
}

void DxShader::Select(bool vertexOnly) {
    TheDxRnd.Device()->SetVertexShader(mVShader);
    TheDxRnd.Device()->SetPixelShader(vertexOnly ? nullptr : mPShader);
    if (TheRnd.ShowShaderCost()) {
        float min, max;
        EstimatedCost(min, max);
        static float div = SystemConfig("rnd", "estimated_cost_divisor")->Float(1);
        Vector4 v;
        v.z = 0;
        v.w = 1;
        float div1 = ((min + max) / 2.0f) / div;
        float f3 = Max(0.0f, div1);
        float f2 = Max(0.0f, 1.0f - div1);
        v.x = Min(f3, 1.0f);
        v.y = Min(f2, 1.0f);
        TheShaderMgr.SetPConstant((PShaderConstant)4, v);
    }
}

void DxShader::Copy(const RndShaderProgram &src) {
    MILO_ASSERT(mPreCreated == false, 0xA5);
    MILO_ASSERT(src.Cached(), 0xA6);
    DX_RELEASE(mVShader);
    DX_RELEASE(mPShader);
    const DxShader &dxSrc = static_cast<const DxShader &>(src);
    mVShader = dxSrc.mVShader;
    mVShader->AddRef();
    mPShader = dxSrc.mPShader;
    mPShader->AddRef();
    mMinOverall = dxSrc.mMinOverall;
    mMaxOverall = dxSrc.mMaxOverall;
}

void DxShader::EstimatedCost(float &min, float &max) {
    if (mMinOverall < 0 || mMaxOverall < 0) {
        mMinOverall = 0;
        mMaxOverall = 0;
        if (mPShader) {
            UINT sizeOfData;
            mPShader->GetFunction(nullptr, &sizeOfData);
            if (sizeOfData != 0) {
                std::vector<char> chars(sizeOfData);
                auto it = chars.begin();
                mPShader->GetFunction(it, &sizeOfData);
                XGIDEALSHADERCOST shaderCost;
                if (XGEstimateIdealShaderCost(it, 0, &shaderCost) == 0) {
                    mMinOverall = shaderCost.MinOverall;
                    mMaxOverall = shaderCost.MaxOverall;
                }
            }
        }
    }
    min = mMinOverall;
    max = mMaxOverall;
}

RndShaderBuffer *DxShader::NewBuffer(unsigned int numBytes) {
    return new DxShaderBuffer(numBytes);
}

bool DxShader::Compile(
    ShaderType s,
    const ShaderOptions &opts,
    RndShaderBuffer *&bufVertex,
    RndShaderBuffer *&bufPixel
) {
    std::vector<ShaderMacro> defines;
    opts.GenerateMacros(s, defines);
    const char *shaderName = ShaderTypeName(s);
    MILO_ASSERT(streq("PIXEL_SHADER", defines[0].Name), 0xBB);
    MILO_ASSERT(!mVShader, 0xBD);
    MILO_ASSERT(!mPShader, 0xBE);
    LPCSTR data = nullptr;
    UINT bytes = 0;
    if (!SUCCEEDED(TheDxShaderInclude.Open(
            D3DXINC_LOCAL, shaderName, nullptr, (LPCVOID *)&data, &bytes, nullptr, 0
        ))) {
        return false;
    } else {
        for (int i = 0; i < 8; i++) {
            defines[i].Value = 0;
        }
        bufVertex = new DxShaderBuffer();
        defines[0].Value = "0";
        ID3DXBuffer *vertexShader;
        ID3DXBuffer *vertexErrorMsgs;
        HRESULT vRes = D3DXCompileShaderExA(
            data,
            bytes,
            reinterpret_cast<const D3DXMACRO *>(defines.begin()),
            &TheDxShaderInclude,
            "vshader",
            "vs_3_0",
            0,
            &vertexShader,
            &vertexErrorMsgs,
            nullptr,
            nullptr
        );
        bufPixel = new DxShaderBuffer();
        defines[0].Value = "1";
        ID3DXBuffer *pixelShader;
        ID3DXBuffer *pixelErrorMsgs;
        HRESULT pRes = D3DXCompileShaderExA(
            data,
            bytes,
            reinterpret_cast<const D3DXMACRO *>(defines.begin()),
            &TheDxShaderInclude,
            "pshader",
            "ps_3_0",
            0,
            &pixelShader,
            &pixelErrorMsgs,
            nullptr,
            nullptr
        );
        bool failed = !(SUCCEEDED(vRes)) || !(SUCCEEDED(pRes));
        if (failed) {
            if (!SUCCEEDED(vRes)) {
                if (vertexErrorMsgs) {
                    MILO_NOTIFY((const char *)vertexErrorMsgs->GetBufferPointer());
                } else {
                    MILO_NOTIFY("VShader '%s' compile failure: %d", shaderName, vRes);
                }
            }
            if (!SUCCEEDED(pRes)) {
                if (pixelErrorMsgs) {
                    MILO_NOTIFY((const char *)pixelErrorMsgs->GetBufferPointer());
                } else {
                    MILO_NOTIFY("PShader '%s' compile failure: %d", shaderName, pRes);
                }
            }
        }
        if (vertexErrorMsgs) {
            vertexErrorMsgs->Release();
            vertexErrorMsgs = nullptr;
        }
        if (pixelErrorMsgs) {
            pixelErrorMsgs->Release();
            pixelErrorMsgs = nullptr;
        }
        TheDxShaderInclude.DxShaderInclude::Close((LPCVOID)data);
        return !failed;
    }
}

void DxShader::CreateVertexShader(RndShaderBuffer &buffer) {
    MILO_ASSERT(mVShader == NULL, 0x80);
    HRESULT hr =
        TheDxRnd.Device()->CreateVertexShader((const DWORD *)buffer.Storage(), &mVShader);
    DX_ASSERT(hr, 0x82);
}

void DxShader::CreatePixelShader(RndShaderBuffer &buffer, ShaderType) {
    MILO_ASSERT(mPShader == NULL, 0x86);
    HRESULT hr =
        TheDxRnd.Device()->CreatePixelShader((const DWORD *)buffer.Storage(), &mPShader);
    DX_ASSERT(hr, 0x88);
}

void DxShader::SetShaders(D3DVertexShader *v, D3DPixelShader *p) {
    if (mCached) {
        MILO_ASSERT(mPreCreated, 0x92);
        MILO_ASSERT(mVShader, 0x93);
        MILO_ASSERT(mPShader, 0x94);
    } else {
        MILO_ASSERT(mVShader == NULL, 0x99);
        MILO_ASSERT(mPShader == NULL, 0x9A);
        MILO_ASSERT(mPreCreated == false, 0x9B);
        mVShader = v;
        mPShader = p;
        mCached = true;
        mPreCreated = true;
    }
}

#pragma endregion
#pragma region DxShaderMgr

void DxShaderMgr::PreInit() {
    unk60 = 0x38;
    RndShaderMgr::PreInit();
    RELEASE(mWorkMat);
    mWorkMat = Hmx::Object::New<RndMat>();
    CreateAndSetMetaMat(mWorkMat);
    RELEASE(mPostProcMat);
    mPostProcMat = Hmx::Object::New<RndMat>();
    CreateAndSetMetaMat(mPostProcMat);
    RELEASE(mDrawHighlightMat);
    mDrawHighlightMat = Hmx::Object::New<RndMat>();
    mDrawHighlightMat->SetUseEnv(false);
    mDrawHighlightMat->SetZMode(kZModeForce);
    mDrawHighlightMat->SetBlend(BaseMaterial::kBlendSrc);
    mDrawHighlightMat->SetAlphaCut(false);
    CreateAndSetMetaMat(mDrawHighlightMat);
    RELEASE(mDrawRectMat);
    mDrawRectMat = Hmx::Object::New<RndMat>();
    mDrawRectMat->SetZMode(kZModeDisable);
    mDrawRectMat->SetUseEnv(false);
    mDrawRectMat->SetPreLit(true);
    mDrawRectMat->SetBlend(BaseMaterial::kBlendSrcAlpha);
    mDrawRectMat->SetAlphaCut(false);
    CreateAndSetMetaMat(mDrawRectMat);
}

void DxShaderMgr::Terminate() {
    RELEASE(mDrawHighlightMat);
    RELEASE(mDrawRectMat);
    RELEASE(mWorkMat);
    RELEASE(mPostProcMat);
    RndShaderMgr::Terminate();
}

void DxShaderMgr::SetVConstant(VShaderConstant vsc, RndTex *tex) {
    if (tex) {
        tex->Select(vsc);
    } else {
        TheDxRnd.Device()->SetTexture(vsc, nullptr);
    }
}

void DxShaderMgr::SetVConstant(VShaderConstant vsc, const Vector4 &v4) {
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, (const float *)&v4, 1);
}

void DxShaderMgr::SetVConstant(
    VShaderConstant vsc, const float *__restrict fs, unsigned int num
) {
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, fs, num);
}

void DxShaderMgr::SetVConstant(VShaderConstant vsc, int i) {
    TheDxRnd.Device()->SetVertexShaderConstantI(vsc, &i, 1);
}

void DxShaderMgr::SetVConstant(VShaderConstant vsc, bool b) {
    BOOL msB = b;
    TheDxRnd.Device()->SetVertexShaderConstantB(vsc, &msB, 1);
}

void DxShaderMgr::SetVConstant(VShaderConstant vsc, const Hmx::Matrix4 &mtx) {
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, (const float *)&mtx.m[0], 1);
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, (const float *)&mtx.m[1], 1);
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, (const float *)&mtx.m[2], 1);
    TheDxRnd.Device()->SetVertexShaderConstantF(vsc, (const float *)&mtx.m[3], 1);
}

void DxShaderMgr::SetPConstant(PShaderConstant psc, RndCubeTex *tex) {
    if (tex) {
        tex->Select(psc);
    } else {
        TheRnd.GetNullTexture()->Select(psc);
    }
}

void DxShaderMgr::SetPConstant(PShaderConstant psc, const Vector4 &v4) {
    TheDxRnd.Device()->SetPixelShaderConstantF(psc, (const float *)&v4, 1);
}

void DxShaderMgr::SetPConstant(PShaderConstant psc, RndTex *tex) {
    if (!tex) {
        tex = TheRnd.GetNullTexture();
    }
    if (tex) {
        tex->Select(psc);
    } else {
        TheDxRnd.Device()->SetTexture(psc, nullptr);
    }
}

void DxShaderMgr::SetPConstant(PShaderConstant psc, int i) {
    TheDxRnd.Device()->SetPixelShaderConstantI(psc, &i, 1);
}

void DxShaderMgr::SetPConstant(PShaderConstant psc, bool b) {
    BOOL msB = b;
    TheDxRnd.Device()->SetPixelShaderConstantB(psc, &msB, 1);
}

void DxShaderMgr::LoadShaderFile(FileStream &fs) {
    RndSplasherResume();
    PhysMemTypeTracker tracker("D3D(phys):ShaderCache");
    unsigned int fileType, fileVersion;
    fs >> fileType;
    fs >> fileVersion;
    if (fileType == XBOX_SHADERS_TYPE && fileVersion == XBOX_SHADERS_VERSION) {
        int numShaders;
        fs >> numShaders;
        // for each shader
        for (unsigned int i = 0; i < numShaders; i++) {
            Symbol name;
            fs >> name;
            ShaderType shaderType = ShaderTypeFromName(name.Str());
            int alloc;
            fs >> alloc; // not sure of this var name
            void *shaders[2];
            void *physParts[2];
            shaders[0] = nullptr;
            shaders[1] = nullptr;
            physParts[0] = nullptr;
            physParts[1] = nullptr;
            for (unsigned int j = 0; j < 2; j++) {
                SIZE_T shaderSize, physPartSize;
                fs >> shaderSize;
                fs >> physPartSize;
                BeginMemTrackFileName(fs.Name());
                shaders[j] = XMemAlloc(shaderSize, 0x20800000);
                physParts[j] = XMemAlloc(physPartSize, 0xB5800000);
                EndMemTrackFileName();
                fs.Read(shaders[j], shaderSize);
                fs.Read(physParts[j], physPartSize);
            }
            ShaderPoolAlloc(alloc);
            RndSplasherSuspend();
            for (unsigned int j = 0; j < alloc; j++) {
                u64 shaderOptsMask;
                fs >> shaderOptsMask;
                D3DPixelShader *pPS = nullptr;
                D3DVertexShader *pVS = nullptr;
                for (unsigned int k = 0; k < 2; k++) {
                    int shaderOffset;
                    int physPartOffset;
                    fs >> shaderOffset;
                    fs >> physPartOffset;
                    void *curShader = (char *)shaders[k] + shaderOffset;
                    void *curPhysPart = (char *)physParts[k] + physPartOffset;
                    bool isVertex = k == 1;
                    if (isVertex) {
                        pVS = (D3DVertexShader *)curShader;
                        XGRegisterVertexShader(pVS, curPhysPart);
                    } else {
                        pPS = (D3DPixelShader *)curShader;
                        XGRegisterPixelShader(pPS, curPhysPart);
                    }
                }
                MILO_ASSERT(pPS != NULL, 0x1FA);
                MILO_ASSERT(pVS != NULL, 0x1FB);
                DxShader &shader =
                    static_cast<DxShader &>(FindShader(shaderType, shaderOptsMask));
                shader.SetShaders(pVS, pPS);
                RndSplasherPoll();
            }
            RndSplasherResume();
        }
    }
    RndSplasherSuspend();
}

RndShaderProgram *DxShaderMgr::NewShaderProgram() { return new DxShader(); }
