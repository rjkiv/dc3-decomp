#include "rndobj/Group.h"
#include "Rnd.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/Std.h"

bool gInReplace;

RndGroup::RndGroup()
    : mObjects(this, kObjListOwnerControl), mDrawOnly(this), mSortInWorld(false) {}

bool RndGroup::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (ref->Parent() != &mObjects) {
        if (!obj) {
            Hmx::Object *theObj = ref->GetObj();
            mObjects.remove(theObj);
            VectorRemove(mAnims, theObj);
            VectorRemove(mDraws, theObj);
        } else {
            AddObject(obj, ref->GetObj());
            gInReplace = true;
            RemoveObject(ref->GetObj());
            gInReplace = false;
        }
        return true;
    } else {
        return RndTransformable::Replace(ref, obj);
    }
}

BEGIN_HANDLERS(RndGroup)
    HANDLE_ACTION(sort_draws, SortDraws())
    HANDLE_ACTION(add_object, AddObject(_msg->Obj<Hmx::Object>(2)))
    HANDLE_ACTION(remove_object, RemoveObject(_msg->Obj<Hmx::Object>(2)))
    HANDLE_ACTION(clear_objects, ClearObjects())
    HANDLE_ACTION(
        insert_object, AddObject(_msg->Obj<Hmx::Object>(2), _msg->Obj<Hmx::Object>(3))
    )
    HANDLE_ACTION(move_object, MoveObject(_msg->Obj<Hmx::Object>(2), _msg->Int(3)))
    HANDLE_EXPR(num_objects, mObjects.size())
    HANDLE_EXPR(has_object, mObjects.find(_msg->Obj<Hmx::Object>(2)) != mObjects.end())
    HANDLE_EXPR(get_group_children, GetGroupChildren())
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndGroup)
    SYNC_PROP_MODIFY(objects, mObjects, Update())
    SYNC_PROP(draw_only, mDrawOnly)
    SYNC_PROP(sort_in_world, mSortInWorld)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndGroup)
    SAVE_REVS(0x10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mObjects;
    bs << mDrawOnly;
    bs << mSortInWorld;
END_SAVES

BEGIN_COPYS(RndGroup)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(RndGroup)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDrawOnly)
        COPY_MEMBER(mSortInWorld)
        if (ty == kCopyDeep)
            COPY_MEMBER(mObjects)
        else if (ty == kCopyFromMax)
            Merge(c);
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(RndGroup)
    LOAD_REVS(bs)
    ASSERT_REVS(0x10, 0)
    if (d.rev > 7) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndAnimatable)
    LOAD_SUPERCLASS(RndTransformable)
    LOAD_SUPERCLASS(RndDrawable)
    if (d.rev > 10) {
        bs >> mObjects;
        if (d.rev < 0x10) {
            ObjPtr<RndEnviron> env(this);
            bs >> env;
            if (env) {
                mObjects.push_back(env);
            }
        }
        if (d.rev > 0xC) {
            bs >> mDrawOnly;
        } else {
            mDrawOnly = nullptr;
        }
        Update();
    }
    if (d.rev > 0xB && d.rev < 0xF) {
        String str;
        float x;
        bs >> str;
        bs >> x;
    }
    if (d.rev > 0xD) {
        d >> mSortInWorld;
    }
END_LOADS

void RndGroup::StartAnim() {
    for (std::vector<RndAnimatable *>::iterator it = mAnims.begin(); it != mAnims.end();
         ++it) {
        (*it)->StartAnim();
    }
}

void RndGroup::EndAnim() {
    for (std::vector<RndAnimatable *>::iterator it = mAnims.begin(); it != mAnims.end();
         ++it) {
        (*it)->EndAnim();
    }
}

void RndGroup::SetFrame(float frame, float blend) {
    if (Showing()) {
        RndAnimatable::SetFrame(frame, blend);
        for (std::vector<RndAnimatable *>::iterator it = mAnims.begin();
             it != mAnims.end();
             ++it) {
            (*it)->SetFrame(frame, blend);
        }
    }
}

float RndGroup::EndFrame() {
    float end = 0;
    for (std::vector<RndAnimatable *>::iterator it = mAnims.begin(); it != mAnims.end();
         ++it) {
        end = Max(end, (*it)->EndFrame());
    }
    return end;
}

void RndGroup::ListAnimChildren(std::list<RndAnimatable *> &children) const {
    children.insert(children.end(), mAnims.begin(), mAnims.end());
}

