#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Part.h"
#include "xdk/D3D9.h"

class DxParticleSys : public RndParticleSys, public DxObject {
public:
    virtual ~DxParticleSys() {}
    OBJ_CLASSNAME(ParticleSys);
    OBJ_SET_TYPE(ParticleSys);
    virtual void DrawShowing();
    virtual void SetPool(int x, Type t) { RndParticleSys::SetPool(x, t); }

    static void Init();
    NEW_OBJ(DxParticleSys);

protected:
    DxParticleSys();
    void DrawParticles(const Hmx::Color &col);

    static D3DVertexDeclaration *sVertexDecl;
};
