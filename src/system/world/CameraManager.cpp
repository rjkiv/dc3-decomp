#include "world/CameraManager.h"
#include "macros.h"
#include "math/Mtx.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/DOFProc.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "world/Crowd.h"
#include "world/Dir.h"
#include "world/FreeCamera.h"

Rand CameraManager::sRand(0);
int CameraManager::sSeed;

CameraManager::CameraManager()
    : mParent(nullptr), mNextShot(this), mBlendTime(0), mNextShotChanged(true),
      mCurrentCam(this), mCamStartTime(0), mFreeCam(nullptr), mCrowds(this) {}

CameraManager::CameraManager(WorldDir *parent)
    : mParent(parent), mNextShot(this), mBlendTime(0), mNextShotChanged(true),
      mCurrentCam(this), mCamStartTime(0), mFreeCam(nullptr), mCrowds(this) {
    MILO_ASSERT(mParent, 0x34);
}

CameraManager::~CameraManager() {
    StartShot_(nullptr);
    RELEASE(mFreeCam);
    FOREACH (it, mCategories) {
        delete it->mShots;
    }
}

BEGIN_HANDLERS(CameraManager)
    HANDLE(pick_shot, OnPickCameraShot)
    HANDLE(find_shot, OnFindCameraShot)
    HANDLE_ACTION(force_shot, ForceCamShot(_msg->Obj<CamShot>(2)))
    HANDLE_EXPR(current_shot, GetCurrentShot())
    HANDLE_EXPR(next_shot, GetNextShot())
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

BEGIN_PROPSYNCS(CameraManager)
    SYNC_PROP_SET(next_shot, mNextShot.Ptr(), ForceCameraShot(_val.Obj<CamShot>(), false))
    SYNC_PROP(blend_time, mBlendTime)
    SYNC_PROP(parent, mParent)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CameraManager)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mNextShot;
END_SAVES

BEGIN_COPYS(CameraManager)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CameraManager)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNextShot)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(CameraManager)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mNextShot;
END_LOADS

void CameraManager::Enter() {
    mNextShotChanged = true;
    mBlendTime = 0.0f;
    StartShot_(0);
    DeleteFreeCam();
}

void CameraManager::ForceCamShot(CamShot *shot) {
    mNextShotChanged = true;
    mNextShot = shot;
}

float CameraManager::CalcFrame() {
    float ttime = TheTaskMgr.Time(mCurrentCam->Units()) - mCamStartTime;
    ttime *= mCurrentCam->FramesPerUnit();
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
        MILO_NOTIFY("%s can't make free cam without parent", PathName(this));
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
    mNextShotChanged = (shot != mNextShot || b) || mNextShotChanged;
    mNextShot = shot;
}

void CameraManager::FirstShotOk(Symbol s) {
    static Message first_shot_ok("first_shot_ok", "");
    first_shot_ok[0] = s;
    HandleType(first_shot_ok);
}

void CameraManager::StartShot_(CamShot *shot) {
    if (mCurrentCam)
        mCurrentCam->EndAnim();

    if (TheDOFProc && !shot && mCurrentCam) {
        TheDOFProc->UnSet();
    }

    mCurrentCam = shot;
    if (mCurrentCam) {
        mCurrentCam->StartAnim();
        mCamStartTime = TheTaskMgr.Time(shot->Units());
        mLastBlend = 0.0f;
    }
}

struct NameSort {
    bool operator()(CamShot *o1, CamShot *o2) const {
        return strcmp(o1->Name(), o2->Name()) < 0;
    }
};

void CameraManager::RandomizeCategory(ObjPtrList<CamShot> &camlist) {
    std::vector<CamShot *> camshots;
    {
        MemDoTempAllocations m;
        camshots.resize(camlist.size());
    }
    int idx = 0;
    FOREACH (it, camlist) {
        camshots[idx++] = *it;
    }
    std::sort(camshots.begin(), camshots.end(), NameSort());
    for (int i = 0; i < camshots.size(); i++) {
        int randIdx = sRand.Int(i, camshots.size());
        std::swap(camshots[i], camshots[randIdx]);
    }
    camlist.clear();
    for (int i = 0; i < idx; i++) {
        camlist.push_back(camshots[i]);
    }
}

