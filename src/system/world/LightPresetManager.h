#pragma once
#include "obj/Data.h"

class LightPreset;
class WorldDir;

class LightPresetManager {
public:
    LightPresetManager(WorldDir *);
    virtual DataNode Handle(DataArray *, bool);
    virtual ~LightPresetManager();

private:
    std::map<Symbol, std::vector<LightPreset *> > mPresets; // 0x4
    Symbol mLastCategory; // 0x1c
    WorldDir *mParent; // 0x20
    LightPreset *mPresetOverride; // 0x24
    LightPreset *mPresetNew; // 0x28
    LightPreset *mPresetPrev; // 0x2c
    float unk30; // 0x30
    float unk34; // 0x34
    float unk38; // 0x38
    bool unk3c; // 0x3c
    float mBlend; // 0x40
    float unk44; // 0x44
    int unk48; // 0x48
    bool mIgnoreLightingEvents; // 0x4c
};
