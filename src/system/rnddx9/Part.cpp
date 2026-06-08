#include "rnddx9/Part.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "xdk/win_types.h"

DxParticleSys::DxParticleSys() {}

static D3DVERTEXELEMENT9 sElement[5] = {
    { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
    { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, 32, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    D3DDECL_END()
};

void DxParticleSys::Init() {
    REGISTER_OBJ_FACTORY(DxParticleSys);
    MILO_ASSERT(!sVertexDecl, 0x46);
    HRESULT hr = TheDxRnd.Device()->CreateVertexDeclaration(sElement, &sVertexDecl);
    DX_ASSERT(hr, 0x47);
}
