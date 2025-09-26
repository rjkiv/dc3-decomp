#pragma once
#include "char/Character.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "world/CameraShot.h"

enum HamPlayerFlags {
    /** "Player 0" */
    kHamPlayer0 = 0,
    /** "Player 1" */
    kHamPlayer1 = 1,
    /** "Both Players" */
    kHamPlayerBoth = 2,
    /** "Neither Player" */
    kHamPlayerOff = 3,
    /** "Player 0 Solo" */
    kHamPlayer0SoloInOut = 4,
    /** "Player 1 Solo" */
    kHamPlayer1SoloInOut = 5
};

/** "Hammer specific camera shot" */
class HamCamShot : public CamShot {
public:
    struct Target {
        Target(Hmx::Object *owner)
            : mFastForward(0), mEnvOverride(owner), mForceLOD(kLODPerFrame),
              mTeleport(false), mReturn(true), mSelfShadow(false), unk68p4(true),
              unk68p3(true) {
            mTo.Reset();
        }
        void UpdateTarget(Symbol, HamCamShot *);
        void Store(HamCamShot *);

        /** "Symbolic name of target" */
        Symbol mTarget; // 0x0
        /** "the transform to teleport the character to" */
        Transform mTo; // 0x4
        /** "Name of CharClipGroup to play on character" */
        Symbol mAnimGroup; // 0x44
        /** "Fast forward chosen animation by this time, in camera units" */
        float mFastForward; // 0x48
        /** "Event to fastforward relative to" */
        Symbol mForwardEvent; // 0x4c
        /** "environment override for this target during this shot" */
        ObjPtr<RndEnviron> mEnvOverride; // 0x50
        /** "Forces LOD, kLODPerFrame is normal behavior of picking per frame,
            the others force the lod (0 is highest res lod, 2 is lowest res lod)" */
        int mForceLOD : 3; // 0x64
        // 0x68
        /** "do we teleport this character?" */
        bool mTeleport : 1;
        /** "return to original position after shot?" */
        bool mReturn : 1;
        /** "should character cast a self shadow" */
        bool mSelfShadow : 1;
        bool unk68p4 : 1;
        bool unk68p3 : 1;
    };

    // size 0x4c
    struct TargetCache {
        TargetCache() : unk0(0), unk4(0) { unkxfm.Reset(); }

        Symbol unksym; // 0x8
        int unk0; // 0x0
        RndTransformable *unk4; // 0x4
        Transform unkxfm; // 0xc
    };

    // Hmx::Object
    OBJ_CLASSNAME(HamCamShot);
    OBJ_SET_TYPE(HamCamShot);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void StartAnim();
    virtual void EndAnim();
    virtual void SetFrame(float frame, float blend);
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // CamShot
    virtual void SetPreFrame(float, float);
    virtual CamShot *CurrentShot() { return mCurrentShot; }

    RndTransformable *FindTarget(Symbol);
    float GetTotalDurationSeconds();
    float GetTotalDuration();
    void Store();
    HamCamShot *InitialShot();
    int GetNumShots();
    void Reteleport(const Vector3 &, bool, Symbol);
    bool TargetTeleportTransform(Symbol, Transform &);
    void TeleportTarget(RndTransformable *, const Transform &, bool);

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(HamCamShot)

private:
    DataNode OnTestDelta(DataArray *);
    DataNode AddTarget(DataArray *);
    DataNode OnAllowableNextShots(const DataArray *);
    DataNode OnListAllNextShots(const DataArray *);
    DataNode OnListTargets(const DataArray *);

protected:
    HamCamShot();

    virtual bool CheckShotStarted() { return !unk2d4 && CamShot::CheckShotStarted(); }
    virtual bool CheckShotOver(float f1) {
        return !unk2d4 && f1 >= unk2d8 && CamShot::CheckShotOver(f1);
    }
    virtual void SetFrameEx(float, float);

    void CheckNextShots();
    void ResetNextShot();
    void FlipTargetAnimGroups();
    void UpdateTargetsFlipped();
    bool IterateNextShot();
    bool ListNextShots(std::list<HamCamShot *> &);

    std::list<TargetCache>::iterator CreateTargetCache(Symbol);

    static std::list<TargetCache> sCache;

    ObjList<Target> mTargets; // 0x284
    /** "30fps reg: minimum time this shot can last,
        DCuts: time past zero time in which the shot can be interupted" */
    int mMinTime; // 0x290
    /** "30fps maximum duration for this shot, 0 is infinite" */
    int mMaxTime; // 0x294
    /** "synchronization time for this camshot" */
    float mZeroTime; // 0x298
    /** "Flag to determine player configuration" */
    HamPlayerFlags mPlayerFlag; // 0x29c
    /** "Next camshots, in order" */
    ObjPtrList<HamCamShot> mNextShots; // 0x2a0
    ObjPtrList<HamCamShot>::iterator unk2b4; // 0x2b4
    ObjPtr<HamCamShot> mCurrentShot; // 0x2b8
    float unk2cc; // 0x2cc
    float unk2d0; // 0x2d0 - duration
    bool unk2d4; // 0x2d4
    float unk2d8; // 0x2d8
    bool unk2dc; // 0x2dc
    bool unk2dd; // 0x2dd
    /** "Anims set throughout this shot and any next shots
        Not valid entries for a next shot." */
    ObjPtrList<RndAnimatable> mMasterAnims; // 0x2e0
    int unk2f4; // 0x2f4
    ObjPtrList<RndDrawable> unk2f8; // 0x2f8
    ObjPtrList<RndDrawable> unk30c; // 0x30c
    ObjPtrList<RndDrawable> unk320; // 0x320
    std::vector<RndDrawable *> unk334; // 0x334
    ObjPtrList<RndDrawable> unk340; // 0x340
    ObjPtrList<RndDrawable> unk354; // 0x354
    ObjPtrList<RndDrawable> unk368; // 0x368
    std::vector<RndDrawable *> unk37c; // 0x37c
    bool unk388; // 0x388
};
