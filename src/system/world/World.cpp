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
    REGISTER_OBJ_FACTORY(BeatClock)
    REGISTER_OBJ_FACTORY(CameraManager)
    REGISTER_OBJ_FACTORY(ColorPalette)
    REGISTER_OBJ_FACTORY(WorldCrowd)
    REGISTER_OBJ_FACTORY(WorldCrowd3DCharHandle)
    CamShot::Init();
    REGISTER_OBJ_FACTORY(WorldReflection)
    Spotlight::Init();
    REGISTER_OBJ_FACTORY(LightPreset)
    REGISTER_OBJ_FACTORY(LightHue)
    SpotlightDrawer::Init();
    REGISTER_OBJ_FACTORY(SpotlightEnder)
    REGISTER_OBJ_FACTORY(WorldInstance)
    REGISTER_OBJ_FACTORY(PhysicsVolume)
    REGISTER_OBJ_FACTORY(PostProcer)
    NgSpotlightDrawer::Init();
    PreloadSharedSubdirs("world");
}
