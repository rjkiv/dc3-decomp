#include "rndobj/Trans.h"
#include "Trans.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/System.h"
#include "rndobj/Cam.h"
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
            _msg->Obj<RndTransformable>(2),
            _msg->Size() > 3 ? (bool)(_msg->Int(3) != 0) : false
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
    SYNC_PROP_MODIFY(world_xfm, mWorldXfm, SyncWorldXfm())
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

INIT_REVS(9, 0)

BEGIN_LOADS(RndTransformable)
    LOAD_REVS(bs)
    ASSERT_REVS(9, 0)
    LOAD_VIRTUAL_SUPERCLASS(Hmx::Object)
    if (gLoadingProxyFromDisk) {
        Transform t;
        d >> t >> t;
    } else {
        d >> mLocalXfm >> mWorldXfm;
    }
    if (d.rev < 9) {
        ObjPtrList<RndTransformable> l(this);
        d >> l;
        FOREACH (it, l) {
            (*it)->SetTransParent(this, false);
        }
    }

    if (d.rev > 8) {
        d >> (int &)mConstraint;
    } else if (d.rev > 6) {
        d >> (int &)mConstraint;
        if (mConstraint == 4) {
            mConstraint = kConstraintNone;
        } else if (mConstraint > 1 && mConstraint < 5) {
            mConstraint = (Constraint)(mConstraint + kConstraintLocalRotate);
        }
    } else if (d.rev == 6) {
        d >> (int &)mConstraint;
        mPreserveScale = mConstraint > kConstraintTargetWorld;
        if (mConstraint > 9) {
            mConstraint = (Constraint)(mConstraint - kConstraintBillboardZ);
        } else if (mConstraint > 2) {
            mConstraint = (Constraint)(mConstraint - kConstraintLocalRotate);
        } else if (mConstraint == 2) {
            mConstraint = kConstraintParentWorld;
        }
    } else if (d.rev >= 3) {
        int unkb0;
        d >> unkb0;
        mPreserveScale = unkb0 & 0x80;
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
    } else if (d.rev > 0) {
        unsigned int numb4;
        d >> numb4;
        int sp80[6] = { 0, 0, 0, 5, 6, 7 };
        if (numb4 >= 0x18) {
            mConstraint = kConstraintNone;
        } else {
            mConstraint = (Constraint)sp80[numb4];
        }
    }
    if (d.rev > 0 && d.rev < 7) {
        Vector3 v;
        d >> v;
        bool isZero = v == Vector3(0, 0, 0);
        if (!isZero) {
            MILO_LOG("Transform origin no longer supported\n");
        }
    }
    if (d.rev > 1 && d.rev < 5) {
        bool b3u;
        d >> b3u;
    }
    if (d.rev > 5 && d.rev < 8) {
        Sphere s;
        d >> s;
        RndDrawable *draw = dynamic_cast<RndDrawable *>(this);
        if (draw)
            draw->SetSphere(s);
    }
    if (d.rev > 5) {
        if (gLoadingProxyFromDisk) {
            ObjPtr<RndTransformable> tPtr(this);
            tPtr.Load(d.stream, false, nullptr);
        } else
            d >> mTarget;
    }
    if (d.rev > 6)
        d >> mPreserveScale;
    if (d.rev > 8) {
        ObjPtr<RndTransformable> tPtr(this);
        if (!gLoadingProxyFromDisk) {
            d >> tPtr;
            SetTransParent(tPtr, false);
        } else {
            tPtr.Load(d.stream, false, nullptr);
        }
    } else if (d.rev > 6) {
        ObjPtr<RndTransformable> tPtr(this);
        d >> tPtr;
        if (tPtr != this) {
            SetTransParent(tPtr, false);
            mConstraint = kConstraintParentWorld;
        }
    } else if (d.rev == 6 && mConstraint == kConstraintParentWorld) {
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

void RndTransformable::ApplyDynamicConstraint() {
    if (mConstraint == kConstraintTargetWorld) {
        if (mTarget) {
            mWorldXfm = mTarget->WorldXfm();
        }
    } else if (mConstraint == kConstraintShadowTarget) {
        Transform tf40;
        if (mTarget) {
            Transpose(mTarget->WorldXfm(), tf40);
            Multiply(mWorldXfm, tf40, mWorldXfm);
        } else {
            tf40.Reset();
        }
        Plane pl50;
        Multiply(sShadowPlane, tf40, pl50);
        float planeB;
        if (pl50.b != 0) {
            planeB = 1 / pl50.b;
        } else {
            planeB = 0.001f;
        }
        tf40.m.Set(1, -pl50.a * planeB, 0, 0, 0, 0, 0, -pl50.c * planeB, 1);
        tf40.v.Set(0, -pl50.d * planeB, 0);
        Multiply(mWorldXfm, tf40, mWorldXfm);
        Multiply(mWorldXfm, mTarget->WorldXfm(), mWorldXfm);
    } else if (RndCam::Current()) {
        Vector3 v60;
        RndTransformable *cur = mTarget ? mTarget.Ptr() : RndCam::Current();
        const Transform &curWorld = cur->WorldXfm();
        if (mPreserveScale) {
            MakeScale(mWorldXfm.m, v60);
        }
        switch (mConstraint) {
        case kConstraintLookAtTarget:
            if (mTarget) {
                Subtract(mTarget->WorldXfm().v, mWorldXfm.v, mWorldXfm.m.y);
                Normalize(mWorldXfm.m, mWorldXfm.m);
            }
            break;
        case kConstraintBillboardZ:
            Subtract(mWorldXfm.v, curWorld.v, mWorldXfm.m.y);
            if (mPreserveScale) {
                Normalize(mWorldXfm.m.z, mWorldXfm.m.z);
            }
            Cross(mWorldXfm.m.y, mWorldXfm.m.z, mWorldXfm.m.x);
            Normalize(mWorldXfm.m.x, mWorldXfm.m.x);
            Cross(mWorldXfm.m.z, mWorldXfm.m.x, mWorldXfm.m.y);
            break;
        case kConstraintBillboardXZ:
            Subtract(mWorldXfm.v, curWorld.v, mWorldXfm.m.y);
            Normalize(mWorldXfm.m.y, mWorldXfm.m.y);
            Cross(mWorldXfm.m.y, mWorldXfm.m.z, mWorldXfm.m.x);
            Normalize(mWorldXfm.m.x, mWorldXfm.m.x);
            Cross(mWorldXfm.m.x, mWorldXfm.m.y, mWorldXfm.m.z);
            break;
        case kConstraintBillboardXYZ:
            Subtract(mWorldXfm.v, curWorld.v, mWorldXfm.m.y);
            mWorldXfm.m.z = curWorld.m.z;
            Normalize(mWorldXfm.m, mWorldXfm.m);
            break;
        case kConstraintFastBillboardXYZ:
            mWorldXfm.m = curWorld.m;
            break;
        case kConstraintSkyBox:
            Add(mLocalXfm.v, curWorld.v, mWorldXfm.v);
            mWorldXfm.m = mLocalXfm.m;
            break;
        case kConstraintSkyBoxXY:
            Add(mLocalXfm.v, curWorld.v, mWorldXfm.v);
            mWorldXfm.v.z = mLocalXfm.v.z;
            mWorldXfm.m = mLocalXfm.m;
            break;
        }
        if (mPreserveScale) {
            Scale(v60, mWorldXfm.m, mWorldXfm.m);
        }
    }
    SetDirty_Force();
}
