#include "rndobj/Group.h"

#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/Std.h"

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

BEGIN_PROPSYNCS(RndGroup)
    SYNC_PROP_MODIFY(objects, mObjects, Update())
    SYNC_PROP(draw_only, mDrawOnly)
    SYNC_PROP(sort_in_world, mSortInWorld)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

RndGroup::RndGroup()
    : mObjects(this, kObjListOwnerControl), mDrawOnly(this), mSortInWorld(false) {}
