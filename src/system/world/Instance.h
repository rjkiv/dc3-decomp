#pragma once
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Group.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class SharedGroup;

class WorldInstance : public RndDir {
public:
    // Hmx::Object
    virtual ~WorldInstance();
    OBJ_CLASSNAME(WorldInstance)
    OBJ_SET_TYPE(WorldInstance)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PostSave(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SetProxyFile(const FilePath &, bool);
    virtual const FilePath &ProxyFile() { return mDir.GetFile(); }
    // RndDrawable
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x3F)
    NEW_OBJ(WorldInstance)
private:
    void LoadPersistentObjects(BinStreamRev &);
    void SavePersistentObjects(BinStream &);
    void DeleteTransientObjects();
    void SyncDir();

protected:
    WorldInstance();

    /** "Which file we instance, only set in instances" */
    ObjDirPtr<WorldInstance> mDir; // 0x1fc
    /** "Pointer to shared group, if any" */
    SharedGroup *mSharedGroup; // 0x210
    SharedGroup *mSharedGroup2; // 0x214
};

class SharedGroup : public RndPollable {
public:
    SharedGroup(RndGroup *);
    virtual ~SharedGroup() {}

    void ClearPollMaster();
    RndGroup *Group() const { return mGroup; }
    WorldInstance *PollMaster() const { return mPollMaster; }

    void TryPoll(WorldInstance *);
    void TryEnter(WorldInstance *);
    float DistanceToPlane(const Transform &tf, const Plane &pl, Vector3 &v);
    void MakeWorldSphere(const Transform &tf, Sphere &s);
    bool Collide(const Transform &tf, const Segment &s, float &f, Plane &pl);
    void Draw(const Transform &tf);

private:
    void AddPolls(RndGroup *);

    RndGroup *mGroup; // 0x8
    /** "Am I the guy that polls the shared group" */
    ObjPtr<WorldInstance> mPollMaster; // 0xc
    std::list<RndPollable *> mPolls; // 0x20
};
