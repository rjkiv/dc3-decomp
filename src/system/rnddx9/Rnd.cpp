#include "rnddx9/Rnd.h"
#include "Tex.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Object.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Mat.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "xdk/D3D9.h"

DxRnd TheDxRnd;

BEGIN_HANDLERS(DxRnd)
    HANDLE_ACTION(suspend, Suspend())
    HANDLE_SUPERCLASS(Rnd)
END_HANDLERS

void DxRnd::Clear(unsigned int ui, const Hmx::Color &c) {
    float z;
    if (unk_0x301) {
        z = 0;
    } else {
        z = 1;
    }
    int mask = 0;
    if (ui & 1) {
        mask = 0xF;
    }
    if (ui & 2) {
        mask |= 0x30;
    }
    mD3DDevice->Clear(0, nullptr, mask, MakeColor(c), z, 0);
}

void DxRnd::DrawRect(
    const Hmx::Rect &rect,
    const Hmx::Color &colorRef,
    RndMat *mat,
    const Hmx::Color *colorPtr1,
    const Hmx::Color *colorPtr2
) {
    DrawRect(rect, mat, kDrawRectShader, colorRef, colorPtr1, colorPtr2);
}

void DxRnd::DrawLine(const Vector3 &v1, const Vector3 &v2, const Hmx::Color &c, bool b4) {
    DxLineVertex vertices[2];
    vertices[0].x = v1.x;
    vertices[0].y = v1.y;
    vertices[0].z = v1.z;
    vertices[0].diffuse = MakeColor(c);
    vertices[1].x = v2.x;
    vertices[1].y = v2.y;
    vertices[1].z = v2.z;
    vertices[1].diffuse = vertices[0].diffuse;
    Transform xfm;
    xfm.Reset();
    TheShaderMgr.SetTransform(xfm);
    RndShader::SelectConfig(nullptr, b4 ? kLineShader : kLineNozShader, false);
    mD3DDevice->SetFVF(D3DFVF_DIFFUSE | D3DFVF_XYZ);
    mD3DDevice->DrawVerticesUP(
        D3DPT_LINELIST, DIM(vertices), vertices, sizeof(DxLineVertex)
    );
}

void DxRnd::MakeDrawTarget() {
    if (mWorldEnded) {
        mD3DDevice->SetRenderTarget(0, unk384);
        mD3DDevice->SetDepthStencilSurface(unk38c);
    } else {
        mD3DDevice->SetRenderTarget(0, mBackBuffer);
        mD3DDevice->SetDepthStencilSurface(unk388);
    }
    NgMat::SetCurrent(nullptr);
}

void DxRnd::SetViewport(const Viewport &v) {
    if (GetGfxMode() == kNewGfx) {
        NgRnd::SetViewport(v);
    }
    D3DVIEWPORT9 dxViewport;
    dxViewport.X = v.mX;
    dxViewport.Y = v.mY;
    dxViewport.Width = v.mWidth;
    dxViewport.Height = v.mHeight;
    if (unk_0x301) {
        dxViewport.MinZ = 1.0f - v.mMinZ;
        dxViewport.MaxZ = 1.0f - v.mMaxZ;
    } else {
        dxViewport.MinZ = v.mMinZ;
        dxViewport.MaxZ = v.mMaxZ;
    }
    mD3DDevice->SetViewport(&dxViewport);
}

bool DxRnd::Offscreen() const {
    D3DSurface *back = BackBuffer();
    D3DSurface *target;
    mD3DDevice->GetRenderTarget(0, &target);
    bool ret = target != back;
    if (target) {
        target->Release();
    }
    if (back) {
        back->Release();
    }
    return ret;
}

