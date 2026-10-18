#include "rndobj/TexBlender.h"
#include "Utl.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/Tex.h"
#include "rndobj/TexBlendController.h"

#pragma region Hmx::Object

RndTexBlender::RndTexBlender()
    : mBaseMap(this), mNearMap(this), mFarMap(this), mOutputTexture(this),
      mControllerList(this), mOwner(this), mFactorScale(1), mOutputTexState(0),
      mDirty(true) {}

BEGIN_HANDLERS(RndTexBlender)
    HANDLE(get_render_textures, OnGetRenderTextures)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndTexBlender)
    SYNC_PROP(base_map, mBaseMap)
    SYNC_PROP(near_map, mNearMap)
    SYNC_PROP(far_map, mFarMap)
    SYNC_PROP(output_texture, mOutputTexture)
    SYNC_PROP(controller_list, mControllerList)
    SYNC_PROP(owner, mOwner)
    SYNC_PROP(controller_influence, mFactorScale)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndTexBlender)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mOutputTexture;
    bs << mBaseMap;
    bs << mNearMap;
    bs << mFarMap;
    bs << mControllerList;
    bs << mOwner;
    bs << mFactorScale;
END_SAVES

BEGIN_COPYS(RndTexBlender)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndTexBlender)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOutputTexture)
        COPY_MEMBER(mBaseMap)
        COPY_MEMBER(mNearMap)
        COPY_MEMBER(mFarMap)
        COPY_MEMBER(mControllerList)
        COPY_MEMBER(mOwner)
        COPY_MEMBER(mFactorScale)
    END_COPYING_MEMBERS
    mOutputTexState = 0;
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndTexBlender)
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mOutputTexture;
    d >> mBaseMap;
    d >> mNearMap;
    d >> mFarMap;
    d >> mControllerList;
    d >> mOwner;
    if (d.rev > 1)
        d >> mFactorScale;
    else
        mFactorScale = 0.7071068f;
    mOutputTexState = 0;
END_LOADS

#pragma endregion
#pragma region RndDrawable

float RndTexBlender::GetDistanceToPlane(const Plane &plane, Vector3 &vec) {
    if (mOwner) {
        return mOwner->GetDistanceToPlane(plane, vec);
    } else
        return 0;
}

bool RndTexBlender::MakeWorldSphere(Sphere &sphere, bool b) {
    if (mOwner) {
        return mOwner->MakeWorldSphere(sphere, b);
    } else
        return 0;
}

struct BlendSorter {
    bool operator()(
        const std::pair<RndTexBlendController *, float> &p1,
        const std::pair<RndTexBlendController *, float> &p2
    ) const {
        return p1.second < p2.second;
    }
};

