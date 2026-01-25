#include "rndobj/Trans.h"
#include "Trans.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/TransAnim.h"
#include "rndobj/Utl.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "math/Rot.h"
#include "utl/TextStream.h"

RndTransformable::RndTransformable()
    : mParent(this), mTarget(this), mConstraint(kConstraintNone), mPreserveScale(false),
      mDirty(true) {
    mLocalXfm.Reset();
    mWorldXfm.Reset();
}

RndTransformable::~RndTransformable() {
    if (mParent) {
        mParent->mChildren.remove(this);
    }
    FOREACH (it, mChildren) {
        (*it)->mParent = nullptr;
        (*it)->SetDirty();
    }
}

bool RndTransformable::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mParent == from) {
        SetTransParent(dynamic_cast<RndTransformable *>(to), false);
        return true;
    } else
        return Hmx::Object::Replace(from, to);
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
            _msg->Obj<RndTransformable>(2), _msg->Size() > 3 ? (bool)(_msg->Int(3) != 0) : false
        )
    )
    HANDLE_EXPR(trans_parent, mParent.Ptr())
    HANDLE_ACTION(reset_xfm, DirtyLocalXfm().Reset())
    HANDLE_ACTION(
        distribute_children, DistributeChildren(_msg->Int(2) != 0, _msg->Float(3))
    )
    HANDLE(get_trans_children, OnGetChildren)
    HANDLE_VIRTUAL_SUPERCLASS(Hmx::Object)
END_HANDLERS

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
    SYNC_PROP_MODIFY(world_xfm, mWorldXfm, ComputeLocalXfm(mLocalXfm))
    SYNC_VIRTUAL_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndTransformable)
    SAVE_REVS(9, 0)
    SAVE_VIRTUAL_SUPERCLASS(Hmx::Object)
    bs << mLocalXfm;
    bs << mWorldXfm;
    bs << mConstraint;
    bs << mTarget << mPreserveScale << mParent;
END_SAVES

