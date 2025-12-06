#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

/**
 * @brief A color to used by UI objects.
 * Original _objects description:
 * "Just a color, used by UI components"
 */
class UIColor : public Hmx::Object {
public:
    OBJ_CLASSNAME(UIColor);
    OBJ_SET_TYPE(UIColor);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    NEW_OBJ(UIColor)
    OBJ_MEM_OVERLOAD(0x10)

    const Hmx::Color &GetColor() const;
    void SetColor(const Hmx::Color &);

protected:
    UIColor();

    /** The color. */
    Hmx::Color mColor; // 0x2c
};