float RndGroup::GetDistanceToPlane(const Plane &p, Vector3 &v) {
    if (mDraws.empty())
        return 0;
    else {
        float ret = 0;
        bool first = true;
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            Vector3 locvec;
            float dist = (*it)->GetDistanceToPlane(p, locvec);
            if (first || (std::fabs(dist) < std::fabs(ret))) {
                first = false;
                ret = dist;
                v = locvec;
            }
        }
        return ret;
    }
}

bool RndGroup::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        s.Zero();
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            Sphere local_s;
            (*it)->MakeWorldSphere(local_s, true);
            s.GrowToContain(local_s);
        }
        return true;
    } else {
        return false;
    }
}

void RndGroup::Draw() {
    if (mShowing) {
        TheRnd.PushClipPlanes(mClipPlanes);
        RndGroup::DrawShowing();
        TheRnd.PopClipPlanes(mClipPlanes);
    }
}

void RndGroup::ListDrawChildren(std::list<RndDrawable *> &children) {
    children.insert(children.end(), mDraws.begin(), mDraws.end());
}

RndDrawable *RndGroup::CollideShowing(const Segment &seg, float &f, Plane &p) {
    RndDrawable *ret = nullptr;
    Segment localseg(seg);
    f = 1.0f;
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        float locf;
        RndDrawable *collided = (*it)->Collide(localseg, locf, p);
        if (collided) {
            ret = collided;
            Interp(localseg.start, localseg.end, locf, localseg.end);
            f *= locf;
        }
    }
    return ret;
}

void RndGroup::CollideList(const Segment &seg, std::list<Collision> &colls) {
    if (mShowing) {
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            (*it)->CollideList(seg, colls);
        }
    }
}

void RndGroup::Update() {
    mAnims.clear();
    mDraws.clear();
    for (ObjPtrList<Hmx::Object>::iterator it = mObjects.begin(); it != mObjects.end();
         ++it) {
        RndAnimatable *anim = dynamic_cast<RndAnimatable *>(*it);
        if (anim)
            mAnims.push_back(anim);
        RndDrawable *draw = dynamic_cast<RndDrawable *>(*it);
        if (draw)
            mDraws.push_back(draw);
    }
    if (mDrawOnly && !VectorFind(mDraws, mDrawOnly.Ptr())) {
        mDrawOnly = nullptr;
    }
}

void RndGroup::AddObject(Hmx::Object *o1, Hmx::Object *o2) {
    if (o1 && o1 != this) {
        if (mObjects.find(o1) != mObjects.end()) {
            if (!o2)
                return;
            RemoveObject(o1);
        }
        if (o2) {
            mObjects.insert(mObjects.find(o2), o1);
            Update();
        } else {
            mObjects.push_back(o1);
            RndAnimatable *anim = dynamic_cast<RndAnimatable *>(o1);
            if (anim)
                mAnims.push_back(anim);
            RndDrawable *draw = dynamic_cast<RndDrawable *>(o1);
            if (draw)
                mDraws.push_back(draw);
        }
    }
}

void RndGroup::RemoveObject(Hmx::Object *obj) {
    mObjects.remove(obj);
    VectorRemove(mDraws, obj);
    VectorRemove(mAnims, obj);
    if (mDrawOnly == obj && !gInReplace) {
        mDrawOnly = nullptr;
    }
}

void RndGroup::ClearObjects() {
    mObjects.clear();
    Update();
}

void RndGroup::Merge(const RndGroup *group) {
    if (group) {
        for (ObjPtrList<Hmx::Object>::iterator it = group->mObjects.begin();
             it != group->mObjects.end();
             ++it) {
            AddObject(*it);
        }
    }
}

void RndGroup::SortDraws() {
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        mObjects.remove(*it);
    }
    std::sort(mDraws.begin(), mDraws.end(), ::SortDraws);
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        mObjects.push_back(*it);
    }
    mAnims.clear();
    for (ObjPtrList<Hmx::Object>::iterator it = mObjects.begin(); it != mObjects.end();
         ++it) {
        RndAnimatable *anim = dynamic_cast<RndAnimatable *>(*it);
        if (anim)
            mAnims.push_back(anim);
    }
}

DataNode RndGroup::GetGroupChildren() {
    DataArrayPtr ptr(new DataArray(mObjects.size()));
    int idx = 0;
    for (ObjPtrList<Hmx::Object>::iterator it = mObjects.begin(); it != mObjects.end();
         ++it, ++idx) {
        ptr->Node(idx) = *it;
    }
    return ptr;
}
