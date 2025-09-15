#include "world/LightPresetManager.h"

LightPresetManager::LightPresetManager(WorldDir *dir)
    : mParent(dir), mPresetOverride(0), mPresetNew(0), mPresetPrev(0), unk30(0), unk34(0),
      unk38(0), unk3c(0), mBlend(1.0f), unk44(0), unk48(0), mIgnoreLightingEvents(0) {
    MILO_ASSERT(mParent, 0x22);
}

LightPresetManager::~LightPresetManager() {}