BEGIN_COPYS(RndTransformable)
    COPY_VIRTUAL_SUPERCLASS(Hmx::Object)
    CREATE_COPY(RndTransformable)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mWorldXfm)
        COPY_MEMBER(mLocalXfm)
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mPreserveScale)
            COPY_MEMBER(mConstraint)
            COPY_MEMBER(mTarget)
        } else if (mConstraint == c->mConstraint) {
            COPY_MEMBER(mTarget)
        }
        SetTransParent(c->mParent, false);
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndTransformable)
    LOAD_REVS(bs)
    int gRev = d.rev;
    ASSERT_REVS(9, 0)
    if (ClassName() == StaticClassName()) {
        Hmx::Object::Load(bs);
    }
    if (gLoadingProxyFromDisk) {
        Transform t;
        bs >> t >> t;
    } else {
        bs >> mLocalXfm >> mWorldXfm;
    }
    if (gRev < 9) {
        ObjPtrList<RndTransformable> l(this);
        bs >> l;
        FOREACH (it, l) {
            (*it)->SetTransParent(this, false);
        }
    }

    switch (gRev) {
    default:
        bs >> (int &)mConstraint;
        break;
    case 7:
    case 8:
        bs >> (int &)mConstraint;
        if (mConstraint == 4) {
            mConstraint = kConstraintNone;
        } else if (mConstraint == 2 || mConstraint == 3 || mConstraint == 4) {
            mConstraint = (Constraint)(mConstraint + kConstraintLocalRotate);
        }
        break;
    case 6:
        bs >> (int &)mConstraint;
        mPreserveScale = mConstraint > kConstraintTargetWorld;
        if (mConstraint > 9) {
            mConstraint = (Constraint)(mConstraint - kConstraintBillboardZ);
        } else if (mConstraint > 2) {
            mConstraint = (Constraint)(mConstraint - kConstraintLocalRotate);
        } else if (mConstraint == 2) {
            mConstraint = kConstraintParentWorld;
        }
        break;
    case 3:
    case 4:
    case 5:
        int unkb0;
        bs >> unkb0;
        mPreserveScale = unkb0;

        switch (unkb0) {
        case 0x4:
        case 0x84:
            mConstraint = kConstraintBillboardZ;
            break;
        case 0x8:
        case 0x88:
            mConstraint = kConstraintBillboardXZ;
            break;
        case 0x10:
        case 0x90:
            mConstraint = kConstraintBillboardXYZ;
            break;
        case 0x20:
        case 0xA0:
            mConstraint = kConstraintFastBillboardXYZ;
            break;
        case 0x40:
            mConstraint = kConstraintLocalRotate;
            break;
        default:
            mConstraint = kConstraintNone;
            break;
        }
        break;
    case 1:
    case 2: {
        int numb4;
        bs >> numb4;
        int sp80[6] = { 0, 0, 0, 5, 6, 7 };
        if (numb4 >= 0x18) {
            mConstraint = kConstraintNone;
        } else {
            mConstraint = (Constraint)sp80[numb4];
        }
        break;
    }
    case 0:
        break;
    }
    if (gRev > 0 && gRev < 7) {
        Vector3 v;
        bs >> v;
        if (!v.IsZero()) {
            MILO_LOG("Transform origin no longer supported\n");
        }
    }
    if (gRev > 1 && gRev < 5) {
        bool b3u;
        d >> b3u;
    }
    if (gRev > 5 && gRev < 8) {
        Sphere s;
        bs >> s;
        RndDrawable *draw = dynamic_cast<RndDrawable *>(this);
        if (draw)
            draw->SetSphere(s);
    }
    if (gRev > 5) {
        if (gLoadingProxyFromDisk) {
            ObjPtr<RndTransformable> tPtr(this);
            tPtr.Load(bs, false, 0);
        } else
            bs >> mTarget;
    }
    if (gRev > 6)
        d >> mPreserveScale;
    if (gRev > 8) {
        ObjPtr<RndTransformable> tPtr(this);
        if (!gLoadingProxyFromDisk) {
            bs >> tPtr;
            SetTransParent(tPtr, false);
        } else
            tPtr.Load(bs, false, 0);
    } else if (gRev > 6) {
        ObjPtr<RndTransformable> tPtr(this);
        bs >> tPtr;
        if (tPtr != this) {
            SetTransParent(tPtr, false);
            mConstraint = kConstraintParentWorld;
        }
    } else if (gRev == 6 && mConstraint == kConstraintParentWorld) {
        SetTransParent(mTarget, false);
    }
END_LOADS

void RndTransformable::Highlight() { UtilDrawAxes(WorldXfm(), 3, Hmx::Color(1, 1, 1)); }

__forceinline TextStream &operator<<(TextStream &ts, RndTransformable::Constraint c) {
    if (c == RndTransformable::kConstraintNone) {
        ts << "None";
    } else if (c == RndTransformable::kConstraintLocalRotate) {
        ts << "LocalRotate";
    } else if (c == RndTransformable::kConstraintLookAtTarget) {
        ts << "LookAtTarget";
    } else if (c == RndTransformable::kConstraintShadowTarget) {
        ts << "ShadowTarget";
    } else if (c == RndTransformable::kConstraintParentWorld) {
        ts << "ParentWorld";
    } else if (c == RndTransformable::kConstraintBillboardZ) {
        ts << "BillboardZ";
    } else if (c == RndTransformable::kConstraintBillboardXZ) {
        ts << "BillboardXZ";
    } else if (c == RndTransformable::kConstraintBillboardXYZ) {
        ts << "BillboardXYZ";
    } else if (c == RndTransformable::kConstraintFastBillboardXYZ) {
        ts << "FastBillboardXYZ";
    }
    return ts;
}

