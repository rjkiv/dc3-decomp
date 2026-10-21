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
        bool operator<(const Category &c) const { return mCategory < c.mCategory; }

        Symbol mCategory; // 0x0
        ObjPtrList<CamShot> *mShots; // 0x4
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

    CamShot *GetNextShot() const { return mNextShot; }
    CamShot *GetCurrentShot() const { return mCurrentCam; }
    bool HasFreeCam() const { return mFreeCam; }
    void ForceCamShot(CamShot *);
    FreeCamera *GetFreeCam(int padNum);
    void DeleteFreeCam();
    CamShot *ShotAfter(CamShot *cur);
    CamShot *FindCameraShot(Symbol category, const std::vector<PropertyFilter> &filters);
    CamShot *MiloCamera();
    void ForceCameraShot(CamShot *shot, bool forceRestart);
    void PrePoll();
    void Randomize();
    void Enter();
    bool SetCrowds(ObjVector<CamShotCrowd> &crowds);
    int NumCameraShots(
        Symbol category,
        const std::vector<PropertyFilter> &filters,
        std::list<CamShot *> *shots
    );
    void SyncObjects(WorldDir *parent);
    CamShot *PickCameraShot(Symbol category, const std::vector<PropertyFilter> &filters);
    void Poll();
    std::vector<Category> &GetCameras() { return mCategories; }

    //  public: void ClearFlags(int32_t);
    //  public: void SetBlendTime(float);

protected:
    CameraManager();

private:
    void StartShot_(CamShot *shot);
    float CalcFrame();
    void FirstShotOk(Symbol category);
    void RandomizeCategory(ObjPtrList<CamShot> &cams);
    bool ShotMatches(CamShot *shot, const std::vector<PropertyFilter> &filters);
    Symbol MakeCategoryAndFilters(
        DataArray *args, std::vector<PropertyFilter> &filterList, float *blendTime
    );
    ObjPtrList<CamShot> &FindOrAddCategory(Symbol cat);

    DataNode OnPickCameraShot(DataArray *);
    DataNode OnFindCameraShot(DataArray *);
    DataNode OnCycleShot(DataArray *);
    DataNode OnRandomSeed(DataArray *);
    DataNode OnIterateShot(DataArray *);
    DataNode OnNumCameraShots(DataArray *);
    DataNode OnGetShotList(DataArray *);

    /** "Controlling world object" */
    WorldDir *mParent; // 0x2c
    std::vector<Category> mCategories; // 0x30
    /** "Which shot to play right now" */
    ObjPtr<CamShot> mNextShot; // 0x3c
    /** "Next camera blend time in units of camera, is run-time, not serialized" */
    float mBlendTime; // 0x50
    float mLastBlend; // 0x54
    bool mNextShotChanged; // 0x58
    ObjPtr<CamShot> mCurrentCam; // 0x5c
    float mCamStartTime; // 0x70
    FreeCamera *mFreeCam; // 0x74
    ObjPtrList<WorldCrowd> mCrowds; // 0x78
};