void CameraManager::PrePoll() {
    if (!MiloCamera()) {
        if (mNextShotChanged) {
            StartShot_(mNextShot);
            mNextShotChanged = false;
        }
        if (mCurrentCam) {
            mCurrentCam->SetPreFrame(CalcFrame(), 1.0f);
        }
    }
}

CamShot *CameraManager::ShotAfter(CamShot *cshot) {
    ObjDirItr<CamShot> it((ObjectDir *)mParent, true);
    CamShot *ret = it;
    for (; it != nullptr && it != cshot; ++it)
        ;
    if (it)
        ++it;
    if (!it)
        return ret;
    else
        return it;
}

DataNode CameraManager::OnCycleShot(DataArray *da) {
    CamShot *after = ShotAfter(mCurrentCam);
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
    int floatIdx = 3;
    if (da->Size() > 3) {
        const DataNode &n = da->Evaluate(3);
        DataArray *nArr = n.Type() == kDataArray ? n.Array() : nullptr;
        if (nArr) {
            DataArray *arr = da->Array(3);
            floatIdx = 4;
            for (uint i = 0; i != arr->Size(); i++) {
                DataArray *currArr = arr->Array(i);
                PropertyFilter filt;
                filt.prop = currArr->Evaluate(0);
                if (filt.prop.Type() == kDataSymbol && filt.prop.Sym() == flags_exact) {
                    filt.mask = currArr->Int(1);
                    filt.match = currArr->Int(2);
                } else if (filt.prop.Type() == kDataSymbol && filt.prop.Sym() == flags_any) {
                    filt.mask = currArr->Int(1);
                    filt.match = 1;
                } else {
                    filt.match = currArr->Evaluate(1);
                    filt.mask = -1;
                }
                filts.push_back(filt);
            }
        }
        if (f) {
            *f = da->Float(floatIdx);
        }
    }
    return sym;
}

bool CameraManager::SetCrowds(ObjVector<CamShotCrowd> &crowds) {
    bool ret = false;
    FOREACH (it, mCrowds) {
        WorldCrowd *curCrowd = *it;
        ObjVector<CamShotCrowd>::iterator target;
        FOREACH (cit, crowds) {
            if (curCrowd == cit->mCrowd) {
                target = cit;
                goto done;
            }
        }
        target = crowds.end();
    done:
        if (target != crowds.end()) {
            curCrowd->SetShowing(true);
            ret = true;
            curCrowd->SetRotate(target->mCrowdRotate);
        } else {
            curCrowd->SetShowing(false);
        }
    }
    return ret;
}

bool CameraManager::ShotMatches(CamShot *shot, const std::vector<PropertyFilter> &filts) {
    static Symbol flags_exact("flags_exact");
    static Symbol flags_any("flags_any");
    int shotFlags = shot->Flags();
    FOREACH (it, filts) {
        DataNode n;
        if (it->prop.Type() == kDataArray) {
            n = shot->Property(it->prop.Array())->Evaluate();
        } else {
            Symbol s = it->prop.Sym();
            if (s == flags_exact) {
                n = it->mask & shotFlags;
            } else if (s == flags_any) {
                n = (it->mask & shotFlags) != 0;
            } else {
                n = shot->Property(s)->Evaluate();
            }
        }
        if (it->match.Type() == kDataArray) {
            DataArray *arr = it->match.Array();
            uint i = 0;
            for (; i != arr->Size(); i++) {
                if (n.Equal(arr->Evaluate(i), nullptr, true))
                    break;
            }
            if (i == arr->Size()) {
                return false;
            }
        } else if (n != it->match) {
            return false;
        }
    }
    return true;
}