void RndTexBlender::DrawShowing() {
    if (TheRnd.DrawMode() == Rnd::kDrawNormal
        && ((TheRnd.ProcCmds() & kProcessWorld) != 0 || TheRnd.ProcCmds() == 0)
        && mOutputTexture) {
        if ((mOutputTexture->GetType() & RndTex::kRenderedNoZ) != RndTex::kRenderedNoZ) {
            MILO_NOTIFY_ONCE(
                "%s: \"%s\" must be renderable with no z-buffer",
                PathName(this),
                mOutputTexture->Name()
            );
        } else {
            if (0x40000 < mOutputTexture->Width() * mOutputTexture->Height()) {
                MILO_NOTIFY_ONCE(
                    "%s: \"%s\" is %d x %d, must be no larger than 512 x 512",
                    PathName(this),
                    mOutputTexture->Name(),
                    mOutputTexture->Height(),
                    mOutputTexture->Width()
                );
            }
            std::vector<std::pair<RndTexBlendController *, float> > pairs1b0, pairs1c0,
                pairs1d0;
            FOREACH (it, mControllerList) {
                RndTexBlendController *curCtrlr = *it;
                float second;
                RndTexBlendController::BlendState state =
                    curCtrlr->GetBlendState(second, mFactorScale);
                switch (state) {
                case 1:
                    pairs1b0.push_back(std::make_pair(curCtrlr, second));
                    break;
                case 2:
                    pairs1c0.push_back(std::make_pair(curCtrlr, second));
                    break;
                case 3:
                    pairs1d0.push_back(std::make_pair(curCtrlr, second));
                    break;
                }
            }
            if (mDirty || !pairs1b0.empty() || !pairs1c0.empty() || !pairs1d0.empty()
                || mOutputTexState != 1) {
                mDirty = false;
                RndCam *cam = TheRnd.GetDefaultCam();
                RndTex *targetTex = RndCam::Current()->TargetTex();
                if (targetTex) {
                    MILO_NOTIFY_ONCE(
                        "%s: Cannot render to texture (%s) while already rendering to texture (%s).",
                        PathName(targetTex),
                        PathName(this),
                        PathName(targetTex)
                    );
                }
                cam->SetTargetTex(mOutputTexture);
                cam->Select();
                if (mBaseMap) {
                    RndMat *work = TheShaderMgr.GetWork();
                    SetupMaterial(work, mBaseMap);
                    work->SetAlpha(1);
                    TheNgRnd.DrawRect(
                        Hmx::Rect(0, 0, mOutputTexture->Width(), mOutputTexture->Height()),
                        work,
                        (ShaderType)6,
                        Hmx::Color(1, 1, 1),
                        nullptr,
                        nullptr
                    );
                    mOutputTexState = 1;
                }
                std::sort(pairs1b0.begin(), pairs1b0.end(), BlendSorter());
                std::sort(pairs1c0.begin(), pairs1c0.end(), BlendSorter());
                if (mNearMap && !pairs1b0.empty()) {
                    mOutputTexState |= 2;
                    RndMat *work = TheShaderMgr.GetWork();
                    Transform xfm;
                    xfm.Reset();
                    TheShaderMgr.SetVConstant((VShaderConstant)4, Hmx::Matrix4(xfm));
                    TheShaderMgr.SetTransform(xfm);
                    SetupMaterial(work, mNearMap);
                    work->SetBlend(BaseMaterial::kBlendSrcAlpha);
                    float alpha = -1;
                    FOREACH (it, pairs1b0) {
                        RndTexBlendController *blendCtrlr = it->first;
                        float f17 = it->second;
                        if (f17 != alpha) {
                            work->SetAlpha(f17);
                            RndShader::SelectConfig(work, (ShaderType)0x16, false);
                            alpha = f17;
                        }
                        RndMesh *mesh = blendCtrlr->GetMesh();
                        if (mesh->IsSkinned()) {
                            MILO_NOTIFY_ONCE(
                                "%s: \"%s\" should not be a skinned mesh",
                                PathName(this),
                                mesh->Name()
                            );
                        }
                        mesh->DrawFacesInRange(0, -1);
                    }
                    work->SetAlpha(1);
                    if (RndCam::Current()) {
                        TheShaderMgr.SetVConstant(
                            (VShaderConstant)4, RndCam::Current()->ViewProj()
                        );
                    }
                }
                if (mFarMap && !pairs1c0.empty()) {
                    mOutputTexState |= 4;
                    RndMat *work = TheShaderMgr.GetWork();
                    Transform xfm;
                    xfm.Reset();
                    TheShaderMgr.SetVConstant((VShaderConstant)4, Hmx::Matrix4(xfm));
                    TheShaderMgr.SetTransform(xfm);
                    SetupMaterial(work, mFarMap);
                    work->SetBlend(BaseMaterial::kBlendSrcAlpha);
                    float alpha = -1;
                    FOREACH (it, pairs1c0) {
                        RndTexBlendController *blendCtrlr = it->first;
                        float f17 = it->second;
                        if (f17 != alpha) {
                            work->SetAlpha(f17);
                            RndShader::SelectConfig(work, (ShaderType)0x16, false);
                            alpha = f17;
                        }
                        RndMesh *mesh = blendCtrlr->GetMesh();
                        if (mesh->IsSkinned()) {
                            MILO_NOTIFY_ONCE(
                                "%s: \"%s\" should not be a skinned mesh",
                                PathName(this),
                                mesh->Name()
                            );
                        }
                        mesh->DrawFacesInRange(0, -1);
                    }
                    work->SetAlpha(1);
                    if (RndCam::Current()) {
                        TheShaderMgr.SetVConstant(
                            (VShaderConstant)4, RndCam::Current()->ViewProj()
                        );
                    }
                }
                DrawBlendList(pairs1d0, (TexState)8);
                cam->SetTargetTex(nullptr);
                RndCam::Current()->Select();
            }
        }
    }
}

#pragma endregion
#pragma region RndTexBlender

RndMat *RndTexBlender::SetupMaterial(RndMat *mat, RndTex *tex) {
    mat->SetZMode(BaseMaterial::kDisable);
    mat->SetBlend(RndMat::kBlendSrc);
    mat->SetCull(BaseMaterial::kCullNone);
    mat->SetTexWrap(BaseMaterial::kClamp);
    mat->SetDiffuseTex(tex);
    return mat;
}

void RndTexBlender::DrawBlendList(
    const std::vector<std::pair<RndTexBlendController *, float> > &pairs, TexState state
) {
    RndTex *which = state == 2 ? mNearMap : mFarMap;
    if ((which || (state == 8)) && !pairs.empty()) {
        mOutputTexState |= state;
        RndMat *work = TheShaderMgr.GetWork();
        Transform xfm;
        xfm.Reset();
        TheShaderMgr.SetVConstant((VShaderConstant)4, Hmx::Matrix4(xfm));
        TheShaderMgr.SetTransform(xfm);
        SetupMaterial(work, which);
        work->SetBlend(BaseMaterial::kBlendSrcAlpha);
        float f11 = -1;
        FOREACH (it, pairs) {
            RndTexBlendController *blendCtrlr = it->first;
            float f12 = it->second;
            if (state == 8) {
                work->SetDiffuseTex(blendCtrlr->GetOverrideMap());
            }
            if (f12 != f11 || state == 8) {
                work->SetAlpha(f12);
                RndShader::SelectConfig(work, kUnwrapUVShader, false);
                f11 = f12;
            }
            RndMesh *mesh = blendCtrlr->GetMesh();
            if (mesh->IsSkinned()) {
                MILO_NOTIFY_ONCE(
                    "%s: \"%s\" should not be a skinned mesh",
                    PathName(this),
                    mesh->Name()
                );
            }
            mesh->GeomOwner()->DrawFacesInRange(0, -1);
        }
        work->SetAlpha(1);
        if (RndCam::Current()) {
            TheShaderMgr.SetVConstant((VShaderConstant)4, RndCam::Current()->ViewProj());
        }
    }
}

DataNode RndTexBlender::OnGetRenderTextures(DataArray *) {
    return GetRenderTexturesNoZ(Dir());
}
