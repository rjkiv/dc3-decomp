#include "world/CameraManager.h"
#include "macros.h"
#include "math/Rand.h"
#include "obj/Dir.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "rndobj/DOFProc.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "world/FreeCamera.h"

Rand CameraManager::sRand(0);
int CameraManager::sSeed;

CameraManager::CameraManager()
    : mParent(nullptr), mNextShot(this), mBlendTime(0), unk58(true), mCurrentShot(this),
      mCamStartTime(0), mFreeCam(nullptr), unk78(this) {}

CameraManager::CameraManager(WorldDir *parent)
    : mParent(parent), mNextShot(this), mBlendTime(0), unk58(true), mCurrentShot(this),
      mCamStartTime(0), mFreeCam(nullptr), unk78(this) {
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

BEGIN_COPYS(CameraManager)
    COPY_SUPERCLASS(Hmx::Object)

END_COPYS

BEGIN_LOADS(CameraManager)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mNextShot;
END_LOADS

BEGIN_PROPSYNCS(CameraManager)
END_PROPSYNCS

void CameraManager::Enter() {
    unk58 = true;
    mBlendTime = 0.0f;
    StartShot_(0);
    DeleteFreeCam();
}

void CameraManager::ForceCamShot(CamShot *shot) {
    unk58 = true;
    mNextShot = shot;
}

float CameraManager::CalcFrame() {
    float ttime = TheTaskMgr.Time(mCurrentShot->Units()) - mCamStartTime;
    ttime *= mCurrentShot->FramesPerUnit();
    return ttime;
}

CamShot *CameraManager::MiloCamera() {
    if (TheLoadMgr.EditMode()) {
        static DataNode &anim = DataVariable("milo.anim");
        if (anim.Type() == kDataObject) {
            return anim.Obj<CamShot>();
        }
    }
    return nullptr;
}

FreeCamera *CameraManager::GetFreeCam(int padnum) {
    if (!mParent) {
        MILO_NOTIFY("%s can\'t make free cam without parent", PathName(this));
        return nullptr;
    }
    if (!mFreeCam) {
        mFreeCam = new FreeCamera(mParent, 0.001f, 0.05f, 0);
        mFreeCam->SetPadNum(padnum);
    }
    return mFreeCam;
}

void CameraManager::DeleteFreeCam() { RELEASE(mFreeCam); }

void CameraManager::ForceCameraShot(CamShot *shot, bool b) {
    unk58 = (shot != mNextShot || b) || unk58;
    mNextShot = shot;
}

void CameraManager::FirstShotOk(Symbol s) {
    static Message first_shot_ok("first_shot_ok", "");
    first_shot_ok[0] = s;
    HandleType(first_shot_ok);
}

void CameraManager::StartShot_(CamShot *shot) {
    if (mCurrentShot)
        mCurrentShot->EndAnim();

    if (TheDOFProc && !shot && mCurrentShot) {
        TheDOFProc->UnSet();
    }

    mCurrentShot = shot;
    if (mCurrentShot) {
        mCurrentShot->StartAnim();
        mCamStartTime = TheTaskMgr.Time(shot->Units());
        unk54 = 0.0f;
    }
}

void CameraManager::RandomizeCategory(ObjPtrList<CamShot> &camlist) {}

void CameraManager::PrePoll() {
    if (!MiloCamera()) {
        if (unk58) {
            StartShot_(mNextShot);
            unk58 = false;
        }
        if (mCurrentShot) {
            mCurrentShot->SetPreFrame(CalcFrame(), 1.0f);
        }
    }
}

CamShot *CameraManager::ShotAfter(CamShot *cshot) {
    ObjDirItr<CamShot> it((ObjectDir *)mParent, true);
    CamShot *ret = it;
    for (; it != 0 && it != cshot; ++it)
        ;
    if (it)
        ++it;
    if (!it)
        return ret;
    else
        return it;
}

DataNode CameraManager::OnCycleShot(DataArray *da) {
    CamShot *after = ShotAfter(mCurrentShot);
    if (after)
        ForceCameraShot(after, true);
    return 0;
}

Symbol CameraManager::MakeCategoryAndFilters(
    DataArray *da, std::vector<PropertyFilter> &filts, float *f
) {
    static Symbol flags_exact("flags_exact");
    static Symbol flags_any("flags_any");
    Symbol sym = da->Sym(2);
    if (da->Size() > 3) {
        DataArray *arr = da->Array(3);
        for (uint i = 0; i != arr->Size(); i++) {
            DataArray *currArr = arr->Array(i);
            PropertyFilter filt;
            filt.prop = currArr->Evaluate(0);
            bool b1 = false;
            if (filt.prop.Type() == kDataSymbol) {
                if (filt.prop.Sym() == flags_exact) {
                    b1 = true;
                }
            }
            if (b1) {
                filt.mask = currArr->Int(1);
                filt.match = currArr->Int(2);
            } else {
                b1 = false;
                if (filt.prop.Type() == kDataSymbol) {
                    if (filt.prop.Sym() == flags_any) {
                        b1 = true;
                    }
                }
                if (b1) {
                    filt.mask = currArr->Int(1);
                    filt.match = 1;
                } else {
                    filt.match = currArr->Evaluate(1);
                    filt.mask = -1;
                }
            }
            filts.push_back(filt);
        }
    }
    return sym;
}
