#include "rndobj/Group.h"
#include "Rnd.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/Std.h"

bool gInReplace;

struct GroupDrawDist {
    RndDrawable *draw; // 0x0
    float lensq; // 0x4
};

bool SortInWorld(const GroupDrawDist &g1, const GroupDrawDist &g2) {
    return g1.lensq < g2.lensq;
}

RndGroup::RndGroup()
    : mObjects(this, kObjListOwnerControl), mDrawOnly(this), mSortInWorld(false) {}

bool RndGroup::Replace(ObjRef *from, Hmx::Object *to) {
    // theRef is potentially a Node inside of mObjects, inside an iterator
    ObjRef *theRef = from->Parent() == &mObjects ? from : nullptr;
    if (theRef) {
        if (!to) {
            Hmx::Object *theObj = from->GetObj();
            ObjPtrList<Hmx::Object>::iterator it =
                *reinterpret_cast<ObjPtrList<Hmx::Object>::iterator *>(&theRef);
            mObjects.erase(it);
            VectorRemove(mAnims, theObj);
            VectorRemove(mDraws, theObj);
        } else {
            AddObject(to, from->GetObj());
            gInReplace = true;
            RemoveObject(from->GetObj());
            gInReplace = false;
        }
        return true;
    } else {
        return RndTransformable::Replace(from, to);
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

INIT_REVS(0x10, 0)

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
        d >> mObjects;
        if (d.rev < 0x10) {
            ObjPtr<RndEnviron> env(this);
            d >> env;
            if (env) {
                mObjects.push_front(env);
            }
        }
        if (d.rev > 0xC) {
            d >> mDrawOnly;
        } else {
            mDrawOnly = nullptr;
        }
        Update();
    }
    if (d.rev > 0xB && d.rev < 0xF) {
        String str;
        float x;
        d >> str;
        d >> x;
    }
    if (d.rev > 0xD) {
        d >> mSortInWorld;
    }
END_LOADS

void RndGroup::StartAnim() {
    FOREACH (it, mAnims) {
        (*it)->StartAnim();
    }
}

void RndGroup::EndAnim() {
    FOREACH (it, mAnims) {
        (*it)->EndAnim();
    }
}

void RndGroup::SetFrame(float frame, float blend) {
    if (Showing()) {
        RndAnimatable::SetFrame(frame, blend);
        FOREACH (it, mAnims) {
            (*it)->SetFrame(frame, blend);
        }
    }
}

float RndGroup::EndFrame() {
    float end = 0;
    FOREACH (it, mAnims) {
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
        FOREACH (it, mDraws) {
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
        FOREACH (it, mDraws) {
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
    if (Showing()) {
        TheRnd.PushClipPlanes(ClipPlanes());
        RndGroup::DrawShowing();
        TheRnd.PopClipPlanes(ClipPlanes());
    }
}

void RndGroup::DrawShowing() {
    RndEnvironTracker tracker(nullptr, nullptr);
    if (!mSortInWorld) {
        FOREACH (it, mDraws) {
            (*it)->Draw();
        }
    } else if (mDrawOnly) {
        mDrawOnly->Draw();
    } else {
        std::vector<GroupDrawDist> dists;
        dists.reserve(mDraws.size());
        const Vector3 &worldVector = RndCam::Current()->WorldXfm().v;
        FOREACH (it, mDraws) {
            RndTransformable *t = dynamic_cast<RndTransformable *>(*it);
            Vector3 v = t ? t->WorldXfm().v : Vector3(0, 0, 0);
            GroupDrawDist curDist;
            curDist.draw = *it;
            Vector3 diff;
            Subtract(worldVector, v, diff);
            curDist.lensq = LengthSquared(diff);
            dists.push_back(curDist);
        }
        std::sort(dists.begin(), dists.end(), SortInWorld);
        FOREACH (it, dists) {
            it->draw->Draw();
        }
    }
}

void RndGroup::ListDrawChildren(std::list<RndDrawable *> &children) {
    children.insert(children.end(), mDraws.begin(), mDraws.end());
}

RndDrawable *RndGroup::CollideShowing(const Segment &seg, float &f, Plane &p) {
    RndDrawable *ret = nullptr;
    Segment localseg(seg);
    f = 1.0f;
    FOREACH (it, mDraws) {
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

int RndGroup::CollidePlane(const Plane &p) {
    int ret = -1;
    bool b2 = false;
    FOREACH (it, mDraws) {
        Sphere s;
        if ((*it)->Showing() && (*it)->MakeWorldSphere(s, false)) {
            if (!b2) {
                ret = (*it)->CollidePlane(p);
                b2 = true;

            } else if (ret != (*it)->CollidePlane(p)) {
                return 0;
            }
        }
    }
    return ret;
}

void RndGroup::CollideList(const Segment &seg, std::list<Collision> &colls) {
    if (Showing()) {
        FOREACH (it, mDraws) {
            (*it)->CollideList(seg, colls);
        }
    }
}

void RndGroup::Update() {
    mAnims.clear();
    mDraws.clear();
    FOREACH (it, mObjects) {
        RndAnimatable *anim = dynamic_cast<RndAnimatable *>(*it);
        if (anim) {
            mAnims.push_back(anim);
        }
        RndDrawable *draw = dynamic_cast<RndDrawable *>(*it);
        if (draw) {
            mDraws.push_back(draw);
        }
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
            if (anim) {
                mAnims.push_back(anim);
            }
            RndDrawable *draw = dynamic_cast<RndDrawable *>(o1);
            if (draw) {
                mDraws.push_back(draw);
            }
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
        FOREACH (it, group->mObjects) {
            AddObject(*it);
        }
    }
}

int RndGroup::MoveObject(Hmx::Object *obj, int idx) {
    auto found = mObjects.find(obj);
    if (found == mObjects.end()) {
        return 0;
    } else {
        auto otherIt = found;
        int newIdx = idx;
        if (idx > 0) {
            ++otherIt;
            for (; newIdx != 0 && otherIt != mObjects.end(); ++otherIt, --newIdx) {
            }
        } else if (idx != 0) {
            for (; newIdx != 0 && otherIt != mObjects.begin(); --otherIt, ++newIdx) {
            }
        }
        mObjects.MoveItem(otherIt, mObjects, found);
        Update();
        return idx - newIdx;
    }
}

void RndGroup::SortDraws() {
    FOREACH (it, mDraws) {
        mObjects.remove(*it);
    }
    std::sort(mDraws.begin(), mDraws.end(), ::SortDraws);
    FOREACH (it, mDraws) {
        mObjects.push_back(*it);
    }
    mAnims.clear();
    FOREACH (it, mObjects) {
        RndAnimatable *anim = dynamic_cast<RndAnimatable *>(*it);
        if (anim)
            mAnims.push_back(anim);
    }
}

DataNode RndGroup::GetGroupChildren() {
    DataArrayPtr ptr(new DataArray(mObjects.size()));
    int idx = 0;
    FOREACH (it, mObjects) {
        ptr->Node(idx) = *it;
        idx++;
    }
    return ptr;
}
