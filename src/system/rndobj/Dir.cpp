#include "rndobj/Dir.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Group.h"
#include "rndobj/Poll.h"
#include "rndobj/PostProc.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

RndDir::RndDir() : mEnv(this) {}

bool RndDir::Replace(ObjRef *ref, Hmx::Object *obj) {
    return RndTransformable::Replace(ref, obj);
}

BEGIN_HANDLERS(RndDir)
    HANDLE(show_objects, OnShowObjects)
    HANDLE(supported_events, OnSupportedEvents)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(ObjectDir)
END_HANDLERS

BEGIN_PROPSYNCS(RndDir)
    SYNC_PROP(environ, mEnv)
    SYNC_PROP(polls, mPolls)
    SYNC_PROP(enters, mEnters)
    SYNC_PROP(draws, mDraws)
    SYNC_PROP(test_event, mTestEvent)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(ObjectDir)
END_PROPSYNCS

BEGIN_SAVES(RndDir)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(ObjectDir)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mEnv << mTestEvent;
END_SAVES

BEGIN_COPYS(RndDir)
    COPY_SUPERCLASS(ObjectDir)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndDrawable)
    COPY_SUPERCLASS(RndTransformable)
    CREATE_COPY(RndDir)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mEnv)
            COPY_MEMBER(mTestEvent)
        }
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(RndDir)
    ObjectDir::Load(bs);
END_LOADS

void RndDir::Export(DataArray *a, bool b2) {
    Hmx::Object::Export(a, b2);
    for (int i = 0; i < mSubDirs.size(); i++) {
        if (mSubDirs[i]) {
            mSubDirs[i]->Export(a, false);
        }
    }
}

void RndDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(10, 0)
    ObjectDir::PreLoad(bs);
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void RndDir::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    ObjectDir::PostLoad(bs);
    RndAnimatable::Load(d.stream);
    RndDrawable::Load(d.stream);
    if (d.rev > 0) {
        RndTransformable::Load(d.stream);
    }
    if (d.rev > 1) {
        if (gLoadingProxyFromDisk) {
            ObjPtr<RndEnviron> env(this);
            env.Load(d.stream, false, nullptr);
        } else {
            d.stream >> mEnv;
        }
    }
    if (d.rev > 2 && d.rev != 9) {
        d.stream >> mTestEvent;
    }
    if (d.rev > 3 && d.rev < 9) {
        Symbol s;
        d.stream >> s;
        d.stream >> s;
    }
    if (d.rev > 4 && d.rev < 8) {
        RndPostProc *pp = Hmx::Object::New<RndPostProc>();
        pp->LoadRev(d);
        delete pp;
    }
}

void RndDir::SetSubDir(bool b1) {
    ObjectDir::SetSubDir(b1);
    mDraws.clear();
    mPolls.clear();
    mEnters.clear();
    mAnims.clear();
}

void RndDir::SyncObjects() {
    mAnims.clear();
    mPolls.clear();
    mEnters.clear();
    for (int i = 0; i < mSubDirs.size(); i++) {
        ObjectDir *curSubDir = mSubDirs[i];
        if (curSubDir
            && (curSubDir->InlineSubDirType() == kInlineCached
                || curSubDir->InlineSubDirType() == kInlineAlways)) {
            RndTransformable *t = dynamic_cast<RndTransformable *>(curSubDir);
            if (t) {
                t->DirtyLocalXfm().Reset();
                t->SetTransParent(this, false);
            }
        }
    }
    if (!IsSubDir()) {
        SyncDrawables();
        std::list<RndAnimatable *> animchildren;
        for (ObjDirItr<RndAnimatable> it(this, true); it != nullptr; ++it) {
            if (it != this) {
                mAnims.push_back(it);
                it->ListAnimChildren(animchildren);
            }
        }
        for (std::list<RndAnimatable *>::const_iterator it = animchildren.begin();
             it != animchildren.end();
             ++it) {
            VectorRemove(mAnims, *it);
        }
        std::vector<RndPollable *> pollchildren;
        HarvestPollables(pollchildren);
        int numTotalChildren = pollchildren.size();
        int numEnabled = 0;
        for (; (unsigned int)numEnabled < (unsigned int)numTotalChildren
               && pollchildren[numEnabled]->PollEnabled();
             numEnabled++)
            ;
        int numRemaining = numTotalChildren - numEnabled;
        mPolls.resize(numEnabled);
        if (numEnabled != 0) {
            memcpy(
                mPolls.begin(), pollchildren.begin(), numEnabled * sizeof(RndPollable *)
            );
        }
        mEnters.resize(numRemaining);
        if (numRemaining != 0) {
            memcpy(
                mEnters.begin(),
                pollchildren.begin() + numEnabled,
                numRemaining * sizeof(RndPollable *)
            );
        }
        if (IsProxy() && Dir()) {
            ChainSourceSubdir(Dir(), this);
        }
        ObjectDir::SyncObjects();
    }
}