CamShot *
CameraManager::FindCameraShot(Symbol s, const std::vector<PropertyFilter> &filts) {
    FirstShotOk(s);
    ObjPtrList<CamShot> &camlist = FindOrAddCategory(s);
    FOREACH (it, camlist) {
        CamShot *cur = *it;
        if (!cur->Disabled() && ShotMatches(cur, filts)) {
            if (cur->ShotOk(mCurrentCam)) {
                camlist.splice(camlist.end(), camlist, it);
                return cur;
            }
        }
    }
    return 0;
}

ObjPtrList<CamShot> &CameraManager::FindOrAddCategory(Symbol cat) {
    Category targetCat;
    targetCat.mCategory = cat;
    Category *lowerCat =
        std::lower_bound(mCategories.begin(), mCategories.end(), targetCat);
    if (lowerCat == mCategories.end() || lowerCat->mCategory != cat) {
        targetCat.mShots = new ObjPtrList<CamShot>(mParent);
        mCategories.push_back(targetCat);
        std::sort(mCategories.begin(), mCategories.end());
        lowerCat = std::lower_bound(mCategories.begin(), mCategories.end(), targetCat);
    }
    return *lowerCat->mShots;
}

int CameraManager::NumCameraShots(
    Symbol s, const std::vector<PropertyFilter> &filts, std::list<CamShot *> *shots
) {
    FirstShotOk(s);
    ObjPtrList<CamShot> &camlist = FindOrAddCategory(s);
    int num = 0;
    FOREACH (it, camlist) {
        CamShot *cur = *it;
        if (cur->Disabled() == 0 && ShotMatches(cur, filts) && cur->ShotOk(mCurrentCam)) {
            shots->push_back(cur);
            num++;
        }
    }
    return num;
}

void CameraManager::Randomize() {
    sRand.Seed(sSeed);
    FOREACH (it, mCategories) {
        RandomizeCategory(*it->mShots);
    }
}

void CameraManager::Poll() {
    static Symbol shot("shot");
    static Symbol category("category");
    if (!MiloCamera()) {
        if (mCurrentCam) {
            bool shotOver = mCurrentCam->ShotOver();
            RndCam *cam = mCurrentCam->GetCam();
            if (cam) {
                Transform tfc0 = cam->LocalXfm();
                float yFov = cam->YFov();
                float nearPlane = cam->NearPlane();
                float farPlane = cam->FarPlane();
                float frame = CalcFrame();
                mCurrentCam->SetFrame(frame, 1);
                float f16 = mBlendTime > 0 ? Clamp(0.0f, 1.0f, frame / mBlendTime) : 1;
                if (mNextShotChanged) {
                    f16 = 0;
                }
                if (f16 < 1) {
                    Transform &localXfm = cam->DirtyLocalXfm();
                    Interp(tfc0.v, localXfm.v, f16, localXfm.v);
                    Interp(tfc0.m.y, localXfm.m.y, f16, localXfm.m.y);
                    Normalize(localXfm.m.y, localXfm.m.y);
                    Interp(tfc0.m.x, localXfm.m.x, f16, localXfm.m.x);
                    // NormalizeAboutY?
                    // NormalizeAboutY(localXfm.m);
                    Cross(localXfm.m.x, localXfm.m.y, localXfm.m.z);
                    Normalize(localXfm.m.z, localXfm.m.z);
                    Cross(localXfm.m.y, localXfm.m.z, localXfm.m.x);
                    float blendYFov = Interp(yFov, cam->YFov(), f16);
                    float blendNear = Interp(nearPlane, cam->NearPlane(), f16);
                    float blendFar = Interp(farPlane, cam->FarPlane(), f16);
                    cam->SetFrustum(blendNear, blendFar, blendYFov, 1);
                } else if (mLastBlend < 1) {
                    static Message msg("blend_finished", 0);
                    msg[0] = mCurrentCam.Ptr();
                    Export(msg, true);
                }
                mLastBlend = f16;
            }
            if (!shotOver && mCurrentCam && mCurrentCam->ShotOver()) {
                static Message msg("shot_over", 0);
                msg[0] = mCurrentCam.Ptr();
                Export(msg, true);
            }
        }
        if (mFreeCam) {
            mFreeCam->Poll();
        }
    }
}

