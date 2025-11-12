#pragma once
#include "obj/Object.h"
#include "rndobj/Trans.h"

class WorldDir;

class FreeCamera : public Hmx::Object {
public:
    FreeCamera(WorldDir *, float, float, int);
    virtual ~FreeCamera() {}
    virtual DataNode Handle(DataArray *, bool);

    void Poll();
    void SetParentDof(bool b1, bool b2, bool b3);
    void SetPadNum(int p) { mPadNum = p; }

protected:
    void UpdateFromCamera();

    RndTransformable *mParent; // 0x2c
    Vector3 mRot; // 0x30
    Transform mXfm; // 0x40
    float mFov; // 0x80
    bool mFrozen; // 0x84
    int mPadNum; // 0x88
    float mRotateRate; // 0x8c
    float mSlewRate; // 0x90
    float mFocalPlane; // 0x94
    bool mUseParentRotateX; // 0x98
    bool mUseParentRotateY; // 0x99
    bool mUseParentRotateZ; // 0x9a
    WorldDir *mWorld; // 0x9c
};
