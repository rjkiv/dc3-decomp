#pragma once
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "A cuff used to constrain colliding outfits against each other.
    for example boots against pants.  The widest cuff wins" */
class CharCuff : public RndTransformable {
public:
    struct Shape {
        float offset; // 0x0
        float radius; // 0x4
    };
    // Hmx::Object
    virtual ~CharCuff();
    OBJ_CLASSNAME(CharCuff)
    OBJ_SET_TYPE(CharCuff)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(CharCuff)

protected:
    CharCuff();

    // offset0, radius0, offset1, radius1, offset2, radius2, outer_radius
    // offset0: "Inner offset, usually negative, the inside of the cuff"
    // radius0: "Inner radius, usually the smallest, the inside of the cuff"
    // offset1: "middle offset, usually zero, the center of the cuff"
    // radius1: "middle radius for the center of the cuff, should be at the cuff line"
    // offset2: "Outer offset, usually positive, the outside of the cuff"
    // radius2: "Outer radius, usually the largest, the outside of the cuff"
    Shape mShape[3]; // 0xc0, 0xc8, 0xd0
    /** "Outside radius, should encompass
        the biggest thing on the outside, biggest one wins.
        For incompressible things like big boots should be the biggest part.
        For soft things like cloth should just be radius1" */
    float mOuterRadius; // 0xd8
    /** "Is the inside open or closed, open is good for things like gauntlets" */
    bool mOpenEnd; // 0xdc
    /** "meshes to never deform" */
    ObjPtrList<RndMesh> mIgnore; // 0xe0
    /** "The bone of interest, like bone_R-knee for boot and pant cuffs" */
    ObjPtr<RndTransformable> mBone; // 0xf4
    /** "How much smaller to make the radius along Y, must be < 1". Ranges from 0 to 1. */
    float mEccentricity; // 0x108
    /** "The outfit category to cuff against, must be set to work" */
    Symbol mCategory; // 0x10c
};
