#include "Mesh.h"
#include "Rnd.h"
#include "rnddx9/Utl.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"

DxMesh::DxMesh() : mNumVerts(0), mNumFaces(0), unk1ac(0), unk1b0(0) {
    if (!sVertexDecl) {
        // clang-format off
        static const D3DVERTEXELEMENT9 sVertexElements[] = {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT16_2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 20, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 24, D3DDECLTYPE_DEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
            { 0, 28, D3DDECLTYPE_UDEC4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
            { 0, 32, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
            D3DDECL_END()
        };
        // clang-format on
        HRESULT hr =
            TheDxRnd.Device()->CreateVertexDeclaration(sVertexElements, &sVertexDecl);
        DX_ASSERT(hr, 0xA8);
    }
    if (!sMutableVertexDecl) {
        // clang-format off
        static const D3DVERTEXELEMENT9 sMutableVertexElements[] = {
            { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 72, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
            { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
            D3DDECL_END()
        };
        // clang-format on
        HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(
            sMutableVertexElements, &sMutableVertexDecl
        );
        DX_ASSERT(hr, 0xAF);
    }
    if (!sMutableSkinnedVertexDecl) {
        // clang-format off
        static const D3DVERTEXELEMENT9 sMutableSkinnedVertexElements[] = {
            { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            { 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
            { 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 72, D3DDECLTYPE_SHORT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
            { 0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
            D3DDECL_END()
        };
        // clang-format on
        HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(
            sMutableSkinnedVertexElements, &sMutableSkinnedVertexDecl
        );
        DX_ASSERT(hr, 0xB5);
    }
}

DxMesh::~DxMesh() {
    DX_RELEASE(unk1ac);
    DX_RELEASE(unk1b0);
}

void _fake(void) {
    BufLock<struct D3DVertexBuffer> buf(nullptr, 0);
    BufLock<struct D3DIndexBuffer> buf2(nullptr, 0);
}
