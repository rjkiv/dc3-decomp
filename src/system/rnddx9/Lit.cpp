#include "rnddx9/Lit.h"
#include "os/Memory.h"
#include "obj/Object.h"

DxLight::DxLight() {}

void DxLight::Init() {
    REGISTER_OBJ_FACTORY(DxLight)
    PhysMemTypeTracker tracker("D3D(phys):DxLight");
}

void DxLight::Terminate() {}
