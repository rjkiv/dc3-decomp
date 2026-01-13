#pragma once
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

class RndDir : public ObjectDir,
               public RndDrawable,
               public RndAnimatable,
               public RndTransformable,
               public RndPollable {
public:
    // Hmx::Object
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(RndDir);
    OBJ_SET_TYPE(RndDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void Export(DataArray *, bool);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SetSubDir(bool);
    virtual void SyncObjects();
    virtual void ChainSourceSubdir(Hmx::Object *, ObjectDir *);
    virtual void
    CollideListSubParts(const Segment &, std::list<RndDrawable::Collision> &);
    // RndDrawable
    virtual void UpdateSphere();
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void CollideList(const Segment &, std::list<Collision> &);
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // RndTransformable
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit(); // 0x1ac
    virtual void ListPollChildren(std::list<RndPollable *> &) const;

    OBJ_MEM_OVERLOAD(0x19);
    NEW_OBJ(RndDir)
    static void Init() { REGISTER_OBJ_FACTORY(RndDir) }

    void SetEnv(RndEnviron *env) { mEnv = env; }
    RndEnviron *GetEnv() const { return mEnv; }
    void SyncDrawables();

protected:
    RndDir();

    virtual void RemovingObject(Hmx::Object *);
    virtual void OldLoadProxies(BinStream &, int);

    void HarvestPollables(std::vector<RndPollable *> &);

    DataNode OnShowObjects(DataArray *);
    DataNode OnSupportedEvents(DataArray *);

    /** "List of all the draws" */
    std::vector<RndDrawable *> mDraws; // 0x1b4
    /** Animations for this dir. */
    std::vector<RndAnimatable *> mAnims; // 0x1c0
    /** "List of all the polls" */
    std::vector<RndPollable *> mPolls; // 0x1cc
    std::vector<RndPollable *> mEnters; // 0x1d8
    /** The dedicated RndEnviron for this dir. */
    ObjPtr<RndEnviron> mEnv; // 0x1e4
    /** "Test event" */
    Symbol mTestEvent; // 0x1f8
};
