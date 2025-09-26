#include "world/FreeCamera.h"
#include "obj/Object.h"
#include "math/Rot.h"
#include "rndobj/Cam.h"
#include "rndobj/DOFProc.h"

float gUnitsPerMeter = 39.370079;

FreeCamera::FreeCamera(WorldDir *dir, float f1, float f2, int i)
    : mParent(0), mFrozen(0), mPadNum(i), mRotateRate(f1), mSlewRate(gUnitsPerMeter * f2),
      mUseParentRotateX(1), mUseParentRotateY(1), mUseParentRotateZ(1), mWorld(dir) {
    UpdateFromCamera();
}

BEGIN_HANDLERS(FreeCamera)
    HANDLE_ACTION(set_parent, mParent = _msg->Obj<RndTransformable>(2))
    HANDLE_ACTION(set_pos, mXfm.v.Set(_msg->Float(2), _msg->Float(3), _msg->Float(4)))
    HANDLE_ACTION(
        set_rot,
        mRot.Set(
            _msg->Float(2) * DEG2RAD, _msg->Float(3) * DEG2RAD, _msg->Float(4) * DEG2RAD
        )
    )
    HANDLE_ACTION(set_parent_dof, SetParentDof(_msg->Int(2), _msg->Int(3), _msg->Int(4)))
    HANDLE_ACTION(set_frozen, mFrozen = _msg->Int(2))
END_HANDLERS

void FreeCamera::SetParentDof(bool b1, bool b2, bool b3) {
    mUseParentRotateX = b1;
    mUseParentRotateY = b2;
    mUseParentRotateZ = b3;
}

// void FreeCamera::UpdateFromCamera() {
//     RndCam *cam = mWorld->GetCam();
//     mFov = cam->YFov();
//     mXfm = cam->WorldXfm();
//     MakeEuler(mXfm.m, mRot);
//     mParent = 0;
//     mFocalPlane = TheDOFProc->FocalPlane();
// }
