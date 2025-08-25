#pragma once
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"

class RndTexRenderer : public RndDrawable, public RndAnimatable, public RndPollable {
public:
    // Hmx::Object
    virtual ~RndTexRenderer() {}
    OBJ_CLASSNAME(TexRenderer);
    OBJ_SET_TYPE(TexRenderer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual void DrawPreClear();
    virtual void UpdatePreClearState();
    // RndAnimatable
    virtual void SetFrame(float frame, float blend);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // RndPollable
    virtual void ListPollChildren(std::list<RndPollable *> &) const;

protected:
    RndTexRenderer();
};
