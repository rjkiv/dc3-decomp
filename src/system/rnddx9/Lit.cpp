#include "rnddx9/Lit.h"
#include "os/Memory.h"
#include "obj/Object.h"

DxLight::DxLight() {}

void DxLight::Init() {
    DxLight::Register();
    PhysMemTypeTracker tracker("D3D(phys):DxLight");
}

void DxLight::Terminate() {}