void RndDir::RemovingObject(Hmx::Object *obj) {
    ObjectDir::RemovingObject(obj);
    VectorRemove(mDraws, obj);
    VectorRemove(mPolls, obj);
    VectorRemove(mEnters, obj);
    VectorRemove(mAnims, obj);
}

void RndDir::OldLoadProxies(BinStream &bs, int rev) {
    int numItems;
    bs >> numItems;
    for (int i = 0; i < numItems; i++) {
        FilePath path;
        String name;
        Transform localXfm;
        String transName;
        String envName;
        bs >> path;
        bs >> name;
        bs >> localXfm;
        if (rev > 11) {
            bs >> transName;
        } else {
            transName = 0;
        }
        bs >> envName;
        float order;
        bs >> order;
        bool showing;
        if (rev > 11) {
            bs >> showing;
        } else {
            showing = true;
        }
        RndDir *loadedDir =
            dynamic_cast<RndDir *>(DirLoader::LoadObjects(path, nullptr, nullptr));
        MILO_ASSERT(!name.empty(), 0x22A);
        loadedDir->SetName(name.c_str(), this);
        loadedDir->SetLocalXfm(localXfm);
        loadedDir->SetTransParent(Find<RndTransformable>(transName.c_str(), false), false);
        loadedDir->SetOrder(order);
        loadedDir->SetShowing(showing);
        loadedDir->SetEnv(Find<RndEnviron>(envName.c_str(), false));
        loadedDir->SetProxyFile(path, true);
    }
}

void RndDir::ChainSourceSubdir(Hmx::Object *obj, ObjectDir *dir) {
    if (dir && dir->Sinks()) {
        Hmx::Object::ChainSource(obj, dir);
        for (int i = 0; i < dir->SubDirs().size(); i++) {
            ChainSourceSubdir(obj, dir->SubDirs()[i]);
        }
    }
}

void RndDir::CollideListSubParts(const Segment &seg, std::list<Collision> &colls) {
    if (CollideSphere(seg)) {
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            (*it)->CollideList(seg, colls);
        }
    }
}

void RndDir::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    Transform tf;
    FastInvert(WorldXfm(), tf);
    Multiply(s, tf, s);
    SetSphere(s);
}

float RndDir::GetDistanceToPlane(const Plane &plane, Vector3 &vec) {
    if (mDraws.empty())
        return 0;
    else {
        float f5 = 0;
        bool first = true;
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            Vector3 drawvec;
            float f6 = (*it)->GetDistanceToPlane(plane, drawvec);
            if (first || std::fabs(f6) < std::fabs(f5)) {
                first = false;
                f5 = f6;
                vec = drawvec;
            }
        }
        return f5;
    }
}

bool RndDir::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        s.Zero();
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            Sphere locSphere;
            (*it)->MakeWorldSphere(locSphere, true);
            s.GrowToContain(locSphere);
        }
        return true;
    } else {
        if (mSphere.GetRadius()) {
            Multiply(mSphere, WorldXfm(), s);
            return true;
        } else
            return false;
    }
}

void RndDir::DrawShowing() {
    if (!mDraws.empty()) {
        RndEnvironTracker tracker(mEnv, &WorldXfm().v);
        for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
             ++it) {
            (*it)->Draw();
        }
    }
}

void RndDir::ListDrawChildren(std::list<RndDrawable *> &children) {
    if (!IsProxy()) {
        children.insert(children.end(), mDraws.begin(), mDraws.end());
    }
}

RndDrawable *RndDir::CollideShowing(const Segment &s, float &fl, Plane &pl) {
    RndDrawable *ret = nullptr;
    Segment seg(s);
    fl = 1.0f;
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        float locf;
        RndDrawable *curCollide = (*it)->Collide(seg, locf, pl);
        if (curCollide) {
            if (IsProxy()) {
                fl = locf;
                return this;
            }
            ret = curCollide;
            Interp(seg.start, seg.end, locf, seg.end);
            fl *= locf;
        }
    }
    return ret;
}

int RndDir::CollidePlane(const Plane &pl) {
    int ret = -1;
    bool b2 = false;
    for (std::vector<RndDrawable *>::iterator it = mDraws.begin(); it != mDraws.end();
         ++it) {
        Sphere s;
        if ((*it)->Showing()) {
            if (dynamic_cast<RndGroup *>(*it) || (*it)->MakeWorldSphere(s, false)) {
                if (!b2) {
                    ret = (*it)->CollidePlane(pl);
                    b2 = true;
                } else if (ret != (*it)->CollidePlane(pl)) {
                    return 0;
                }
            }
        }
    }
    return ret;
}

