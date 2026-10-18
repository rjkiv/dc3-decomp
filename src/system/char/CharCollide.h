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
        kPlane = 0x0000,
        kSphere = 0x0001,
        kInsideSphere = 0x0002,
        kCigar = 0x0003,
        kInsideCigar = 0x0004
    };

    struct VertData {
        int vertIndex; // 0x0
        Vector3 offset; // 0x4
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

    const Vector3 &Axis() const { return mAxis; }
    Shape GetShape() const { return mShape; }
    float Radius() const { return mCurRadii[0]; }

    float GetRadius(const Vector3 &v1, Vector3 &vout) const {
        Subtract(v1, mPos, vout);
        float ret = mCurRadii[0];
        if (mShape >= kCigar) {
            float clamped =
                Clamp(mCurLengths[0], mCurLengths[1], mInvAxis2 * Dot(vout, mAxis));
            ScaleAddEq(vout, mAxis, -clamped);
            Interp(ret, mCurRadii[1], mInvLength * (clamped - mCurLengths[0]), ret);
        } else if (mShape == kPlane) {
            ret = Dot(vout, mAxis);
            Scale(mAxis, ret, vout);
        }
        return ret;
    }

    void SyncWorldState() {
        mPos = WorldXfm().v;
        if (mShape >= kCigar || mShape == kPlane) {
            mAxis = WorldXfm().m.x;
            mInvAxis2 = 1.0f / LengthSquared(mAxis);
        }
        if (mShape >= kCigar) {
            mInvLength = 1.0f / (mCurLengths[1] - mCurLengths[0]);
        }
    }

protected:
    CharCollide();
    int NumSpheres(Shape) const;

    /** "Type of collision" */
    Shape mShape; // 0xc0
    int mFlags; // 0xc4
    /** "Optional mesh that will deform, used to resize ourselves.
        If this is set, make sure you are not parented to any bone with scale,
        such as an exo bone" */
    ObjPtr<RndMesh> mMesh; // 0xc8
    CSHA1::Digest mMeshHash; // 0xdc
    VertData mVertData[8]; // 0xe0
    /** radius0: "Radius of the sphere, or of length0 hemisphere if cigar" */
    /** radius1: "cigar: Radius of length1 hemisphere" */
    float mOriginalRadii[2]; // 0x190
    /** length0: "cigar: placement of radius0 hemisphere along X axis,
        must be < than length0, not used for sphere shapes" */
    /** length1: "cigar: placement of radius1 hemisphere along X axis,
        must be >= length0" */
    float mOriginalLengths[2]; // 0x198
    Transform mInvParentXfm; // 0x1a0
    float mCurRadii[2]; // 0x1e0
    float mCurLengths[2]; // 0x1e8
    /** "For spheres + cigars, finds mesh points along positive y axis (the green one),
        makes a better fit for spheres where only one side should be the fit,
        like for chest and back collision volumes" */
    bool mMeshYBias; // 0x1f0
    float mInvLength; // 0x1f4
    float mInvAxis2; // 0x1f8
    Vector3 mAxis; // 0x1fc
    Vector3 mPos; // 0x20c
};
