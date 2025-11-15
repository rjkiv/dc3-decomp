#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"

class CharMeshHide : public Hmx::Object {
public:
    class Hide {
    public:
        Hide(Hmx::Object *);
        Hide(const Hide &);
        Hide &operator=(const Hide &);

        ObjPtr<RndDrawable> mDraw; // 0x0
        int mFlags; // 0x14
        bool mShow; // 0x18
    };

    virtual ~CharMeshHide();
    OBJ_CLASSNAME(CharMeshHide);
    OBJ_SET_TYPE(CharMeshHide);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    static void Init();
    NEW_OBJ(CharMeshHide)

    ObjVector<Hide> mHides; // 0x2c
    int mFlags; // 0x3c

protected:
    CharMeshHide();
};
