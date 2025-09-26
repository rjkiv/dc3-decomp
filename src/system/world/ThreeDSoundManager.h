#pragma once
#include "math/Mtx.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "synth/ThreeDSound.h"

class WorldDir;

class ThreeDSoundManager {
    friend class WorldDir;

public:
    ThreeDSoundManager(WorldDir *);
    ~ThreeDSoundManager();

    void SyncObjects();
    void Poll();
    void SetListener(RndTransformable *t) {
        mListener = t;
        unk6c = true;
    }

    static void HarvestSounds(ObjectDir *, ObjPtrList<ThreeDSound> &);

private:
    void CalculateDistance(ThreeDSound *, const Transform &, float &, float &);
    float CalculateAngle(ThreeDSound *, const Transform &);
    float CalculateDoppler(ThreeDSound *, const Transform &, float, float, float);

    WorldDir *mParent; // 0x0
    ObjPtrList<ThreeDSound> mSounds; // 0x4
    Transform unk18; // 0x18
    /** "Position of the listener for 3D sounds.  None defaults to the current camera." */
    ObjPtr<RndTransformable> mListener; // 0x58
    bool unk6c; // 0x6c
    /** "Change in frequency due to doppler shift is raised to this power.
        0 is no doppler shift, 0.5 is half normal shift, 2 is twice normal, etc." */
    float mDopplerPower; // 0x70
};
