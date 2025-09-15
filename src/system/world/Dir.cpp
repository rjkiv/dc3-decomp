#include "world/Dir.h"
#include "Dir.h"

WorldDir::WorldDir()
    : mPresetOverrides(this), mBitmapOverrides(this), mMatOverrides(this), unk28c(this),
      unk2a0(this), unk2b4(this), unk2c8(this), unk2e4(0), unk2e8(0), unk2ec(this),
      unk300(this), unk314(0), m3DSoundMgr(this), mLightPresetMgr(this), unk3dc(0),
      unk3e0(0), unk3e1(0), unk3f4(0), unk3f8(this), unk40c(this), unk420(10), unk424(1) {
    ClearDeltas();
}

WorldDir::~WorldDir() {}

void WorldDir::ClearDeltas() {
    for (int i = 0; i < 4; i++)
        mDeltaSincePoll[i] = 0;
}