void RndDir::CollideList(const Segment &seg, std::list<Collision> &colls) {
    if (IsProxy() && !sForceSubpartSelection) {
        RndDrawable::CollideList(seg, colls);
    } else
        CollideListSubParts(seg, colls);
}

void RndDir::SetFrame(float frame, float blend) {
    if (Showing()) {
        RndAnimatable::SetFrame(frame, blend);
        for (std::vector<RndAnimatable *>::iterator it = mAnims.begin();
             it != mAnims.end();
             ++it) {
            (*it)->SetFrame(frame, blend);
        }
    }
}

float RndDir::EndFrame() {
    float frame = 0.0f;
    for (std::vector<RndAnimatable *>::iterator it = mAnims.begin(); it != mAnims.end();
         ++it) {
        frame = Max(frame, (*it)->EndFrame());
    }
    return frame;
}

void RndDir::ListAnimChildren(std::list<RndAnimatable *> &children) const {
    if (!IsProxy()) {
        children.insert(children.end(), mAnims.begin(), mAnims.end());
    }
}

void RndDir::Poll() {
    if (Showing()) {
        for (std::vector<RndPollable *>::iterator it = mPolls.begin(); it != mPolls.end();
             ++it) {
            (*it)->Poll();
        }
    }
}

void RndDir::Enter() {
    if (TheLoadMgr.EditMode()) {
        DataNode events = OnSupportedEvents(0);
        DataArray *arr = events.Array();
        if (!arr->Contains(mTestEvent)) {
            mTestEvent = "";
        }
    }
    for (std::vector<RndPollable *>::iterator it = mEnters.begin(); it != mEnters.end();
         ++it) {
        (*it)->Enter();
    }
    for (std::vector<RndPollable *>::iterator it = mPolls.begin(); it != mPolls.end();
         ++it) {
        (*it)->Enter();
    }
    if (IsProxy() && Dir()) {
        ChainSourceSubdir(Dir(), this);
    }
    RndPollable::Enter();
}

void RndDir::Exit() {
    for (std::vector<RndPollable *>::iterator it = mEnters.begin(); it != mEnters.end();
         ++it) {
        (*it)->Exit();
    }
    for (std::vector<RndPollable *>::iterator it = mPolls.begin(); it != mPolls.end();
         ++it) {
        (*it)->Exit();
    }
    RndPollable::Exit();
}

void RndDir::ListPollChildren(std::list<RndPollable *> &children) const {
    if (!IsProxy()) {
        children.insert(children.end(), mPolls.begin(), mPolls.end());
        children.insert(children.end(), mEnters.begin(), mEnters.end());
    }
}

void RndDir::SyncDrawables() {
    mDraws.clear();
    if (!IsSubDir()) {
        std::list<RndDrawable *> drawchildren;
        for (ObjDirItr<RndDrawable> it(this, true); it != nullptr; ++it) {
            if (it != this) {
                mDraws.push_back(it);
                it->ListDrawChildren(drawchildren);
                it->UpdatePreClearState();
            }
        }
        UpdatePreClearState();
        for (std::list<RndDrawable *>::const_iterator it = drawchildren.begin();
             it != drawchildren.end();
             ++it) {
            VectorRemove(mDraws, *it);
        }
        std::sort(mDraws.begin(), mDraws.end(), SortDraws);
    }
}

void RndDir::HarvestPollables(std::vector<RndPollable *> &polls) {
    MemTemp tmp;
    std::list<RndPollable *> pollchildren;
    for (ObjDirItr<RndPollable> it(this, true); it != nullptr; ++it) {
        if (it != this) {
            polls.push_back(it);
            it->ListPollChildren(pollchildren);
        }
    }
    for (std::list<RndPollable *>::const_iterator it = pollchildren.begin();
         it != pollchildren.end();
         ++it) {
        VectorRemove(polls, *it);
    }
    std::sort(polls.begin(), polls.end(), SortPolls);
}

DataNode RndDir::OnShowObjects(DataArray *da) {
    DataArray *array = da->Array(2);
    bool show = da->Int(3);
    for (int i = 0; i < array->Size(); i++) {
        RndDrawable *d = array->Obj<RndDrawable>(i);
        if (d)
            d->SetShowing(show);
    }
    return 0;
}

DataNode RndDir::OnSupportedEvents(DataArray *) {
    DataArrayPtr ptr(new DataArray(0x400));
    std::list<DataArray *> oList;
    int idx = 0;
    ptr->Node(idx++) = Symbol();
    for (ObjDirItr<EventTrigger> it(this, true); it != nullptr; ++it) {
        DataArray *events = it->SupportedEvents();
        if (!ListFind(oList, events)) {
            oList.push_back(events);
            for (int i = 0; i < events->Size(); i++) {
                ptr->Node(idx++) = events->Node(i);
            }
        }
    }
    ptr->Resize(idx);
    return ptr;
}
