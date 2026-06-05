#pragma once
#include "obj/Object.h"
#include "rndobj/CubeTex.h"
#include "xdk/D3D9.h"

class DxCubeTex : public RndCubeTex {
public:
    DxCubeTex();
    virtual ~DxCubeTex();
    OBJ_CLASSNAME(CubeTex);
    OBJ_SET_TYPE(CubeTex);
    virtual void Select(int);

    NEW_OBJ(DxCubeTex)

private:
    virtual void Reset();
    virtual void Sync();

    D3DCubeTexture *mTex; // 0x1ac
};
