#include "world/Instance.h"
#include "math/Rot.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Utl.h"
#include "utl/BinStream.h"

#pragma region WorldInstance

WorldInstance::WorldInstance() : mSharedGroup(0), mSharedGroup2(0) {}

WorldInstance::~WorldInstance() {
    if (mSharedGroup2)
        mSharedGroup2->ClearPollMaster();
    delete mSharedGroup2;
}

BEGIN_HANDLERS(WorldInstance)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_PROPSYNCS(WorldInstance)
    SYNC_PROP_MODIFY(instance_file, mDir, SyncDir())
    SYNC_PROP_SET(shared_group, mSharedGroup ? mSharedGroup->Group() : nullptr, )
    SYNC_PROP_SET(poll_master, mSharedGroup ? mSharedGroup->PollMaster() == this : false, )
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(WorldInstance)
    SAVE_REVS(3, 0)
    bs << mDir.GetFile();
    SaveInlined(mDir.GetFile(), true, kInlineCachedShared);
    SAVE_SUPERCLASS(RndDir)
    SavePersistentObjects(bs);
END_SAVES

BEGIN_COPYS(WorldInstance)
    COPY_SUPERCLASS(RndDir)
END_COPYS

BEGIN_LOADS(WorldInstance)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void WorldInstance::PreSave(BinStream &) {}
void WorldInstance::PostSave(BinStream &bs) { SyncDir(); }

INIT_REVS(3, 0)

void WorldInstance::PreLoad(BinStream &bs) {
    if (IsProxy())
        DeleteObjects();
    LOAD_REVS(bs);
    ASSERT_REVS(3, 0);
    if (d.rev > 0) {
        FilePath fp;
        d >> fp;
        PreLoadInlined(fp, true, kInlineCachedShared);
    } else
        d >> mDir;

    RndDir::PreLoad(d.stream);
    if (ObjectDir::ProxyFile().length() != 0) {
        MILO_NOTIFY(
            "WorldInstance %s was created as RndDir. Object needs to be deleted and recreated.",
            Name()
        );
    }
    d.PushRev(this);
}

void WorldInstance::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    RndDir::PostLoad(d.stream);
    if (d.rev > 0) {
        auto &ptr = PostLoadInlined();
        mDir = dynamic_cast<WorldInstance *>(ptr.Ptr());
    } else {
        mDir.PostLoad(nullptr);
    }
    if (d.rev > 1) {
        LoadPersistentObjects(d);
    }
    SyncDir();
}

void WorldInstance::SetProxyFile(const FilePath &fp, bool override) {
    MILO_ASSERT(!override, 0x246);
    DeleteObjects();
    mDir.LoadFile(fp, false, true, kLoadFront, false);
    SyncDir();
    if (mDir) {
        Hmx::Object::Copy(mDir, kCopyShallow);
    }
}

float WorldInstance::GetDistanceToPlane(const Plane &pl, Vector3 &v) {
    float dist = RndDir::GetDistanceToPlane(pl, v);
    if (mSharedGroup) {
        Vector3 v28;
        float grpdist = mSharedGroup->DistanceToPlane(WorldXfm(), pl, v28);
        if (dist > grpdist) {
            v = v28;
            dist = grpdist;
        }
    }
    return dist;
}

bool WorldInstance::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        RndDir::MakeWorldSphere(s, true);
        if (mSharedGroup) {
            Sphere s28;
            mSharedGroup->MakeWorldSphere(WorldXfm(), s28);
            s.GrowToContain(s28);
        }
        return true;
    } else {
        if (GetSphere().radius) {
            Multiply(GetSphere(), WorldXfm(), s);
            return true;
        } else
            return false;
    }
}

void WorldInstance::DrawShowing() {
    RndDir::DrawShowing();
    if (mSharedGroup) {
        mSharedGroup->Draw(WorldXfm());
    }
}

RndDrawable *WorldInstance::CollideShowing(const Segment &s, float &f, Plane &pl) {
    if (RndDir::CollideShowing(s, f, pl))
        return this;
    else {
        if (mSharedGroup) {
            if (mSharedGroup->Collide(WorldXfm(), s, f, pl)) {
                return this;
            }
        }
        return 0;
    }
}

void WorldInstance::Poll() {
    if (mSharedGroup)
        mSharedGroup->TryPoll(this);
    RndDir::Poll();
}

void WorldInstance::Enter() {
    if (mSharedGroup)
        mSharedGroup->TryEnter(this);
    RndDir::Enter();
}

