#pragma once

#include "obj/Object.h"
#include "rndobj/Fur.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"

class NgFur : public RndFur {
public:
    OBJ_CLASSNAME(Fur)
    OBJ_SET_TYPE(Fur)
    virtual bool Prep(RndMesh *, RndMat *) const;
    virtual bool Shell(int, RndMesh *, RndMat *) const;

    NEW_OBJ(NgFur)
    REGISTER_OBJ(NgFur)

protected:
    NgFur();
};
