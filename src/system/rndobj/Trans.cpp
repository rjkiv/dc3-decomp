#include "rndobj/Trans.h"
#include "Trans.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Utl.h"
#include "obj/Data.h"

#include "os/Debug.h"
#include "utl/BinStream.h"
#include "math/Rot.h"

void RndTransformable::Print() {
    TheDebug << "   localXfm: " << mLocalXfm << "\n";
    TheDebug << "   worldXfm: " << mWorldXfm << "\n";
    TheDebug << "   constraint: ";
    if (mConstraint == kConstraintNone) {
        TheDebug << "None";
    } else if (mConstraint == kConstraintLocalRotate) {
        TheDebug << "LocalRotate";
    } else if (mConstraint == kConstraintLookAtTarget) {
        TheDebug << "LookAtTarget";
    } else if (mConstraint == kConstraintShadowTarget) {
        TheDebug << "ShadowTarget";
    } else if (mConstraint == kConstraintParentWorld) {
        TheDebug << "ParentWorld";
    } else if (mConstraint == kConstraintBillboardZ) {
        TheDebug << "BillboardZ";
    } else if (mConstraint == kConstraintBillboardXZ) {
        TheDebug << "BillboardXZ";
    } else if (mConstraint == kConstraintBillboardXYZ) {
        TheDebug << "BillboardXYZ";
    } else if (mConstraint == kConstraintFastBillboardXYZ) {
        TheDebug << "FastBillboardXYZ";
    }
    TheDebug << "\n";
    TheDebug << "   preserveScale: " << mPreserveScale << "\n";
    TheDebug << "   parent: " << mParent << "\n";
}

void RndTransformable::GetLocalRot(Vector3 &v) const {
    Hmx::Matrix3 m;
    m = mLocalXfm.m;
    Normalize(m, m);
    MakeEuler(m, v);
    v *= RAD2DEG;
}

void RndTransformable::SetDirty_Force() {
    mDirty = true;
    if (!mChildren.empty()) {
        for (std::list<RndTransformable *>::iterator it = mChildren.begin();
             it != mChildren.end();
             ++it) {
            (*it)->SetDirty();
        }
    }
}

namespace {
    bool HorizontalCmp(const RndTransformable *t1, const RndTransformable *t2) {
        return t1->LocalXfm().v[0] < t2->LocalXfm().v[0];
    }

    bool VerticalCmp(const RndTransformable *t1, const RndTransformable *t2) {
        return t1->LocalXfm().v[2] > t2->LocalXfm().v[2];
    }
}

void RndTransformable::Save(BinStream &bs) {
    bs << 9;
    if (ClassName() == StaticClassName()) {
        SAVE_SUPERCLASS(Hmx::Object)
    }
    bs << mLocalXfm;
    bs << mWorldXfm;
    bs << mConstraint;
    bs << mTarget << mPreserveScale << mParent;
}

DataNode RndTransformable::OnGetLocalPos(const DataArray *da) {
    *da->Var(2) = mLocalXfm.v.x;
    *da->Var(3) = mLocalXfm.v.y;
    *da->Var(4) = mLocalXfm.v.z;
    return 0;
}

DataNode RndTransformable::OnGetLocalPosIndex(const DataArray *a) {
    MILO_ASSERT(a->Int(2) < 3, 0x351);
    return mLocalXfm.v[a->Int(2)];
}

DataNode RndTransformable::OnGetLocalRot(const DataArray *a) {
    Vector3 v;
    GetLocalRot(v);
    *a->Var(2) = v.x;
    *a->Var(3) = v.y;
    *a->Var(4) = v.z;
    return 0;
}

DataNode RndTransformable::OnGetLocalRotIndex(const DataArray *a) {
    MILO_ASSERT(a->Int(2) < 3, 0x36B);
    Vector3 v1, v2;
    MakeEulerScale(mLocalXfm.m, v1, v2);
    v1 *= RAD2DEG;
    return v1[a->Int(2)];
}

DataNode RndTransformable::OnSetLocalScale(const DataArray *a) {
    SetLocalScale(this, Vector3(a->Float(2), a->Float(3), a->Float(4)));
    return 0;
}

DataNode RndTransformable::OnSetLocalScaleIndex(const DataArray *a) {
    MILO_ASSERT(a->Int(2) < 3, 0x3C1);
    Vector3 v28;
    MakeScale(LocalXfm().m, v28);
    v28[a->Int(2)] = a->Float(3);
    SetLocalScale(this, v28);
    return 0;
}

DataNode RndTransformable::OnGetLocalScale(const DataArray *da) {
    Vector3 v20;
    MakeScale(LocalXfm().m, v20);
    *da->Var(2) = v20.x;
    *da->Var(3) = v20.y;
    *da->Var(4) = v20.z;
    return 0;
}

DataNode RndTransformable::OnGetLocalScaleIndex(const DataArray *a) {
    MILO_ASSERT(a->Int(2) < 3, 0x3D9);
    Vector3 v28;
    MakeScale(LocalXfm().m, v28);
    return v28[a->Int(2)];
}

