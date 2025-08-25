#pragma once
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"

class RndRibbon : public RndPollable, public RndDrawable {
public:
    virtual void Poll();
    // Hmx::Object
    virtual ~RndRibbon() {}
    OBJ_CLASSNAME(Ribbon);
    OBJ_SET_TYPE(Ribbon);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

protected:
    RndRibbon();
};
