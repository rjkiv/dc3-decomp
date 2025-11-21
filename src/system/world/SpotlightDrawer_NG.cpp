#include "world/SpotlightDrawer_NG.h"
#include "macros.h"
#include "math/Color.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Rnd.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"

NgSpotlightDrawer::NgSpotlightDrawer() : unk94(), unk98(this), unkac(0), unkb0(false) {
    unk94 = Hmx::Object::New<RndCam>();
}

NgSpotlightDrawer::~NgSpotlightDrawer() { RELEASE(unk94); }

void NgSpotlightDrawer::EndWorld() {
    if (SpotlightDrawer::sNeedDraw) {
    }
    SpotlightDrawer::EndWorld();
}

void NgSpotlightDrawer::DoPost() { RenderScene(); }

char const *NgSpotlightDrawer::GetProcType() { return "NgSpotlightDrawer"; }

void NgSpotlightDrawer::Init() {}

void NgSpotlightDrawer::RenderScene() {}

void NgSpotlightDrawer::SetAmbientColor(Hmx::Color const &) {}

void NgSpotlightDrawer::ClearPostDraw() {}

void NgSpotlightDrawer::ClearPostProc() {}

int NgSpotlightDrawer::RTWidth() { return 1; }

int NgSpotlightDrawer::RTHeight() { return 1; }

bool NgSpotlightDrawer::CheckSharedResources() { return false; }

bool NgSpotlightDrawer::RestoreCam() {
    if (unk98) {
        unk98 = TheRnd.GetDefaultCam();
    }
    return true;
}

bool NgSpotlightDrawer::CheckFogTexture() { return false; }

void NgSpotlightDrawer::SetXSectionTexture(Spotlight::BeamDef const &) {}

void NgSpotlightDrawer::SetupFogDensityMap() {}

void NgSpotlightDrawer::RenderFogProxy() {}

void NgSpotlightDrawer::RenderSphere(Spotlight *) {}

void NgSpotlightDrawer::RenderSheet(Spotlight *) {}

void NgSpotlightDrawer::SetupXSection(Spotlight *, Spotlight::BeamDef const &) {}

void NgSpotlightDrawer::RenderConeDefs(Spotlight *, Hmx::Color const &) {}

void NgSpotlightDrawer::SetupFogDensityState() {}

void NgSpotlightDrawer::RenderCone(Spotlight *sl) {}

void NgSpotlightDrawer::RenderBeams(Hmx::Matrix4 const &) {}

bool NgSpotlightDrawer::CheckCam() { return false; }

void NgSpotlightDrawer::BlurRT(float, float) {}

void NgSpotlightDrawer::BlurRT() {}

void NgSpotlightDrawer::SetupForPostProcess() {}