void CameraManager::SyncObjects(WorldDir *parent) {
    mParent = parent;
    mCategories.clear();
    mCategories.reserve(100);
    mCrowds.clear();
    for (ObjDirItr<Hmx::Object> it(mParent, true); it != nullptr; ++it) {
        CamShot *shot = dynamic_cast<CamShot *>(&*it);
        if (shot) {
            shot->SetParent(mParent);
            if (shot->PlatformOk()) {
                FindOrAddCategory(shot->Category()).push_back(shot);
            }
        } else {
            WorldCrowd *crowd = dynamic_cast<WorldCrowd *>(&*it);
            if (crowd) {
                mCrowds.push_back(crowd);
            }
        }
    }
    Randomize();
}

CamShot *
CameraManager::PickCameraShot(Symbol s, const std::vector<PropertyFilter> &filts) {
    CamShot *ret = FindCameraShot(s, filts);
    if (!ret) {
        static Symbol flags_exact("flags_exact");
        static Symbol flags_any("flags_any");
        String msg("No acceptable camera shot:");
        msg << " cat: " << s;
        FOREACH (it, filts) {
            msg << " (" << it->prop << " " << it->match;
            if (it->prop.Equal(flags_any, nullptr, true)
                || it->prop.Equal(flags_exact, nullptr, true)) {
                msg << MakeString(" 0x%x", it->mask);
            }
            msg << ")";
        }
        MILO_NOTIFY(msg.c_str());
        return nullptr;
    } else {
        mNextShotChanged = true;
        mNextShot = ret;
        return ret;
    }
}

DataNode CameraManager::OnPickCameraShot(DataArray *da) {
    std::vector<PropertyFilter> pvec;
    pvec.reserve(20);
    Symbol sym = MakeCategoryAndFilters(da, pvec, &mBlendTime);
    return PickCameraShot(sym, pvec);
}

DataNode CameraManager::OnFindCameraShot(DataArray *da) {
    std::vector<PropertyFilter> pvec;
    pvec.reserve(20);
    Symbol sym = MakeCategoryAndFilters(da, pvec, nullptr);
    return FindCameraShot(sym, pvec);
}

DataNode CameraManager::OnNumCameraShots(DataArray *da) {
    std::vector<PropertyFilter> pvec;
    pvec.reserve(20);
    Symbol sym = MakeCategoryAndFilters(da, pvec, nullptr);
    return NumCameraShots(sym, pvec, nullptr);
}

DataNode CameraManager::OnRandomSeed(DataArray *da) {
    sSeed = da->Int(2);
    Randomize();
    return 0;
}

DataNode CameraManager::OnGetShotList(DataArray *a) {
    DataArray *list = new DataArray(0);
    FOREACH (it, mCategories) {
        FOREACH_PTR (shotIt, it->mShots) {
            list->Insert(list->Size(), *shotIt);
        }
    }
    list->SortNodes(0);
    list->Insert(0, NULL_OBJ);
    DataNode ret(list);
    list->Release();
    return ret;
}

DataNode CameraManager::OnIterateShot(DataArray *da) {
    DataNode *var = da->Var(2);
    DataNode d28(*var);
    FOREACH (it, mCategories) {
        FOREACH_PTR (lit, it->mShots) {
            *var = *lit;
            for (int i = 3; i < da->Size(); i++) {
                da->Command(i)->Execute();
            }
        }
    }
    *var = d28;
    return 0;
}
