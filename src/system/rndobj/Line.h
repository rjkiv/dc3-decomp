#pragma once
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Line objects represent 3D lines with thickness, perspective, and optional end caps."
 */
class RndLine : public RndDrawable, public RndTransformable {
public:
    class Point {
    public:
        Point() : point(0, 0, 0), color(1, 1, 1) {}
        /** "Position of point" */
        Vector3 point; // 0x0
        /** "Color of point" */
        Hmx::Color color; // 0x10
        int unk[10];
    };

    class VertsMap {
    public:
        int t; // Type
        RndMesh::Vert *v;
    };

    // Hmx::Object
    virtual ~RndLine() { RELEASE(mMesh); }
    OBJ_CLASSNAME(Line);
    OBJ_SET_TYPE(Line);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndDrawable
    virtual void UpdateSphere();
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndLine
    virtual RndMesh *Mesh() const { return mMesh; }

    OBJ_MEM_OVERLOAD(0x1A);
    NEW_OBJ(RndLine)
    static void Init() { REGISTER_OBJ_FACTORY(RndLine) }

    int NumPoints() const { return mPoints.size(); }
    Point &PointAt(int idx) { return mPoints[idx]; }
    float GetWidth() const { return mWidth; }
    void SetWidth(float w) { mWidth = w; }
    RndMat *Mat() const { return mMat.Ptr(); }

    void SetPointPos(int, const Vector3 &);
    void SetPointColor(int, const Hmx::Color &, bool);
    void SetPointsColor(int, int, const Hmx::Color &);
    void SetUpdate(bool);
    void UpdatePointColor(int, bool);
    void SetMat(RndMat *);
    void SetNumPoints(int);

protected:
    RndLine();

    void UpdateInternal();
    void UpdateLine(const Transform &, float);
    void MapVerts(int, VertsMap &);

    DataNode OnSetMat(const DataArray *);

    /** "Width of the line". Ranges from 0 to 1000. */
    float mWidth; // 0x100
    std::vector<Point> mPoints; // 0x104
    RndMesh *mMesh; // 0x110
    /** "The line has end caps" */
    bool mHasCaps; // 0x114
    /** "Consider the [points] as an array of pairs, rather than a continuous line" */
    bool mLinePairs; // 0x115
    /** "Degrees at which the line starts to fold". Ranges from 0 to 180. */
    float mFoldAngle; // 0x118
    float mFoldCos; // 0x11c
    /** "Material the line uses." */
    ObjPtr<RndMat> mMat; // 0x120
    bool mLineUpdate; // 0x134
};
