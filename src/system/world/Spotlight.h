#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"

class Spotlight : public RndDrawable, public RndTransformable, public RndPollable {
public:
    // Hmx::Object
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Spotlight);
    OBJ_SET_TYPE(Spotlight);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void InitObject();
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void UpdateBounds();
    // RndPollable
    virtual void Poll();

protected:
    Spotlight();
};
