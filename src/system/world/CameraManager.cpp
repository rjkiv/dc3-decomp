#include "world/CameraManager.h"
#include "macros.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
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
    FOREACH (it, mCameraShotCategories) {
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

BEGIN_PROPSYNCS(CameraManager)
    SYNC_PROP_SET(next_shot, mNextShot.Ptr(), SetNextShot(_val.Obj<CamShot>()))
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
    bs >> mNextShot;
END_LOADS

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

void CameraManager::SetNextShot(CamShot *shot) {
    unk58 = shot != mNextShot || unk58;
    mNextShot = shot;
}

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

struct NameSort {
    bool operator()(CamShot *o1, CamShot *o2) const {
        return strcmp(o1->Name(), o2->Name()) < 0;
    }
};

void CameraManager::RandomizeCategory(ObjPtrList<CamShot> &camlist) {
    std::vector<CamShot *> camshots;
    {
        MemTemp m;
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
                } else if (filt.prop.Type() == kDataSymbol
                           && filt.prop.Sym() == flags_any) {
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
    FOREACH (it, unk78) {
        WorldCrowd *curCrowd = *it;
        FOREACH (cit, crowds) {
        }
    }
    return false;
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
            if (cur->ShotOk(mCurrentShot)) {
                camlist.MoveItem(camlist.end(), camlist, it);
                return cur;
            }
        }
    }
    return 0;
}

ObjPtrList<CamShot> &CameraManager::FindOrAddCategory(Symbol cat) {
    Category targetCat;
    targetCat.unk0 = cat;
    Category *lowerCat = std::lower_bound(
        mCameraShotCategories.begin(), mCameraShotCategories.end(), targetCat
    );
    if (lowerCat == mCameraShotCategories.end() || lowerCat->unk0 != cat) {
        targetCat.unk4 = new ObjPtrList<CamShot>(mParent);
        mCameraShotCategories.push_back(targetCat);
        std::sort(mCameraShotCategories.begin(), mCameraShotCategories.end());
        lowerCat = std::lower_bound(
            mCameraShotCategories.begin(), mCameraShotCategories.end(), targetCat
        );
    }
    return *lowerCat->unk4;
}

int CameraManager::NumCameraShots(
    Symbol s, const std::vector<PropertyFilter> &filts, std::list<CamShot *> *shots
) {
    FirstShotOk(s);
    ObjPtrList<CamShot> &camlist = FindOrAddCategory(s);
    int num = 0;
    FOREACH (it, camlist) {
        CamShot *cur = *it;
        if (cur->Disabled() == 0 && ShotMatches(cur, filts)
            && cur->ShotOk(mCurrentShot)) {
            shots->push_back(cur);
            num++;
        }
    }
    return num;
}

void CameraManager::Randomize() {
    sRand.Seed(sSeed);
    FOREACH (it, mCameraShotCategories) {
        RandomizeCategory(*it->unk4);
    }
}

void CameraManager::SyncObjects(WorldDir *parent) {
    mParent = parent;
    mCameraShotCategories.clear();
    mCameraShotCategories.reserve(100);
    unk78.clear();
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
                unk78.push_back(crowd);
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
        unk58 = true;
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
    FOREACH (it, mCameraShotCategories) {
        FOREACH_PTR (shotIt, it->unk4) {
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
    FOREACH (it, mCameraShotCategories) {
        FOREACH_PTR (lit, it->unk4) {
            *var = *lit;
            for (int i = 3; i < da->Size(); i++) {
                da->Command(i)->Execute();
            }
        }
    }
    *var = d28;
    return 0;
}
