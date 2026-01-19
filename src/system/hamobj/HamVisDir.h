#pragma once
#include "hamobj/Pose.h"
#include "gesture/SkeletonDir.h"
#include "gesture/FreestyleMotionFilter.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

struct PoseOwner {
    PoseOwner();
    ~PoseOwner();

    Pose *pose; // 0x0
    Pose *holder; // 0x4
    bool in_pose; // 0x8
    Symbol name; // 0xc
};

/** "panel dir that handles the visualizer" */
class HamVisDir : public SkeletonDir {
public:
    // Hmx::Object
    virtual ~HamVisDir();
    OBJ_CLASSNAME(HamVisDir);
    OBJ_SET_TYPE(HamVisDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndPollable
    virtual void Enter();
    // SkeletonCallback
    virtual void PostUpdate(const struct SkeletonUpdateData *);

    OBJ_MEM_OVERLOAD(0x1D)
    NEW_OBJ(HamVisDir)

    void Run(bool);
    void SetGrooviness(float);

protected:
    HamVisDir();
    void CheckPose(int, PoseOwner &);
    void CalcArmLengths(std::vector<float> &, const Skeleton &);

    Transform unk284; // 0x284
    FreestyleMotionFilter *mFilter; // 0x2c4
    bool mRunning; // 0x2c8
    std::vector<unsigned int> unk2cc; // 0x2cc
    int unk2d8; // 0x2d8
    int unk2dc; // 0x2dc
    /** "Animated from 0 - 100, depending on player one's hand height" */
    ObjPtr<RndAnimatable> mPlayer1Right; // 0x2e0
    /** "Animated from 0 - 100, depending on player one's hand height" */
    ObjPtr<RndAnimatable> mPlayer1Left; // 0x2f4
    /** "Animated from 0 - 100, depending on player two's hand height" */
    ObjPtr<RndAnimatable> mPlayer2Right; // 0x308
    /** "Animated from 0 - 100, depending on player two's hand height" */
    ObjPtr<RndAnimatable> mPlayer2Left; // 0x31c
    /** "Allow Milo anim bar to drive the gesture propanim frame,
        not the player's skeleton." */
    bool mMiloManualFrame; // 0x330
    float unk334; // 0x334
    PoseOwner mSquatPoses[2]; // 0x338
    PoseOwner mYPoses[2]; // 0x358
};
