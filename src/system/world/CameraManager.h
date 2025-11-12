#pragma once
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"
#include "world/Crowd.h"
#include "world/FreeCamera.h"

class WorldDir;

/** "Searches for and sequences CamShots" */
class CameraManager : public Hmx::Object {
public:
    class Category {
    public:
        Symbol unk0;
        ObjPtrList<CamShot> *unk4;
    };

    struct PropertyFilter {
        DataNode prop; // 0x0
        DataNode match; // 0x8
        int mask; // 0x10
    };

    CameraManager(WorldDir *);
    // Hmx::Object
    virtual ~CameraManager();
    OBJ_CLASSNAME(CameraManager);
    OBJ_SET_TYPE(CameraManager);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(CameraManager)
    static Rand sRand;
    static int sSeed;

    CamShot *NextShot() const { return mNextShot; }
    CamShot *CurrentShot() const { return mCurrentShot; }
    bool HasFreeCam() const { return mFreeCam; }
    void ForceCamShot(CamShot *);
    FreeCamera *GetFreeCam(int);
    void DeleteFreeCam();
    CamShot *ShotAfter(CamShot *);
    CamShot *FindCameraShot(Symbol, const std::vector<PropertyFilter> &);
    CamShot *MiloCamera();
    void ForceCameraShot(CamShot *, bool);
    void PrePoll();
    void Randomize();
    void Enter();
    bool SetCrowds(ObjVector<CamShotCrowd> &);
    int NumCameraShots(Symbol s, const std::vector<PropertyFilter> &);

private:
    void StartShot_(CamShot *);
    float CalcFrame();
    void FirstShotOk(Symbol);
    void RandomizeCategory(ObjPtrList<CamShot> &);

    DataNode OnPickCameraShot(DataArray *);
    DataNode OnFindCameraShot(DataArray *);
    DataNode OnCycleShot(DataArray *);
    DataNode OnRandomSeed(DataArray *);
    DataNode OnIterateShot(DataArray *);
    DataNode OnNumCameraShots(DataArray *);
    DataNode OnGetShotList(DataArray *);
    Symbol MakeCategoryAndFilters(DataArray *da, std::vector<PropertyFilter> &, float *);
    ObjPtrList<CamShot> &FindOrAddCategory(Symbol);

protected:
    CameraManager();

    /** "Controlling world object" */
    WorldDir *mParent; // 0x2c
    std::vector<Category> mCameraShotCategories; // 0x30
    /** "Which shot to play right now" */
    ObjPtr<CamShot> mNextShot; // 0x3c
    /** "Next camera blend time in units of camera, is run-time, not serialized" */
    float mBlendTime; // 0x50
    float unk54; // 0x54
    bool unk58; // 0x58
    ObjPtr<CamShot> mCurrentShot; // 0x5c
    float mCamStartTime; // 0x70
    FreeCamera *mFreeCam; // 0x74
    ObjPtrList<WorldCrowd> unk78; // 0x78
};
