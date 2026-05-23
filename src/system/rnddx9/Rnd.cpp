#include "rnddx9/Rnd.h"
#include "Tex.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Mat.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9caps.h"
#include "xdk/d3d9i/d3d9types.h"

DxRnd TheDxRnd;

BEGIN_HANDLERS(DxRnd)
    HANDLE_ACTION(suspend, Suspend())
    HANDLE_SUPERCLASS(Rnd)
END_HANDLERS

void DxRnd::Clear(unsigned int ui, const Hmx::Color &c) {
    float f1;
    if (unk_0x301) {
        f1 = 0;
    } else {
        f1 = 1;
    }
    int mask = 0;
    if (ui & 1) {
        mask = 0xF;
    }
    if (ui & 2) {
        mask |= 0x30;
    }
    D3DDevice_Clear(mD3DDevice, 0, nullptr, mask, MakeColor(c), f1, 0, 0);
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
    float vertices[24];
    vertices[0] = v1.x;
    vertices[1] = v1.y;
    vertices[2] = v1.z;
    vertices[3] = MakeColor(c);
    vertices[4] = v2.x;
    vertices[5] = v2.y;
    vertices[6] = v2.z;
    vertices[7] = vertices[3];
    Transform &xfm = reinterpret_cast<Transform &>(vertices[8]);
    xfm.Reset();
    TheShaderMgr.SetTransform(xfm);
    RndShader::SelectConfig(nullptr, b4 ? kLineShader : kLineNozShader, false);
    D3DDevice_SetFVF(mD3DDevice, 0x42);
    D3DDevice_DrawVerticesUP(mD3DDevice, D3DPT_LINELIST, 2, vertices, 0x10);
}

void DxRnd::MakeDrawTarget() {
    if (mWorldEnded) {
        D3DDevice_SetRenderTarget_External(mD3DDevice, 0, unk384);
        D3DDevice_SetDepthStencilSurface(mD3DDevice, unk38c);
    } else {
        D3DDevice_SetRenderTarget_External(mD3DDevice, 0, mBackBuffer);
        D3DDevice_SetDepthStencilSurface(mD3DDevice, unk388);
    }
    NgMat::SetCurrent(nullptr);
}

void DxRnd::SetViewport(const Viewport &v) {
    if (GetGfxMode() == kNewGfx) {
        NgRnd::SetViewport(v);
    }
    D3DVIEWPORT9 dxViewport;
    dxViewport.X = v.unk0;
    dxViewport.Y = v.unk4;
    dxViewport.Width = v.unk8;
    dxViewport.Height = v.unkc;
    if (unk_0x301) {
        dxViewport.MinZ = 1.0f - v.unk10;
        dxViewport.MaxZ = 1.0f - v.unk14;
    } else {
        dxViewport.MinZ = v.unk10;
        dxViewport.MaxZ = v.unk14;
    }
    D3DDevice_SetViewport(mD3DDevice, &dxViewport);
}

bool DxRnd::Offscreen() const {
    D3DSurface *back = BackBuffer();
    D3DSurface *target = D3DDevice_GetRenderTarget(mD3DDevice, 0);
    bool ret = back != target;
    if (target) {
        D3DResource_Release(target);
    }
    if (back) {
        D3DResource_Release(back);
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
        D3DDevice_SetIndices(mD3DDevice, data.unk0);
        D3DDevice_SetStreamSource(mD3DDevice, 0, data.unk4, 0, 20, 1);
        D3DDevice_SetFVF(mD3DDevice, 0x102);
        TheShaderMgr.SetVConstant((VShaderConstant)0x5c, Hmx::Matrix4(tf));
        DxTex *tex = static_cast<DxTex *>(mat->GetDiffuseTex());
        D3DDevice_SetTexture(mD3DDevice, 0x10, tex->Tex(), 0x8000);
        D3DDevice_SetTexture(mD3DDevice, 0, tex->Tex(), 0x80000000);
        D3DDevice_DrawIndexedVertices(
            mD3DDevice, D3DPT_QUADLIST, 0, 0, (data.unkc - 1) * (data.unk8 - 1) * 4
        );
        if (!next)
            break;
        it = next;
        next = mat->NextPass();
    }
    D3DDevice_SetIndices(mD3DDevice, nullptr);
    D3DDevice_SetStreamSource(mD3DDevice, 0, nullptr, 0, 0, 1);
    D3DDevice_SetTexture(mD3DDevice, 0x10, nullptr, 0x8000);
}

void DxRnd::SetVertShaderTex(RndTex *tex, int i2) {
    DxTex *dxTex = static_cast<DxTex *>(tex);
    D3DDevice_SetTexture(
        mD3DDevice,
        i2 + 0x10,
        dxTex ? dxTex->Tex() : nullptr,
        0x8000000000000000 >> (i2 + 0x30U & 0x7F)
    );
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
    HRESULT res = D3DDevice_Reset(mD3DDevice, &mPresentParams);
    DX_ASSERT_CODE(res, 0xD6);
    PostDeviceReset();
}

long DxRnd::GetDeviceCaps(D3DCAPS9 *cap) {
    return Direct3D_GetDeviceCaps(0, mDeviceType, cap);
}

void DxRnd::DrawSafeArea(float percent, bool widescreen, const Hmx::Color &color) {
    if (mShrinkToSafe)
        percent = percent * 1.0526316f;

    Vector2 vec1;
    Vector2 vec2;

    float targetAspect = widescreen ? 16.f / 9.f : 4.f / 3.f;
    float realAspect = (float)mHeight / mWidth;

    vec1.y = (1.0f - percent) * 0.5f;
    vec1.x = -(targetAspect * realAspect - 1.0f) * 0.5f + vec1.y;

    vec2.x = 1.0f - vec1.x;
    vec2.y = 1.0f - vec1.y;

    UtilDrawRect2D(vec1, vec2, color);
}
