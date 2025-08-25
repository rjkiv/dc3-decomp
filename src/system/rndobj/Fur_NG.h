#pragma once

#include "obj/Object.h"
#include "rndobj/Fur.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"

class NgFur : public RndFur {
public:
    OBJ_CLASSNAME(NgFur)
    OBJ_SET_TYPE(NgFur)
    virtual bool Prep(RndMesh *, RndMat *) const;
    virtual bool Shell(int, RndMesh *, RndMat *) const;

protected:
    NgFur();
};