void DxRnd::DrawLargeQuad(
    const LargeQuadRenderData &data, const Transform &tf, RndMat *mat, ShaderType s
) {
    RndMat *it = mat;
    RndMat *next = mat ? mat->NextPass() : nullptr;
    while (true) {
        RndShader::SelectConfig(it, s, false);
        mD3DDevice->SetIndices(data.unk0);
        mD3DDevice->SetStreamSource(0, data.unk4, 0, 20);
        mD3DDevice->SetFVF(D3DFVF_TEX1 | D3DFVF_XYZ);
        TheShaderMgr.SetVConstant((VShaderConstant)0x5c, Hmx::Matrix4(tf));
        DxTex *tex = static_cast<DxTex *>(mat->GetDiffuseTex());
        mD3DDevice->SetTexture(16, tex->Tex());
        mD3DDevice->SetTexture(0, tex->Tex());
        mD3DDevice->DrawIndexedVertices(
            D3DPT_QUADLIST, 0, 0, (data.unkc - 1) * (data.unk8 - 1) * 4
        );
        if (!next)
            break;
        it = next;
        next = mat->NextPass();
    }
    mD3DDevice->SetIndices(nullptr);
    mD3DDevice->SetStreamSource(0, nullptr, 0, 0);
    mD3DDevice->SetTexture(16, nullptr);
}

void DxRnd::SetVertShaderTex(RndTex *tex, int i2) {
    DxTex *dxTex = static_cast<DxTex *>(tex);
    mD3DDevice->SetTexture(i2 + 0x10, dxTex ? dxTex->Tex() : nullptr);
}

void DxRnd::PreDeviceReset() {
    if (mOcclusionQueryMgr) {
        mOcclusionQueryMgr->ReleaseQueries();
    }
    FOREACH (it, unk2b0) {
        (*it)->PreDeviceReset();
    }
    ReleaseAutoRelease();
}

void DxRnd::PostDeviceReset() {
    FOREACH (it, unk2b0) {
        (*it)->PostDeviceReset();
    }
    MakeDrawTarget();
    InitRenderState();
}

D3DFORMAT DxRnd::D3DFormatForBitmap(const RndBitmap &bitmap) {
    int fmt = bitmap.Order() & 0x38;
    int bpp = bitmap.Bpp();
    if (fmt != 0) {
        switch (fmt) {
        case 8:
            return D3DFMT_DXT1;
        case 0x10:
            return D3DFMT_DXT3;
        case 0x18:
            return D3DFMT_DXT5;
        case 0x20:
            return D3DFMT_DXN;
        default:
            MILO_FAIL("Invalid dxt format: %d", fmt);
            MILO_ASSERT(fmt != D3DFMT_UNKNOWN, 999);
            return (D3DFORMAT)0xffffffff;
        }
    } else {
        switch (bpp) {
        case 4:
        case 8:
            return D3DFMT_A8R8G8B8;
        case 0x10:
            return D3DFMT_A1R5G5B5;
        case 0x18:
            return D3DFMT_X8R8G8B8;
        case 0x20:
            return D3DFMT_A8R8G8B8;
        default:
            MILO_FAIL("Invalid bpp: %d", bpp);
            MILO_ASSERT(fmt != D3DFMT_UNKNOWN, 999);
            return (D3DFORMAT)0xffffffff;
        }
    }
}

int DxRnd::BitmapOrderForD3DFormat(D3DFORMAT fmt) {
    switch (fmt) {
    case D3DFMT_DXT1:
    case D3DFMT_LIN_DXT1:
        return 8;
    case D3DFMT_DXT3:
    case D3DFMT_LIN_DXT3:
        return 0x10;
    case D3DFMT_DXT5:
    case D3DFMT_LIN_DXT5:
        return 0x18;
    case D3DFMT_DXN:
    case D3DFMT_LIN_DXN:
        return 0x20;
    default:
        return 0;
    }
}

void DxRnd::ResetDevice() {
    PreDeviceReset();
    HRESULT res = mD3DDevice->Reset(&mPresentParams);
    DX_ASSERT(res, 0xD6);
    PostDeviceReset();
}

HRESULT DxRnd::GetDeviceCaps(D3DCAPS9 *cap) {
    return Direct3D::GetDeviceCaps(0, mDeviceType, cap);
}

void DxRnd::DrawSafeArea(float percent, bool widescreen, const Hmx::Color &color) {
    if (mShrinkToSafe) {
        percent *= 1.0526316f;
    }

    float realAspect = (float)mHeight / (float)mWidth;
    float targetAspect = widescreen ? 16.0f / 9.0f : 4.0f / 3.0f;
    float mult = targetAspect * realAspect;
    float set = (1.0f - percent) / 2.0f;

    Vector2 vec1(set + (mult - 1.0f) / 2.0f, set);
    Vector2 vec2(1.0f - vec1.x, 1.0f - vec1.y);
    UtilDrawRect2D(vec1, vec2, color);
}
