#pragma once
#include "obj/Data.h"

#include "rndobj/Highlight.h"
#include "utl/MemMgr.h"

/**
 * @brief An object controlling position, rotation, etc. for any derivatives.
 * Original _objects description:
 * "Base class for transformable objects. Trans objects have
 * a 3D position, rotation, and scale."
 */
class RndTransformable : public virtual RndHighlightable {
public:
    enum Constraint {
        /** "no constraint" */
        kConstraintNone = 0,
        /** "Uses own local rotation as world rotation" */
        kConstraintLocalRotate = 1,
        /** "Uses [trans_parent] world xfm as own" */
        kConstraintParentWorld = 2,
        /** "Points Y axis at [trans_target]" */
        kConstraintLookAtTarget = 3,
        /** "Flattens self onto the plane specified by [trans_target]" */
        kConstraintShadowTarget = 4,
        /** "Billboard about Z axis only, relative to [trans_target] if set, otherwise
         * relative to current camera" */
        kConstraintBillboardZ = 5,
        /** "Billboard about XZ axes only (no roll), relative to [trans_target] if set,
         * otherwise relative to current camera" */
        kConstraintBillboardXZ = 6,
        /** "Billboards about XYZ axes, relative to [trans_target] if set, otherwise
         * relative to current camera" */
        kConstraintBillboardXYZ = 7,
        /** "Billboards w/o perspective correction, relative to [trans_target] if set,
         * otherwise relative to current camera" */
        kConstraintFastBillboardXYZ = 8,
        /** "Uses [trans_target] world xfm as own" */
        kConstraintTargetWorld = 9,
        /** "Ignores [trans_parent] rotation" */
        kConstraintNoParentRotation = 10,
        /** "Uses current camera as though it were [trans_parent], ignoring its rotation,
         * relative to [trans_target] if set, otherwise relative to current camera" */
        kConstraintSkyBox = 11,
        /** "Uses current camera as though it were [trans_parent], ignoring its rotation,
         * and ignoring the z value, good for skybox bases, relative to [trans_target] if
         * set, otherwise relative to current camera" */
        kConstraintSkyBoxXY = 12
    };

    virtual ~RndTransformable();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Trans)
    OBJ_SET_TYPE(Trans)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void Highlight();
    virtual void Print();

    OBJ_MEM_OVERLOAD(0x1C);

    const Transform &LocalXfm() const { return mLocalXfm; }

    Transform &DirtyLocalXfm() {
        SetDirty();
        return mLocalXfm;
    }

    void SetLocalXfm(const Transform &tf) {
        mLocalXfm = tf;
        SetDirty();
    }

    void SetLocalPos(const Vector3 &vec) {
        mLocalXfm.v = vec;
        SetDirty();
    }

    void SetLocalRot(const Hmx::Matrix3 &mtx) {
        mLocalXfm.m = mtx;
        SetDirty();
    }

    const Transform &WorldXfm() { return !mDirty ? mWorldXfm : WorldXfm_Force(); }

    void GetLocalRot(Vector3 &) const;
    void SetWorldXfm(const Transform &);
    void SetWorldPos(const Vector3 &);
    void SetTransConstraint(Constraint, RndTransformable *, bool);
    void SetTransParent(RndTransformable *, bool);
    void SetLocalRot(Vector3);
    void SetLocalRotIndex(int, float);
    void ComputeLocalXfm(const Transform &tf);
    void DistributeChildren(bool, float);

    static void Init();
    NEW_OBJ(RndTransformable);

private:
    static Plane sShadowPlane;

    void SetDirty() {
        if (!mDirty)
            SetDirty_Force();
    }
    void SetDirty_Force();
    const Transform &WorldXfm_Force();

    DataNode OnCopyLocalTo(const DataArray *);
    DataNode OnGetLocalPos(const DataArray *);
    DataNode OnGetLocalPosIndex(const DataArray *);
    DataNode OnGetLocalRot(const DataArray *);
    DataNode OnGetLocalRotIndex(const DataArray *);
    DataNode OnSetLocalPos(const DataArray *);
    DataNode OnSetLocalPosIndex(const DataArray *);
    DataNode OnSetLocalRot(const DataArray *);
    DataNode OnSetLocalRotIndex(const DataArray *);
    DataNode OnSetLocalRotMat(const DataArray *);
    DataNode OnSetTransConstraint(const DataArray *);
    DataNode OnSetLocalScale(const DataArray *);
    DataNode OnSetLocalScaleIndex(const DataArray *);
    DataNode OnGetLocalScale(const DataArray *);
    DataNode OnGetLocalScaleIndex(const DataArray *);
    DataNode OnGetWorldForward(const DataArray *);
    DataNode OnGetWorldRight(const DataArray *);
    DataNode OnGetWorldUp(const DataArray *);
    DataNode OnGetWorldPos(const DataArray *);
    DataNode OnGetWorldRot(const DataArray *);
    DataNode OnGetChildren(const DataArray *);
    DataNode OnCopyWorldTransFrom(const DataArray *);
    DataNode OnCopyWorldPosFrom(const DataArray *);

protected:
    RndTransformable();

    virtual void UpdatedWorldXfm() {}

    Transform mLocalXfm; // 0x8
    Transform mWorldXfm; // 0x48
    ObjOwnerPtr<RndTransformable> mParent; // 0x88
    std::list<RndTransformable *> mChildren; // 0x9c
    ObjPtr<RndTransformable> mTarget; // 0xa4
    Constraint mConstraint; // 0xb8
    bool mPreserveScale; // 0xbc
    bool mDirty; // 0xbd
};
