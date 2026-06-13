#include "rndobj/Rnd_NG.h"
#include "Env_NG.h"
#include "PostProc.h"
#include "math/Color.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/DOFProc_NG.h"
#include "rndobj/Flare.h"
#include "rndobj/Fur_NG.h"
#include "rndobj/Lit_NG.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Overlay.h"
#include "rndobj/PostProc_NG.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShadowMap.h"
#include "rndobj/SoftParticleBuffer.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Tex.h"
#include "utl/MakeString.h"

NgStats gNgStats[3];
NgStats *TheNgStats = &gNgStats[0];

NgRnd::NgRnd()
    : unk1f8(0), mShadowMap(0), mShadowCam(0), mOcclusionQueryMgr(0), unk208(0),
      unk218(0) {}

NgRnd::~NgRnd() {}

void NgRnd::PreInit() {
    if (!unk218) {
        unk218 = true;
        Rnd::PreInit();
        REGISTER_OBJ_FACTORY(NgEnviron)
        REGISTER_OBJ_FACTORY(NgMat)
        NgLight::Init();
        REGISTER_OBJ_FACTORY(NgFur)
        RndShadowMap::Init();
        REGISTER_OBJ_FACTORY(RndSoftParticleBuffer)
        CreateDefaults();
    }
}

void NgRnd::Init() {
    PreInit();
    TheShaderMgr.Init();
    unk20c.reserve(0x200);
    Rnd::Init();
    unk208 = Hmx::Object::New<RndSoftParticleBuffer>();
}

void NgRnd::ReInit() { TheShaderMgr.InitShaders(); }

void NgRnd::Terminate() {
    RELEASE(mOcclusionQueryMgr);
    RELEASE(unk208);
    TheShaderMgr.Terminate();
    NgPostProc::Terminate();
    NgDOFProc::Terminate();
    RndShadowMap::Terminate();
    NgLight::Terminate();
    Rnd::Terminate();
}

void NgRnd::SetShadowMap(RndTex *tex, RndCam *cam, const Hmx::Color *color) {
    mShadowMap = tex;
    mShadowCam = cam;
    if (cam) {
        const Vector3 &v3 = cam->WorldXfm().m.y;
        Vector4 v4(v3.x, v3.y, v3.z, 1);
        TheShaderMgr.SetPConstant((PShaderConstant)0x6c, v4);
    }
    if (color) {
        Vector4 v4 = Vector4(color->red, color->green, color->blue, color->alpha);
        TheShaderMgr.SetPConstant((PShaderConstant)0x6B, v4);
    }
    if (tex) {
        TheShaderMgr.SetPConstant((PShaderConstant)5, tex);
    }
}

void NgRnd::RemovePointTest(RndFlare *flare) {
    Rnd::RemovePointTest(flare);
    FOREACH (it, unk20c) {
        if (it->unk0 == flare) {
            mOcclusionQueryMgr->ReleaseQuery(it->unk8);
            mOcclusionQueryMgr->ReleaseQuery(it->unk4);
            unk20c.erase(it);
            return;
        }
    }
}

void NgRnd::DoPostProcess() { Rnd::DoPostProcess(); }

void NgRnd::CreateLargeQuad(int, int, LargeQuadRenderData &) {
    MILO_FAIL("NgRnd::CreateLargeQuad not implemented!");
}

void NgRnd::DrawLargeQuad(
    const LargeQuadRenderData &, const Transform &, RndMat *, ShaderType
) {
    MILO_FAIL("NgRnd::DrawLargeQuad not implemented!");
}

void NgRnd::SetVertShaderTex(RndTex *, int) {
    MILO_FAIL("NgRnd::SetVertShaderTex not implemented!");
}

void NgRnd::ResetStats() {
    if (mProcCmds == kProcessWorld || mProcCmds == kProcessAll) {
        TheNgStats = &gNgStats[0];
    } else if (mProcCmds == kProcessPost) {
        TheNgStats = &gNgStats[1];
    } else {
        TheNgStats = &gNgStats[2];
    }
    memset(TheNgStats, 0, sizeof(NgStats));
    TheNgStats->mCams++;
}

float EstimateDraw(int idx) {
    return gNgStats[idx].mMotionBlurs * 0.003f + gNgStats[idx].mFlares * 0.017f
        + gNgStats[idx].mMultiMeshInsts * 0.001f + gNgStats[idx].mLightsApprox * 0.01f
        + gNgStats[idx].mLightsReal * 0.001f + gNgStats[idx].mCams * 0.0068f
        + gNgStats[idx].mMats * 0.0097f + gNgStats[idx].mBones * 0.00126f
        + gNgStats[idx].mMutMeshes * 0.0112f + gNgStats[idx].mRegMeshes * 0.0028f
        + gNgStats[idx].mParts * 0.00023333334f + gNgStats[idx].mPartSys * 0.005f;
}

