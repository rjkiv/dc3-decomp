#include "rnddx9/Rnd.h"
#include "Tex.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rnddx9/Object.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Cam.h"
#include "rndobj/Mat.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/MemTrack.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

DxRnd TheDxRnd;
Rnd &TheRnd = TheDxRnd;
NgRnd &TheNgRnd = TheDxRnd;

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

void DxRnd::PushClipPlanesInternal(ObjPtrVec<RndTransformable> &planes) {
    int mask = 0;
    for (int i = 0; i < unk408; i++) {
        mask |= (1 << i);
    }
    for (int i = 0; i < planes.size() && i < 6; i++) {
        RndTransformable *cur = planes[i];
        if (cur) {
            const Transform &worldXfm = cur->WorldXfm();

            Vector3 tmp;
            Scale(worldXfm.m.z, -1, tmp);
            Plane p70(worldXfm.v, tmp);
            Multiply(p70.AsVector4(), RndCam::Current()->GetMatrix340(), p70.AsVector4());
            float plane[4] = { p70.a, p70.b, p70.c, p70.d };
            mD3DDevice->SetClipPlane(unk408, plane);
            mask |= (1 << unk408++);
        }
    }
    TheDxRnd.Device()->SetRenderState(D3DRS_CLIPPLANEENABLE, mask);
}

void DxRnd::PopClipPlanesInternal(ObjPtrVec<RndTransformable> &planes) {
    for (int i = 0; i < planes.size() && i < 6; i++) {
        if (planes[i]) {
            unk408--;
        }
    }
    int mask = 0;
    for (int i = 0; i < unk408; i++) {
        mask |= (1 << i);
    }
    TheDxRnd.Device()->SetRenderState(D3DRS_CLIPPLANEENABLE, mask);
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

// DrawRect

// size 0x30
struct DxRectVertex {
    float posX, posY, posZ;
    float tex1X, tex1Y, tex1Z;
    float tex2X, tex2Y, tex2Z;
    float tex3X, tex3Y, tex3Z;
};

static D3DVertexDeclaration *sRectDecl;

static D3DVERTEXELEMENT9 sRectElements[] = { { 0,
                                               offsetof(DxRectVertex, posX),
                                               D3DDECLTYPE_FLOAT3,
                                               D3DDECLMETHOD_DEFAULT,
                                               D3DDECLUSAGE_POSITION,
                                               0 },
                                             { 0,
                                               offsetof(DxRectVertex, tex1X),
                                               D3DDECLTYPE_FLOAT3,
                                               D3DDECLMETHOD_DEFAULT,
                                               D3DDECLUSAGE_TEXCOORD,
                                               0 },
                                             { 0,
                                               offsetof(DxRectVertex, tex2X),
                                               D3DDECLTYPE_FLOAT3,
                                               D3DDECLMETHOD_DEFAULT,
                                               D3DDECLUSAGE_TEXCOORD,
                                               1 },
                                             { 0,
                                               offsetof(DxRectVertex, tex3X),
                                               D3DDECLTYPE_FLOAT3,
                                               D3DDECLMETHOD_DEFAULT,
                                               D3DDECLUSAGE_TEXCOORD,
                                               2 },
                                             D3DDECL_END() };

void DxRnd::DrawRectDepth(
    const Vector3 &v3,
    const Vector3 (&varr)[4],
    const Vector4 &v4,
    RndMat *mat,
    ShaderType shaderType
) {
    TheShaderMgr.SetPConstant((PShaderConstant)0x59, v4);

    static DxRectVertex sRectVertices[] = {
        { -1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { -1, -1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, -1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    };

    for (int i = 0; i < DIM(sRectVertices); i++) {
        sRectVertices[i].tex2X = v3.x;
        sRectVertices[i].tex2Y = v3.y;
        sRectVertices[i].tex2Z = v3.z;
        sRectVertices[i].tex3X = varr[i].x;
        sRectVertices[i].tex3Y = varr[i].y;
        sRectVertices[i].tex3Z = varr[i].z;
    }
    RndShader::SelectConfig(mat, shaderType, false);
    if (!sRectDecl) {
        HRESULT hr = mD3DDevice->CreateVertexDeclaration(sRectElements, &sRectDecl);
        DX_ASSERT(hr, 0x2C3);
    }
    TheDxRnd.Device()->SetRenderState(D3DRS_HALFPIXELOFFSET, 1);
    mD3DDevice->SetVertexDeclaration(sRectDecl);
    mD3DDevice->DrawVerticesUP(
        D3DPT_TRIANGLESTRIP, DIM(sRectVertices), sRectVertices, sizeof(DxRectVertex)
    );
    TheDxRnd.Device()->SetRenderState(D3DRS_HALFPIXELOFFSET, 0);
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

void DxRnd::CreateLargeQuad(int i1, int i2, LargeQuadRenderData &quadData) {
    int i1sub = i1 - 1;
    int i2sub = i2 - 1;
    D3DIndexBuffer *indexBuffer;
    BeginMemTrackObjectName(__FUNCTION__);
    HRESULT hr = mD3DDevice->CreateIndexBuffer(
        i2sub * i1sub * 0x10, 0, D3DFMT_INDEX32, 0, &indexBuffer, nullptr
    );
    DX_ASSERT(hr, 0x2E3);
    EndMemTrackObjectName();
    void *data;
    indexBuffer->Lock(0, 0, &data, 0);
    for (int i = 0; i < i2sub; i++) {
        for (int j = 0; j < i1sub; j++) {
        }
    }
    D3DVertexBuffer *vertexBuffer;
    BeginMemTrackObjectName(__FUNCTION__);
    UINT vLen = i2 * i1 * 0x14;
    hr = mD3DDevice->CreateVertexBuffer(vLen, 0, 0, 0, &vertexBuffer, nullptr);
    DX_ASSERT(hr, 0x2FB);
    EndMemTrackObjectName();
    vertexBuffer->Lock(0, vLen, &data, 0);
    for (int i = 0; i < i2; i++) {
        for (int j = 0; j < i1; j++) {
        }
    }
    vertexBuffer->Unlock();
    quadData.unk0 = indexBuffer;
    quadData.unk4 = vertexBuffer;
    quadData.unk8 = i1;
    quadData.unkc = i2;
}

void DxRnd::DrawLargeQuad(
    const LargeQuadRenderData &data, const Transform &tf, RndMat *mat, ShaderType s
) {
    RndMat *next = mat ? mat->NextPass() : nullptr;
    while (mat != nullptr) {
        RndShader::SelectConfig(mat, s, false);
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
        if (next) {
            mat = next;
            next = next->NextPass();
        } else {
            break;
        }
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
            break;
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
            break;
        }
    }
    MILO_ASSERT(fmt != D3DFMT_UNKNOWN, 999);
    return D3DFMT_UNKNOWN;
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

    Vector2 vec1(set + (1.0f - mult) / 2.0f, set);
    Vector2 vec2(1.0f - vec1.x, 1.0f - vec1.y);
    UtilDrawRect2D(vec1, vec2, color);
}
