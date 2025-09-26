#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct ColorSet {
    Hmx::Color mPrimary;
    Hmx::Color mSecondary;
};

/**
 * @brief Contains a set of colors.
 * Original _objects description:
 * "List of primary/secondary colors for OutfitConfig"
 */
class ColorPalette : public Hmx::Object {
public:
    OBJ_CLASSNAME(ColorPalette);
    OBJ_SET_TYPE(ColorPalette);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(ColorPalette)

protected:
    ColorPalette();

    /** "Color for materials" */
    std::vector<Hmx::Color> mColors; // 0x2c
};
