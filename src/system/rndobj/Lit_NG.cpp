#include "rndobj/Lit_NG.h"
#include "Lit.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "os/Memory.h"
#include "rndobj/Lit.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/ShaderMgr.h"

NgLight::NgLight() : mShadowRT(0), mShadowMap(0), unk188(0), unk18c(-1) {}

NgLight::~NgLight() {
    RELEASE(mShadowRT);
    RELEASE(unk188);
}

BEGIN_COPYS(NgLight)
    COPY_SUPERCLASS(RndLight)
    CheckShadowMap();
END_COPYS

BEGIN_LOADS(NgLight)
    RndLight::Load(bs);
    CheckShadowMap();
END_LOADS

void NgLight::RenderShadows(std::vector<RndDrawable *> &shadowCasters) {
    MILO_ASSERT(mShadowRT && !shadowCasters.empty(), 0x112);
    MILO_ASSERT(WantShadows(), 0x113);

    RndCam *cur = RndCam::Current();
    mShadowRT->MakeDrawTarget();
    SetAndClearShadowViewport();
    SetShadowTransforms();
    Rnd::Mode old = TheRnd.DrawMode();
    TheRnd.SetDrawMode(Rnd::kDrawOcclusion);
    auto it = shadowCasters.begin();
    auto itEnd = shadowCasters.end();
    for (; it != itEnd; ++it) {
        if ((*it)->Showing()) {
            (*it)->DrawShowing();
        }
    }
    TheRnd.SetDrawMode(old);
    mShadowRT->FinishDrawTarget();
    BlurShadowRT();
    if (cur) {
        cur->Select();
    } else {
        TheRnd.GetDefaultCam()->Select();
    }
}

void NgLight::SetAndClearShadowViewport() {
    TheNgRnd.SetViewport(
        NgRnd::Viewport(0, 0, mShadowRT->Width(), mShadowRT->Height(), 0, 1)
    );
    TheNgRnd.Clear(1, Hmx::Color(0, 0, 0, 0));
}

bool NgLight::WantShadows() const {
    return GetGfxMode() == kNewGfx && mShadowOverride && !mShadowOverride->empty();
}

bool NgLight::HaveShadows(std::vector<RndDrawable *> &draws) {
    MILO_ASSERT(mShadowOverride && !mShadowOverride->empty(), 0x3D);
    FOREACH_PTR (it, mShadowOverride) {
        RndDrawable *cur = *it;
        Sphere s;
        if (!cur->MakeWorldSphere(s, false) || SphereConeTest(s.center, s.radius)) {
            draws.push_back(cur);
        }
    }
    return !draws.empty();
}

RndTex *NgLight::CreateShadowTex() {
    PhysMemTypeTracker t("D3D(phys): Shadow Map");
    RndTex *tex = Hmx::Object::New<RndTex>();
    tex->SetBitmap(0x100, 0x100, 0x10, RndTex::kRenderedNoZ, false, nullptr);
    return tex;
}

void NgLight::CheckShadowMap() {
    if (TheRnd.Drawing() && TheShaderMgr.AllowPerPixel()) {
        if (mType == kFakeSpot) {
            if (TheRnd.GetDrawCount() != unk18c) {
                bool b3 = !mShadowOverride && !mShadowObjects.empty();
                if (b3) {
                    mShadowOverride = &mShadowObjects;
                }
                if (WantShadows()) {
                    if (!mShadowRT && !unk188) {
                        mShadowRT = CreateShadowTex();
                        unk188 = CreateShadowTex();
                    }
                    std::vector<RndDrawable *> draws;
                    if (HaveShadows(draws)) {
                        MILO_ASSERT(mShadowRT, 0x81);
                        RenderShadows(draws);
                        mShadowMap = mShadowRT;
                    } else {
                        mShadowMap =
                            TheRnd.GetDefaultTex(Rnd::kDefaultTex_WhiteTransparent);
                        MILO_ASSERT(mShadowMap, 0x8A);
                    }
                } else {
                    mShadowMap = TheRnd.GetDefaultTex(Rnd::kDefaultTex_WhiteTransparent);
                    MILO_ASSERT(mShadowMap, 0x91);
                }
                unk18c = TheRnd.GetDrawCount();
                if (b3) {
                    mShadowOverride = nullptr;
                }
            }
        } else {
            RELEASE(mShadowMap);
        }
    }
}

void NgLight::SetShadowTransforms() {
    static Transform sXfm(Hmx::Matrix3(1, 0, 0, 0, 0, 1, 0, 1, 0), Vector3(0, 0, 0));
    Transform tf130;
    Invert(WorldXfm(), tf130);
    Transform tf170;
    Multiply(tf130, sXfm, tf170);
    Hmx::Matrix4 mtx4;
    mtx4.m[0].x = 1;
    mtx4.m[1].x = 0;
    mtx4.m[2].x = 0;
    mtx4.m[3].x = 0;
    mtx4.m[0].y = 0;
    mtx4.m[1].y = 1;
    mtx4.m[2].y = 0;
    mtx4.m[3].y = 0;
    mtx4.m[0].z = 0;
    mtx4.m[1].z = 0;
    mtx4.m[2].z = 1 / mRange;
    mtx4.m[3].z = 0;
    mtx4.m[0].w = 0;
    mtx4.m[1].w = 0;
    mtx4.m[2].w = (mBotRadius - mTopRadius) / mRange;
    mtx4.m[3].w = mTopRadius;
    Hmx::Matrix4 mtxb0 = tf170 * mtx4;
    Transform tff0;
    Invert(tf170, tff0);
    TheShaderMgr.SetVConstant((VShaderConstant)4, mtxb0);
    TheShaderMgr.SetVConstant((VShaderConstant)0x10, Hmx::Matrix4(tff0));
}

void NgLight::Init() {
    REGISTER_OBJ_FACTORY(NgLight);
    PhysMemTypeTracker tracker("D3D(phys):NgLight");
}

void NgLight::Terminate() {}