void RndTransformable::SetWorldXfm(const Transform &xfm) {
    mWorldXfm = xfm;
    mDirty = false;
    UpdatedWorldXfm();
    for (std::list<RndTransformable *>::iterator it = mChildren.begin();
         it != mChildren.end();
         ++it) {
        (*it)->SetDirty();
    }
}

void RndTransformable::SetWorldPos(const Vector3 &pos) {
    mWorldXfm.v = pos;
    UpdatedWorldXfm();
    for (std::list<RndTransformable *>::iterator it = mChildren.begin();
         it != mChildren.end();
         ++it) {
        (*it)->SetDirty();
    }
}

void RndTransformable::SetTransConstraint(
    Constraint cst, RndTransformable *t, bool preserveScale
) {
    MILO_ASSERT(t != this, 0x164);
    mConstraint = cst;
    mPreserveScale = preserveScale;
    mTarget = t;
    SetDirty();
}

DataNode RndTransformable::OnCopyLocalTo(const DataArray *da) {
    DataArray *arr = da->Array(2);
    for (int i = arr->Size() - 1; i >= 0; i--) {
        RndTransformable *t = arr->Obj<RndTransformable>(i);
        t->SetLocalXfm(mLocalXfm);
    }
    return 0;
}

DataNode RndTransformable::OnSetLocalPos(const DataArray *da) {
    SetLocalPos(Vector3(da->Float(2), da->Float(3), da->Float(4)));
    return 0;
}

DataNode RndTransformable::OnSetLocalPosIndex(const DataArray *a) {
    MILO_ASSERT(a->Int(2) < 3, 0x385);
    Vector3 v28(mLocalXfm.v);
    v28[a->Int(2)] = a->Float(3);
    SetLocalPos(v28);
    return 0;
}

void RndTransformable::SetLocalRot(Vector3 v) {
    v *= DEG2RAD;
    Hmx::Matrix3 m;
    MakeRotMatrix(v, m, true);
    SetLocalRot(m);
}

DataNode RndTransformable::OnSetLocalRot(const DataArray *da) {
    SetLocalRot(Vector3(da->Float(2), da->Float(3), da->Float(4)));
    return 0;
}

DataNode RndTransformable::OnSetLocalRotMat(const DataArray *da) {
    Hmx::Matrix3 m(
        da->Float(2),
        da->Float(3),
        da->Float(4),
        da->Float(5),
        da->Float(6),
        da->Float(7),
        da->Float(8),
        da->Float(9),
        da->Float(10)
    );
    SetLocalRot(m);
    return 0;
}

DataNode RndTransformable::OnSetTransConstraint(const DataArray *da) {
    RndTransformable *trans = 0;
    if (da->Size() > 3)
        trans = da->Obj<RndTransformable>(3);
    SetTransConstraint((Constraint)da->Int(2), trans, false);
    return 0;
}

DataNode RndTransformable::OnGetChildren(const DataArray *da) {
    DataArray *arr = new DataArray((int)mChildren.size());
    int idx = 0;
    for (std::list<RndTransformable *>::iterator it = mChildren.begin();
         it != mChildren.end();
         ++it) {
        arr->Node(idx++) = *it;
    }
    DataNode ret(arr, kDataArray);
    arr->Release();
    return ret;
}

bool RndTransformable::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (&mParent == ref) {
        SetTransParent(dynamic_cast<RndTransformable *>(obj), false);
        return true;
    } else
        return Hmx::Object::Replace(ref, obj);
}

void RndTransformable::ComputeLocalXfm(const Transform &tf) {
    if (mParent) {
        Transform tf60;
        MultiplyInverse(tf, WorldXfm(), tf60);
        mLocalXfm = tf60;
    } else {
        mLocalXfm = tf;
    }
    SetDirty();
}

DataNode RndTransformable::OnCopyWorldTransFrom(const DataArray *a) {
    RndTransformable *t = a->Obj<RndTransformable>(2);
    SetWorldXfm(t->WorldXfm());
    return 0;
}

DataNode RndTransformable::OnCopyWorldPosFrom(const DataArray *a) {
    RndTransformable *t = a->Obj<RndTransformable>(2);
    SetWorldPos(t->WorldXfm().v);
    return 0;
}

DataNode RndTransformable::OnGetWorldForward(const DataArray *da) {
    *da->Var(2) = WorldXfm().m.y.x;
    *da->Var(3) = WorldXfm().m.y.y;
    *da->Var(4) = WorldXfm().m.y.z;
    return 0;
}

DataNode RndTransformable::OnGetWorldRight(const DataArray *da) {
    *da->Var(2) = WorldXfm().m.x.x;
    *da->Var(3) = WorldXfm().m.x.y;
    *da->Var(4) = WorldXfm().m.x.z;
    return 0;
}

DataNode RndTransformable::OnGetWorldUp(const DataArray *da) {
    *da->Var(2) = WorldXfm().m.z.x;
    *da->Var(3) = WorldXfm().m.z.y;
    *da->Var(4) = WorldXfm().m.z.z;
    return 0;
}

