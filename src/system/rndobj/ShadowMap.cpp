#include "rndobj/ShadowMap.h"
#include "math/Color.h"
#include "math/Vec.inl"
#include "os/Debug.h"
#include "os/Memory.h"
#include "macros.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Cam.h"
#include "rndobj/Env.h"
#include "rndobj/Lit.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"

RndCam *RndShadowMap::sLightCam;
RndTex *RndShadowMap::sShadowTex;

void RndShadowMap::Init() {
    PhysMemTypeTracker tracker("D3D(phys):Global");
    delete sLightCam;
    sLightCam = Hmx::Object::New<RndCam>();
    delete sShadowTex;
    sShadowTex = Hmx::Object::New<RndTex>();
    sShadowTex->SetBitmap(512, 512, 32, RndTex::kShadowMap, false, nullptr);
    sLightCam->SetTargetTex(sShadowTex);
}

void RndShadowMap::Terminate() {
    RELEASE(sLightCam);
    RELEASE(sShadowTex);
}

void RndShadowMap::EndShadow() { TheRnd.SetShadowMap(nullptr, nullptr, nullptr); }

bool RndShadowMap::PrepShadow(RndDrawable *draw, RndEnviron *env) {
    if (GetGfxMode() == kNewGfx && sLightCam && sShadowTex) {
        RndEnviron *envToUse = env ? env : RndEnviron::Current();
        RndLight *lit = nullptr;
        FOREACH (it, envToUse->LightsApprox()) {
            if ((*it)->GetType() == RndLight::kFloorSpot) {
                lit = *it;
                break;
            }
        }
        if (!lit) {
            FOREACH (it, envToUse->LightsReal()) {
                if ((*it)->GetType() == RndLight::kDirectional
                    || (*it)->GetType() == RndLight::kPoint) {
                    lit = *it;
                    break;
                }
            }
        }
        if (!lit) {
            return false;
        }
        RndCam *curCam = RndCam::Current();
        Sphere s;
        if (!draw->MakeWorldSphere(s, false)) {
            MILO_NOTIFY_ONCE(
                "Can't self-shadow %s; MakeWorldSphere failed.", PathName(draw)
            );
            return false;
        } else {
            Transform xfm(lit->WorldXfm().m, s.center);
            if (lit->GetType() == RndLight::kPoint) {
                Subtract(s.center, lit->WorldXfm().v, xfm.m.y);
                Normalize(xfm.m, xfm.m);
            }
            float f3 = s.radius / tanf(0.39269909262657166);
            float f13 = s.radius + f3;
            float f12 = f3 - s.radius;
            Vector3 tmp;
            Multiply(xfm.m, Vector3(0, -f3, 0), tmp);
            Add(xfm.v, tmp, xfm.v);
            sLightCam->SetWorldXfm(xfm);
            sLightCam->SetFrustum(f12, f13, 0.7853982f, 1);
            sLightCam->Select();
            Rnd::Mode drawMode = TheRnd.DrawMode();
            TheRnd.SetDrawMode(Rnd::kDrawExtrude);
            draw->DrawShowing();
            TheRnd.SetDrawMode(drawMode);
            curCam->Select();
            static Hmx::Color sColor(0, 0, 0, 0);
            Hmx::Color colorToUse =
                lit->GetType() == RndLight::kFloorSpot ? lit->GetColor() : sColor;
            colorToUse.Set(1 - colorToUse.red, 1 - colorToUse.green, 1 - colorToUse.blue);
            TheRnd.SetShadowMap(sShadowTex, sLightCam, &colorToUse);
            return true;
        }
    }
    return false;
}
