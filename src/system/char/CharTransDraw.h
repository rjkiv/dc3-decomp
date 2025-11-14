#pragma once

#include "char/Character.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "utl/BinStream.h"
class CharTransDraw : public RndDrawable {
public:
    // Hmx::Object
    virtual ~CharTransDraw();
    OBJ_CLASSNAME(CharTransDraw)
    OBJ_SET_TYPE(CharTransDraw)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndDrawable
    virtual void DrawShowing();

    ObjPtrList<Character> mChars; // 0x40
    bool unk54;

protected:
    CharTransDraw();
};