void WorldInstance::SavePersistentObjects(BinStream &bs) {
    if (IsProxy()) {
        int hashSize = HashTableUsedSize();
        int strSize = StrTableUsedSize();
        DeleteTransientObjects();
        for (ObjDirItr<Hmx::Object> i(this, false); i != nullptr; ++i) {
            if (i != this) {
                MILO_ASSERT(dynamic_cast<ObjectDir*>(i.Ptr()) == NULL, 0x12F);
                i->PreSave(bs);
            }
        }
        bs << hashSize;
        bs << strSize;
        std::list<Hmx::Object *> objects;
        for (ObjDirItr<Hmx::Object> it(this, false); it != nullptr; ++it) {
            if (it != this) {
                objects.push_back(it);
            }
        }
        objects.sort(DirLoader::ClassAndNameSort());
        bs << objects.size();
        FOREACH_CONST_POST (it, objects) {
            bs << (*it)->ClassName() << (*it)->Name();
        }
        FOREACH (it, objects) {
            (*it)->Save(bs);
        }
        if (!bs.Cached()) {
            FOREACH (it, objects) {
                (*it)->PostSave(bs);
            }
        }
    }
}

void WorldInstance::LoadPersistentObjects(BinStreamRev &d) {
    if (IsProxy()) {
        if (d.rev > 2) {
            // allocate more hashtable and stringtable space
            int hashSize, stringSize;
            d >> hashSize;
            d >> stringSize;
            hashSize *= 2;
            Reserve(hashSize, stringSize);
        }
        // create the persistent objects using their ClassName and Name
        // then push them into our persistent object list
        std::list<Hmx::Object *> objlist;
        int count;
        d >> count;
        while (count-- != 0) {
            Symbol objClassName;
            d >> objClassName;
            char objName[0x80];
            d.stream.ReadString(objName, 0x80);
            if (!Hmx::Object::RegisteredFactory(objClassName)) {
                MILO_NOTIFY("%s: Can't make %s", StoredFile().c_str(), objClassName);
                DeleteObjects();
                return;
            }

            Hmx::Object *obj = Hmx::Object::NewObject(objClassName);
            obj->SetName(objName, this);
            objlist.push_back(obj);
        }

        String dirNameStr;
        ObjectDir *dirDir = nullptr;
        DataArray *dirTypeDef = nullptr;
        ObjDirPtr<ObjectDir> subDir;
        if (mDir) {
            dirNameStr = mDir->Name();
            dirDir = mDir->Dir();
            dirTypeDef = (DataArray *)mDir->TypeDef();
            subDir = mDir;
            AppendSubDir(subDir);
        }
        while (!objlist.empty()) {
            Hmx::Object *cur = objlist.front();
            cur->PreLoad(d.stream);
            cur->PostLoad(d.stream);
            objlist.pop_front();
        }
        if (mDir) {
            RemoveSubDir(subDir);
            mDir->SetName(dirNameStr.c_str(), dirDir);
            mDir->SetTypeDef(dirTypeDef);
        }
    }
}

void WorldInstance::DeleteTransientObjects() {
    if (Dir() && Dir() != DirLoader::TopSaveDir()
        && Dir()->InlineSubDirType() == kInlineAlways) {
        for (ObjDirItr<Hmx::Object> obj(this, false); obj != nullptr; ++obj) {
            if (obj != this && !dynamic_cast<RndMesh *>(obj.Ptr())) {
                Hmx::Object *to = mDir->Find<Hmx::Object>(obj->Name());
                MILO_ASSERT(obj->ClassName() == to->ClassName(), 0x1C7);
                ObjRef refs;
                refs.DetachSelf();
                FOREACH_OBJREF (it, obj) {
                    if (it->RefOwner() && it->RefOwner()->Dir() == this) {
                        it = it->MoveBefore(&refs);
                    }
                }
                refs.ReplaceList(to);
                delete obj;
            }
        }
    } else {
        DeleteObjects();
    }
}

