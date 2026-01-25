#include "rndobj/Env_NG.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Rnd.h"
#include "rndobj/Stats_NG.h"

NgEnviron::NgEnviron()
    : mProjectedBlend(), mNumLightsReal(0), mNumLightsApprox(0), mNumLightsPoint(0),
      mNumLightsProj(0), mHasPointCubeTex(0) {}

void NgEnviron::UpdateApproxLighting(const Vector3 *) {}

void NgEnviron::Select(const Vector3 *pos) {
    mNumLightsReal = 0;
    mNumLightsApprox = 0;
    mNumLightsPoint = 0;
    mNumLightsProj = 0;
    mHasPointCubeTex = false;
    mProjectedBlend = (RndLight::ProjectedBlend)0;

    Rnd::DrawMode mode = TheRnd.GetDrawMode();
    if (mode == Rnd::kDrawOcclusion || mode == Rnd::kDrawExtrude || mode == Rnd::kDrawVelocity
        || mode == Rnd::kDrawShadowColor) {
        RndEnviron::Select(pos);
        NgMat::SetCurrent(0);
        TheNgStats->mLightsReal += mNumLightsReal;
        TheNgStats->mLightsApprox += mNumLightsApprox;
        return;
    }

    ReclassifyLights();
    RndEnviron::Select(pos);
    NgMat::SetCurrent(0);
    TheNgStats->mLightsReal += mNumLightsReal;
    TheNgStats->mLightsApprox += mNumLightsApprox;
}