void RndTransformable::Print() {
    TheDebug << "   localXfm: " << mLocalXfm << "\n";
    TheDebug << "   worldXfm: " << mWorldXfm << "\n";
    TheDebug << "   constraint: " << mConstraint << "\n";
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
        FOREACH (it, mChildren) {
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
    FOREACH (it, mChildren) {
        (*it)->SetDirty();
    }
}

void RndTransformable::SetWorldPos(const Vector3 &pos) {
    mWorldXfm.v = pos;
    UpdatedWorldXfm();
    FOREACH (it, mChildren) {
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
    FOREACH (it, mChildren) {
        arr->Node(idx++) = *it;
    }
    DataNode ret(arr);
    arr->Release();
    return ret;
}

void RndTransformable::ComputeLocalXfm(const Transform &tf) {
    if (mParent) {
        Transform tf60;
        MultiplyInverse(tf, mParent->WorldXfm(), tf60);
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

void RndTransformable::Init() {
    REGISTER_OBJ_FACTORY(RndTransformable);
    DataArray *cfg = SystemConfig("rnd");
    cfg->FindData("shadow_plane", sShadowPlane, true);
}

void RndTransformable::DistributeChildren(bool horizontal, float f) {
    std::vector<RndTransformable *> vec;
    FOREACH (it, mChildren) {
        vec.push_back(*it);
    }
    int count = vec.size();
    if (count < 2)
        return;
    else {
        if (horizontal) {
            std::sort(vec.begin(), vec.end(), HorizontalCmp);
        } else {
            std::sort(vec.begin(), vec.end(), VerticalCmp);
            f *= -1;
        }
        int idx = horizontal ? 0 : 2;
        float at = vec[0]->LocalXfm().v[idx];

        for (int i = 1; i < count; i++) {
            Transform t = vec[i]->LocalXfm();
            t.v[idx] = f * i + at;
            vec[i]->SetLocalXfm(t);
        }
    }
}

void RndTransformable::SetLocalRotIndex(int index, float f2) {
    MILO_ASSERT(index < 3, 0x3A4);
    Vector3 v5c;
    Vector3 v68;
    MakeEulerScale(LocalXfm().m, v5c, v68);
    v5c[index] = f2 * DEG2RAD;
    Hmx::Matrix3 m50;
    MakeRotMatrix(v5c, m50, true);
    Scale(v68, m50, m50);
    SetLocalRot(m50);
}

void RndTransformable::TransformTransAnims(const Transform &tf) {
    FOREACH (it, Refs()) {
        RndTransAnim *transAnim = dynamic_cast<RndTransAnim *>(it->RefOwner());
        if (transAnim && transAnim->Trans() == this) {
            TransformKeys(transAnim, tf);
        }
    }
}

void RndTransformable::SetTransParent(RndTransformable *t, bool recalcLocal) {
    MILO_ASSERT(t != this, 0x5D);
    if (mParent != t) {
        if (recalcLocal) {
            Transform tf48;
            Transform tf78;
            if (mParent)
                tf48 = mParent->WorldXfm();
            else
                tf48.Reset();
            if (t)
                tf78 = t->WorldXfm();
            else
                tf78.Reset();
            Invert(tf78, tf78);
            Multiply(tf48, tf78, tf78);
            Multiply(mLocalXfm, tf78, mLocalXfm);
            TransformTransAnims(tf78);
        }
        if (mParent) {
            mParent->mChildren.remove(this);
        }
        mParent = t;
        if (mParent) {
            mParent->mChildren.push_back(this);
        }
    }
    SetDirty();
}

const Transform &RndTransformable::WorldXfm_Force() {
    START_AUTO_TIMER("updateworldxfm");
    mDirty = false;
    if (!mParent) {
        mWorldXfm = mLocalXfm;
    } else if (mConstraint == kConstraintParentWorld) {
        mWorldXfm = mParent->WorldXfm();
    } else if (mConstraint == kConstraintLocalRotate) {
        Multiply(mLocalXfm.v, mParent->WorldXfm(), mWorldXfm.v);
        mWorldXfm.m = mLocalXfm.m;
    } else if (mConstraint == kConstraintNoParentRotation) {
        Add(mLocalXfm.v, mParent->WorldXfm().v, mWorldXfm.v);
        mWorldXfm.m = mLocalXfm.m;
    } else {
        Multiply(mLocalXfm, mParent->WorldXfm(), mWorldXfm);
    }
    if (HasDynamicConstraint())
        ApplyDynamicConstraint();
    else
        UpdatedWorldXfm();
    return mWorldXfm;
}
