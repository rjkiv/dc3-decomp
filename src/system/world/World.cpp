#include "world/World.h"
#include "BeatClock.h"
#include "CameraManager.h"
#include "Crowd.h"
#include "Dir.h"
#include "PhysicsVolume.h"
#include "SpotlightDrawer.h"
#include "obj/Object.h"
#include "world/ColorPalette.h"
#include "world/Crowd3DCharHandle.h"
#include "world/Instance.h"
#include "world/LightHue.h"
#include "world/LightPreset.h"
#include "world/PostProcer.h"
#include "world/Reflection.h"
#include "world/Spotlight.h"
#include "world/SpotlightEnder.h"
#include "world/SpotlightDrawer_NG.h"

void WorldInit() {
    WorldDir::Init();
    BeatClock::Register();
    CameraManager::Register();
    ColorPalette::Register();
    WorldCrowd::Register();
    WorldCrowd3DCharHandle::Register();
    CamShot::Init();
    WorldReflection::Register();
    Spotlight::Init();
    LightPreset::Register();
    LightHue::Register();
    SpotlightDrawer::Init();
    SpotlightEnder::Register();
    WorldInstance::Register();
    PhysicsVolume::Register();
    PostProcer::Register();
    NgSpotlightDrawer::Init();
    PreloadSharedSubdirs("world");
}
