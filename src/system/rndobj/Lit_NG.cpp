#include "rndobj/Lit_NG.h"
#include "Lit.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Draw.h"
#include "Memory.h"
#include "rndobj/Lit.h"

bool NgLight::WantShadows() const {
    return GetGfxMode() == kNewGfx && mShadowOverride && !mShadowOverride->empty();
}

bool NgLight::HaveShadows(std::vector<RndDrawable *> &draws) {
    MILO_ASSERT(mShadowOverride && !mShadowOverride->empty(), 0x3D);
    for (ObjPtrList<RndDrawable>::iterator it = mShadowOverride->begin();
         it != mShadowOverride->end();
         ++it) {
        RndDrawable *cur = *it;
        Sphere s;
        if (!cur->MakeWorldSphere(s, false) || SphereConeTest(s.center, s.radius)) {
            draws.push_back(cur);
        }
    }
    return !draws.empty();
}

BEGIN_COPYS(NgLight)
    COPY_SUPERCLASS(RndLight)
    CheckShadowMap();
END_COPYS

BEGIN_LOADS(NgLight)
    RndLight::Load(bs);
    CheckShadowMap();
END_LOADS

NgLight::~NgLight() {
    RELEASE(mShadowRT);
    RELEASE(unk188);
}

NgLight::NgLight() : mShadowRT(0), mShadowMap(0), unk188(0), unk18c(-1) {}

void NgLight::Init() {
    REGISTER_OBJ_FACTORY(NgLight);
    PhysMemTypeTracker tracker("D3D(phys):NgLight");
}
