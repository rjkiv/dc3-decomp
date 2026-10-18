#pragma once
#include "rndobj/Draw.h"

class RndMotionBlur : public RndDrawable {
public:
    OBJ_CLASSNAME(MotionBlur);
    OBJ_SET_TYPE(MotionBlur);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x15);
    NEW_OBJ(RndMotionBlur)
    REGISTER_OBJ(RndMotionBlur)

protected:
    RndMotionBlur();

    bool CanMotionBlur(RndDrawable *draw);
    DataNode OnAllowedDrawable(const DataArray *);

    ObjPtrList<RndDrawable> mDrawList; // 0x40
};
