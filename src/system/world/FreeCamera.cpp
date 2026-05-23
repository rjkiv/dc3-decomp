#include "world/FreeCamera.h"
#include "math/Mtx.h"
#include "math/Trig.h"
#include "math/Vec.h"
#include "obj/Task.h"
#include "os/Joypad.h"
#include "rndobj/Trans.h"
#include "world/Dir.h"
#include "obj/Object.h"
#include "math/Rot.h"
#include "rndobj/Cam.h"
#include "rndobj/DOFProc.h"

float gUnitsPerMeter = 39.370079;

FreeCamera::FreeCamera(WorldDir *world, float rotateRate, float slewRate, int padNum)
    : mParent(0), mFrozen(0), mPadNum(padNum), mRotateRate(rotateRate),
      mSlewRate(gUnitsPerMeter * slewRate), mUseParentRotateX(1), mUseParentRotateY(1),
      mUseParentRotateZ(1), mWorld(world) {
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

void FreeCamera::UpdateFromCamera() {
    RndCam *cam = mWorld->Cam();
    mFov = cam->YFov();
    mXfm = cam->WorldXfm();
    MakeEuler(mXfm.m, mRot);
    mParent = nullptr;
    mFocalPlane = TheDOFProc->FocalPlane();
}

void FreeCamera::Poll() {
    JoypadData *jpd = JoypadGetPadData(mPadNum);
    if (jpd) {
        float deltaMs = TheTaskMgr.DeltaUISeconds() * 1000;
        if (mFrozen) {
            deltaMs = 0;
        }
        float lx = jpd->LX();
        float ly = jpd->LY();
        float rotateRate = mRotateRate * deltaMs;
        float left0rate = -fabsf(lx) * rotateRate * lx;
        mRot.z = LimitAng(mRot.z + left0rate);
        float left1rate = fabsf(ly) * rotateRate * ly;
        mRot.x = LimitAng(left1rate + mRot.x);
        MakeRotMatrix(mRot, mXfm.m, true);
        float f21 = mSlewRate * deltaMs;
        if (jpd->Pressed(kPad_Xbox_LT)) {
            f21 /= 10;
        }
        float rx = jpd->RX();
        float ry = jpd->RY();
        float r0rate = fabsf(rx * rx) * rx * f21;
        f21 = -fabsf(ry * ry) * ry * f21;
        ScaleAddEq(mXfm.v, mXfm.m.x, r0rate / 2);
        if (jpd->Pressed(kPad_Xbox_LB)) {
            ScaleAddEq(mXfm.v, mXfm.m.z, f21);
        } else {
            ScaleAddEq(mXfm.v, mXfm.m.y, f21);
        }
        RndCam *cam = mWorld->Cam();
        if (jpd->Pressed(kPad_DUp)) {
            mFov += 0.001f;
        } else if (jpd->Pressed(kPad_DDown)) {
            mFov -= 0.001f;
        }
        if (jpd->Pressed(kPad_Xbox_A)) {
            if (jpd->Pressed(kPad_DLeft)) {
                mRot.y += deltaMs / 1000;
            } else if (jpd->Pressed(kPad_DRight)) {
                mRot.y -= deltaMs / 1000;
            }
        } else {
            if (jpd->Pressed(kPad_DLeft)) {
                mFocalPlane /= powf(2, deltaMs / 1000);
            } else if (jpd->Pressed(kPad_DRight)) {
                mFocalPlane *= powf(2, deltaMs / 1000);
            }
        }
        Transform tf170;
        if (mParent) {
            if (mUseParentRotateX && mUseParentRotateY && mUseParentRotateZ) {
                Multiply(mXfm, mParent->WorldXfm(), tf170);
            } else {
                Hmx::Matrix3 mc0 = mParent->WorldXfm().m;
                Vector3 v180(0, 0, 0);
                MakeEuler(mc0, v180);
                if (!mUseParentRotateX) {
                    v180.x = mRot.x;
                }
                if (!mUseParentRotateY) {
                    v180.y = mRot.y;
                }
                if (!mUseParentRotateZ) {
                    v180.z = mRot.z;
                }
                Hmx::Matrix3 mf0;
                MakeRotMatrix(v180, mf0, false);
                Multiply(mXfm, Transform(mf0, mParent->WorldXfm().v), tf170);
            }
        } else {
            tf170 = mXfm;
        }
        cam->SetFrustum(cam->NearPlane(), cam->FarPlane(), mFov, 1);
        if (cam->TransParent()) {
            Transform tf90;
            Invert(cam->TransParent()->WorldXfm(), tf90);
            Multiply(tf170, tf90, tf170);
        }
        cam->SetLocalXfm(tf170);
        if (TheDOFProc->Enabled()) {
            TheDOFProc->Set(
                cam,
                mFocalPlane,
                TheDOFProc->BlurDepth(),
                TheDOFProc->MaxBlur(),
                TheDOFProc->MinBlur()
            );
        }
    }
}
