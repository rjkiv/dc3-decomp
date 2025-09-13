#include "world/CameraManager.h"
#include "world/CameraShot.h"
#include "obj/Object.h"
#include "os/Debug.h"

CameraManager::CameraManager()
    : mParent(nullptr), mNextShot(this), mBlendTime(0), unk58(true), mCurrentShot(this),
      unk70(0), mFreeCam(nullptr), unk78(this) {}

CameraManager::CameraManager(WorldDir *parent)
    : mParent(parent), mNextShot(this), mBlendTime(0), unk58(true), mCurrentShot(this),
      unk70(0), mFreeCam(nullptr), unk78(this) {
    MILO_ASSERT(mParent, 0x34);
}

CameraManager::~CameraManager() {
    StartShot_(nullptr);
    RELEASE(mFreeCam);
    for (std::vector<Category>::iterator it = mCameraShotCategories.begin();
         it != mCameraShotCategories.end();
         ++it) {
        delete it->unk4;
    }
}

BEGIN_HANDLERS(CameraManager)
    HANDLE(pick_shot, OnPickCameraShot)
    HANDLE(find_shot, OnFindCameraShot)
    HANDLE_ACTION(force_shot, ForceCamShot(_msg->Obj<CamShot>(2)))
    HANDLE_EXPR(current_shot, CurrentShot())
    HANDLE_EXPR(next_shot, NextShot())
    HANDLE_EXPR(get_free_cam, GetFreeCam(_msg->Int(2)))
    HANDLE_EXPR(has_free_cam, HasFreeCam())
    HANDLE_ACTION(delete_free_cam, DeleteFreeCam())
    HANDLE(cycle_shot, OnCycleShot)
    HANDLE_EXPR(shot_after, ShotAfter(_msg->Obj<CamShot>(2)))
    HANDLE(camera_random_seed, OnRandomSeed)
    HANDLE(iterate_shot, OnIterateShot)
    HANDLE(num_shots, OnNumCameraShots)
    HANDLE(get_shot_list, OnGetShotList)
    HANDLE_ACTION(reset_camshots, StartShot_(nullptr))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_SAVES(CameraManager)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mNextShot;
END_SAVES

void CameraManager::ForceCamShot(CamShot *shot) {
    unk58 = true;
    mNextShot = shot;
}