void WorldInstance::SyncDir() {
    if (IsProxy()) {
        DeleteTransientObjects();
        mSharedGroup = nullptr;
        if (mDir) {
            RndGroup *grp = mDir->Find<RndGroup>("shared.grp", false);
            if (!mDir->mSharedGroup2 && grp) {
                mDir->mSharedGroup2 = new SharedGroup(grp);
            }
            mSharedGroup = mDir->mSharedGroup2;
            Sphere sphere = mDir->GetSphere();
            Vector3 v98;
            MakeScale(WorldXfm().m, v98);
            float f21 = Max(v98.y, v98.z);
            f21 = Max(v98.x, f21);
            if (f21 > 1.0f)
                sphere.radius *= f21;
            SetSphere(sphere);
            static Symbol Group("Group");
            static Symbol Tex("Tex");
            static Symbol CubeTex("CubeTex");
            static Symbol Movie("Movie");
            static Symbol SynthSample("SynthSample");
            std::list<ObjPair> objPairs;
            objPairs.push_back(ObjPair(mDir, this));

            for (ObjDirItr<Hmx::Object> it(mDir, false); it != nullptr; ++it) {
                bool curMesh = dynamic_cast<RndMesh *>(&*it); // mismatch here
                if (!grp || (it != grp && !GroupedUnder(grp, it))) {
                lmao:
                    if (it->ClassName() != Tex && it->ClassName() != CubeTex
                        && it->ClassName() != SynthSample && it->ClassName() != Movie
                        && it != mDir) {
                        EventTrigger *trig = dynamic_cast<EventTrigger *>(&*it);
                        if (trig && trig->HasTriggerEvents()) {
                            MILO_NOTIFY("%s must be in shared.grp", PathName(it));
                        } else {
                            Hmx::Object *foundObj = FindObject(it->Name(), false, true);
                            if (!foundObj) {
                                foundObj = Hmx::Object::NewObject(it->ClassName());
                                Hmx::Object::CopyType ty = kCopyShallow;
                                if (it->ClassName() == Group || curMesh)
                                    ty = kCopyDeep;
                                CopyObject(it, foundObj, ty, true);
                            }
                            objPairs.push_back(ObjPair(it, foundObj));
                        }
                    }

                } else if (curMesh) {
                    grp->RemoveObject(it);
                    goto lmao;
                }
            }
            FOREACH (p, objPairs) {
                if (!p->from->Dir()) {
                    MILO_FAIL(
                        "%s %s->Dir() is null, to is %s",
                        PathName(this),
                        p->from->Name(),
                        p->to->Name()
                    );
                }
                ObjRef refs;
                refs.DetachSelf();
                Hmx::Object *pFrom = p->from;
                FOREACH_OBJREF (it, pFrom) {
                    if (it->RefOwner() && !it->RefOwner()->Dir()) {
                        it = it->MoveBefore(&refs);
                    }
                }
                refs.ReplaceList(p->to);
            }

            Reserve(mDir->HashTableSize(), mDir->StrTableSize());

            FOREACH (p, objPairs) {
                if (p->to != this) {
                    p->to->SetName(p->from->Name(), this);
                }
            }

            if (f21 > 1.0f) {
                for (ObjDirItr<RndTransformable> it(this, true); it != nullptr; ++it) {
                    if (GenerationCount(this, it) > 0) {
                        RndDrawable *draw = dynamic_cast<RndDrawable *>(&*it);
                        if (draw) {
                            Sphere s = draw->GetSphere();
                            s.radius *= f21;
                            draw->SetSphere(s);
                        }
                    }
                }
            }
        }
        SyncObjects();
    }
}

#pragma endregion WorldInstance
#pragma region SharedGroup

SharedGroup::SharedGroup(RndGroup *group) : mGroup(group), mPollMaster(this) {
    AddPolls(group);
}

void SharedGroup::ClearPollMaster() { mPollMaster = nullptr; }

void SharedGroup::TryPoll(WorldInstance *inst) {
    if (!mPollMaster)
        mPollMaster = inst;
    else if (mPollMaster != inst)
        return;
    FOREACH (it, mPolls) {
        (*it)->Poll();
    }
}

void SharedGroup::TryEnter(WorldInstance *inst) {
    if (!mPollMaster)
        mPollMaster = inst;
    else if (mPollMaster != inst)
        return;
    FOREACH (it, mPolls) {
        (*it)->Enter();
    }
    ObjectDir *pollDir = mPollMaster->Dir();
    if (pollDir) {
        Hmx::Object *groupDir = mGroup->Dir();
        if (groupDir) {
            groupDir->ChainSource(pollDir, nullptr);
        }
    }
}

float SharedGroup::DistanceToPlane(const Transform &tf, const Plane &pl, Vector3 &v) {
    mGroup->SetWorldXfm(tf);
    return mGroup->GetDistanceToPlane(pl, v);
}

void SharedGroup::MakeWorldSphere(const Transform &tf, Sphere &s) {
    mGroup->SetWorldXfm(tf);
    mGroup->MakeWorldSphere(s, true);
}

bool SharedGroup::Collide(const Transform &tf, const Segment &s, float &f, Plane &pl) {
    mGroup->SetWorldXfm(tf);
    return mGroup->Collide(s, f, pl);
}

void SharedGroup::Draw(const Transform &tf) {
    mGroup->SetWorldXfm(tf);
    mGroup->Draw();
}

void SharedGroup::AddPolls(RndGroup *group) {
    FOREACH (it, group->Objects()) {
        RndPollable *poll = dynamic_cast<RndPollable *>(*it);
        if (poll) {
            mPolls.push_back(poll);
        } else {
            RndGroup *curGroup = dynamic_cast<RndGroup *>(*it);
            if (curGroup) {
                AddPolls(curGroup);
            }
        }
    }
}

#pragma endregion SharedGroup