float NgRnd::UpdateOverlay(RndOverlay *overlay, float y) {
    if (overlay == mStatsOverlay) {
        mStatsOverlay->Clear();
        if (mProcCmds == kProcessWorld || mProcCmds == kProcessPost) {
            *mStatsOverlay
                << MakeString("faces %d %d\n", gNgStats[0].mFaces, gNgStats[1].mFaces);
            *mStatsOverlay
                << MakeString("parts %d %d\n", gNgStats[0].mParts, gNgStats[1].mParts);
            *mStatsOverlay << MakeString(
                "part_sys %d %d\n", gNgStats[0].mPartSys, gNgStats[1].mPartSys
            );
            *mStatsOverlay << MakeString(
                "reg_meshes %d %d\n", gNgStats[0].mRegMeshes, gNgStats[1].mRegMeshes
            );
            *mStatsOverlay << MakeString(
                "mut_meshes %d %d\n", gNgStats[0].mMutMeshes, gNgStats[1].mMutMeshes
            );
            *mStatsOverlay
                << MakeString("bones %d %d\n", gNgStats[0].mBones, gNgStats[1].mBones);
            *mStatsOverlay
                << MakeString("mats %d %d\n", gNgStats[0].mMats, gNgStats[1].mMats);
            *mStatsOverlay
                << MakeString("cams %d %d\n", gNgStats[0].mCams, gNgStats[1].mCams);
            *mStatsOverlay << MakeString(
                "lights (real) %d %d\n", gNgStats[0].mLightsReal, gNgStats[1].mLightsReal
            );
            *mStatsOverlay << MakeString(
                "lights (approx) %d %d\n",
                gNgStats[0].mLightsApprox,
                gNgStats[1].mLightsApprox
            );
            *mStatsOverlay << MakeString(
                "multimesh instances %d %d\n",
                gNgStats[0].mMultiMeshInsts,
                gNgStats[1].mMultiMeshInsts
            );
            *mStatsOverlay << MakeString(
                "multimesh batches %d %d\n",
                gNgStats[0].mMultiMeshBatches,
                gNgStats[1].mMultiMeshBatches
            );
            *mStatsOverlay
                << MakeString("flares %d %d\n", gNgStats[0].mFlares, gNgStats[1].mFlares);
            *mStatsOverlay << MakeString(
                "motion blur %d %d\n", gNgStats[0].mMotionBlurs, gNgStats[1].mMotionBlurs
            );
            *mStatsOverlay << MakeString(
                "spotlights %d %d\n", gNgStats[0].mSpotlights, gNgStats[1].mSpotlights
            );
            *mStatsOverlay
                << MakeString("est draw %.1f %.1f\n", EstimateDraw(0), EstimateDraw(1));
            TheNgStats = &gNgStats[2];
        } else {
            *mStatsOverlay << MakeString("faces %d\n", gNgStats[0].mFaces);
            *mStatsOverlay << MakeString("parts %d\n", gNgStats[0].mParts);
            *mStatsOverlay << MakeString("part_sys %d\n", gNgStats[0].mPartSys);
            *mStatsOverlay << MakeString("reg_meshes %d\n", gNgStats[0].mRegMeshes);
            *mStatsOverlay << MakeString("mut_meshes %d\n", gNgStats[0].mMutMeshes);
            *mStatsOverlay << MakeString("bones %d\n", gNgStats[0].mBones);
            *mStatsOverlay << MakeString("mats %d\n", gNgStats[0].mMats);
            *mStatsOverlay << MakeString("cams %d\n", gNgStats[0].mCams);
            *mStatsOverlay << MakeString("lights (real) %d\n", gNgStats[0].mLightsReal);
            *mStatsOverlay
                << MakeString("lights (approx) %d\n", gNgStats[0].mLightsApprox);
            *mStatsOverlay
                << MakeString("multimesh instances %d\n", gNgStats[0].mMultiMeshInsts);
            *mStatsOverlay
                << MakeString("multimesh batches %d\n", gNgStats[0].mMultiMeshBatches);
            *mStatsOverlay << MakeString("flares %d\n", gNgStats[0].mFlares);
            *mStatsOverlay << MakeString("motion blur %d\n", gNgStats[0].mMotionBlurs);
            *mStatsOverlay << MakeString("spotlights %d\n", gNgStats[0].mSpotlights);
            *mStatsOverlay << MakeString("est draw %.1f\n", EstimateDraw(0));
            TheNgStats = &gNgStats[2];
        }
        return y;
    } else {
        return Rnd::UpdateOverlay(overlay, y);
    }
}
