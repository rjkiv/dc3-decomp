#pragma once
#include "math/SHA1.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Feeds the bones when executed." */
class CharCollide : public RndTransformable {
public:
    enum Shape {
        kCollidePlane = 0,
        kCollideSphere = 1,
        kCollideInsideSphere = 2,
        kCollideCigar = 3,
        kCollideInsideCigar = 4
    };

    struct CharCollideStruct {
        int unk0;
        Vector3 unk4;
    };

    // Hmx::Object
    virtual ~CharCollide();
    OBJ_CLASSNAME(CharCollide)
    OBJ_SET_TYPE(CharCollide)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(CharCollide)

    void SyncShape();
    void CopyOriginalToCur();

protected:
    CharCollide();

    /** "Type of collision" */
    Shape mShape; // 0xc0
    int mFlags; // 0xc4
    /** "Optional mesh that will deform, used to resize ourselves.
        If this is set, make sure you are not parented to any bone with scale,
        such as an exo bone" */
    ObjPtr<RndMesh> mMesh; // 0xc8
    CSHA1::Digest mDigest; // 0xdc
    CharCollideStruct unkStructs[8]; // 0xe0
    /** radius0: "Radius of the sphere, or of length0 hemisphere if cigar" */
    /** radius1: "cigar: Radius of length1 hemisphere" */
    float mOrigRadius[2]; // 0x190
    /** length0: "cigar: placement of radius0 hemisphere along X axis,
        must be < than length0, not used for sphere shapes" */
    /** length1: "cigar: placement of radius1 hemisphere along X axis,
        must be >= length0" */
    float mOrigLength[2]; // 0x198
    Transform unk1a0; // 0x1a0
    float mCurRadius[2]; // 0x1e0
    float mCurLength[2]; // 0x1e8
    /** "For spheres + cigars, finds mesh points along positive y axis (the green one),
        makes a better fit for spheres where only one side should be the fit,
        like for chest and back collision volumes" */
    bool mMeshYBias; // 0x1f0
    float unk1f4; // 0x1f4
    float unk1f8; // 0x1f8
    Vector3 unk1fc; // 0x1fc
    Vector3 unk20c; // 0x20c
};
