#pragma once
#include "math/Mtx.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "synth/ThreeDSound.h"

class WorldDir;

class ThreeDSoundManager {
public:
    ThreeDSoundManager(WorldDir *);
    ~ThreeDSoundManager();

    void SyncObjects();
    void Poll();

    static void HarvestSounds(ObjectDir *, ObjPtrList<ThreeDSound> &);

private:
    void CalculateDistance(ThreeDSound *, const Transform &, float &, float &);
    float CalculateAngle(ThreeDSound *, const Transform &);
    float CalculateDoppler(ThreeDSound *, const Transform &, float, float, float);

    WorldDir *mParent; // 0x0
    ObjPtrList<ThreeDSound> unk4;
    Transform unk18;
    ObjPtr<RndTransformable> unk58;
    bool unk6c;
    float unk70;
};
