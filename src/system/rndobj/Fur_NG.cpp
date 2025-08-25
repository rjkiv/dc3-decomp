#include "Fur_NG.h"
#include "rnddx9/RenderState.h"
#include "rndobj/ShaderMgr.h"

bool NgFur::Prep(RndMesh *, RndMat *) const {
    TheShaderMgr.SetPConstant((PShaderConstant)12, mFurDetail);
    TheRenderState.SetTextureFilter(12, (RndRenderState::FilterMode)1, false);
    return true;
}
bool NgFur::Shell(int, RndMesh *, RndMat *) const { return true; }

NgFur::NgFur() {}
