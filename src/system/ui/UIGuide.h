#pragma once
#include "obj/Object.h"
#include "utl/MemMgr.h"

/** "a guide used to line up UI elements" */
class UIGuide : public Hmx::Object {
public:
    enum Type {
        kGuideVertical,
        kGuideHorizontal
    };
    virtual ~UIGuide();
    OBJ_CLASSNAME(UIGuide);
    OBJ_SET_TYPE(UIGuide);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    NEW_OBJ(UIGuide)
    OBJ_MEM_OVERLOAD(0x11)

protected:
    UIGuide();

    /** "Horizontal or vertical guide?" */
    Type mType; // 0x2c
    /** "position in proportion to screen width (if vertical) or height (if horizontal)".
     * Ranges from 0 to 1. */
    float mPos; // 0x30
};
