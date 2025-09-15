#include "world/LightPreset.h"
#include "LightPreset.h"

LightPreset::LightPreset()
    : mKeyframes(this), mSpotlights(this, (EraseMode)0, kObjListOwnerControl),
      mEnvironments(this, (EraseMode)0, kObjListOwnerControl),
      mLights(this, (EraseMode)0, kObjListOwnerControl),
      mSpotlightDrawers(this, (EraseMode)0, kObjListOwnerControl), unk90(0), unk98(0),
      unk9c(this), unkb0(0), unkb4(this), unke8(0), unkec(-1), unkf0(0), unkf4(0),
      unkf8(0), unkfc(-1), unk100(0), unk104(0), unk108(0), unk10c(0) {}

LightPreset::~LightPreset() { Clear(); }