DataNode RndTransformable::OnGetWorldPos(const DataArray *da) {
    *da->Var(2) = WorldXfm().v.x;
    *da->Var(3) = WorldXfm().v.y;
    *da->Var(4) = WorldXfm().v.z;
    return 0;
}

DataNode RndTransformable::OnGetWorldRot(const DataArray *da) {
    Vector3 v20;
    MakeEuler(WorldXfm().m, v20);
    v20 *= RAD2DEG;
    *da->Var(2) = v20.x;
    *da->Var(3) = v20.y;
    *da->Var(4) = v20.z;
    return 0;
}

DataNode RndTransformable::OnSetLocalRotIndex(const DataArray *a) {
    SetLocalRotIndex(a->Int(2), a->Float(3));
    return 0;
}

RndTransformable::RndTransformable()
    : mParent(this), mTarget(this), mConstraint(kConstraintNone), mPreserveScale(false),
      mDirty(true) {
    mLocalXfm.Reset();
    mWorldXfm.Reset();
}

RndTransformable::~RndTransformable() {
    for (std::list<RndTransformable *>::iterator it = mChildren.begin();
         it != mChildren.end();
         ++it) {
        RndTransformable *cur = *it;
        cur->mParent = nullptr;
        cur->SetDirty();
    }
}

BEGIN_PROPSYNCS(RndTransformable)
    SYNC_PROP_SET(
        trans_parent, mParent.Ptr(), SetTransParent(_val.Obj<RndTransformable>(), true)
    )
    SYNC_PROP_SET(
        trans_constraint,
        mConstraint,
        SetTransConstraint((Constraint)_val.Int(), mTarget, mPreserveScale)
    )
    SYNC_PROP_SET(
        trans_target,
        mTarget.Ptr(),
        SetTransConstraint(mConstraint, _val.Obj<RndTransformable>(), mPreserveScale)
    )
    SYNC_PROP_SET(
        preserve_scale,
        mPreserveScale,
        SetTransConstraint(mConstraint, mTarget, _val.Int())
    )
    SYNC_PROP_MODIFY(local_xfm, mLocalXfm, SetDirty())
    SYNC_PROP_MODIFY(world_xfm, mWorldXfm, ComputeLocalXfm(mWorldXfm))
    if (ClassName() == StaticClassName()) {
        SYNC_SUPERCLASS(Hmx::Object)
    }
END_PROPSYNCS

void RndTransformable::Init() {
    REGISTER_OBJ_FACTORY(RndTransformable);
    DataArray *cfg = SystemConfig("rnd");
    cfg->FindData("shadow_plane", sShadowPlane, true);
}

BEGIN_HANDLERS(RndTransformable)
    HANDLE(copy_local_to, OnCopyLocalTo)
    HANDLE(copy_world_trans_from, OnCopyWorldTransFrom)
    HANDLE(copy_world_pos_from, OnCopyWorldPosFrom)
    HANDLE(set_constraint, OnSetTransConstraint)
    HANDLE(set_local_rot, OnSetLocalRot)
    HANDLE(set_local_rot_index, OnSetLocalRotIndex)
    HANDLE(set_local_rot_mat, OnSetLocalRotMat)
    HANDLE(set_local_pos, OnSetLocalPos)
    HANDLE(set_local_pos_index, OnSetLocalPosIndex)
    HANDLE(get_local_rot, OnGetLocalRot)
    HANDLE(get_local_rot_index, OnGetLocalRotIndex)
    HANDLE(get_local_pos, OnGetLocalPos)
    HANDLE(get_local_pos_index, OnGetLocalPosIndex)
    HANDLE(set_local_scale, OnSetLocalScale)
    HANDLE(set_local_scale_index, OnSetLocalScaleIndex)
    HANDLE(get_local_scale, OnGetLocalScale)
    HANDLE(get_local_scale_index, OnGetLocalScaleIndex)
    HANDLE_ACTION(normalize_local, Normalize(mLocalXfm.m, mLocalXfm.m))
    HANDLE(get_world_forward, OnGetWorldForward)
    HANDLE(get_world_right, OnGetWorldRight)
    HANDLE(get_world_up, OnGetWorldUp)
    HANDLE(get_world_pos, OnGetWorldPos)
    HANDLE(get_world_rot, OnGetWorldRot)
    HANDLE_ACTION(
        set_trans_parent,
        SetTransParent(
            _msg->Obj<RndTransformable>(2), _msg->Size() > 3 ? _msg->Int(3) != 0 : false
        )
    )
    HANDLE_EXPR(trans_parent, mParent.Ptr())
    HANDLE_ACTION(reset_xfm, DirtyLocalXfm().Reset())
    HANDLE_ACTION(
        distribute_children, DistributeChildren(_msg->Int(2) != 0, _msg->Float(3))
    )
    HANDLE(get_children, OnGetChildren)
    if (ClassName() == StaticClassName()) {
        HANDLE_SUPERCLASS(Hmx::Object)
    }
END_HANDLERS
