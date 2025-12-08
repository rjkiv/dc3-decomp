#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "A waypoint for character movement. Characters walk to
 *  these, start themselves out from these, etc." */
class Waypoint : public RndTransformable {
public:
    virtual ~Waypoint();
    OBJ_CLASSNAME(Waypoint)
    OBJ_SET_TYPE(Waypoint)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(Waypoint)
    void SetRadius(float r) { mRadius = r; }
    void SetStrictRadiusDelta(float r) { mStrictRadiusDelta = r; }
    void SetAngRadius(float r) { mAngRadius = r; }
    void SetStrictAngDelta(float d) { mStrictAngDelta = d; }
    void SetYRadius(float r) { mYRadius = r; }
    float MYRadius() { return mYRadius; }
    float Radius() { return mRadius; }

    float ShapeDelta(float);
    void ShapeDelta(Vector3 const &, Vector3 &);
    void Constrain(class Transform &);

    static void Init();
    static void Terminate();
    static Waypoint *FindNearest(const Vector3 &, int);

private:
    static std::list<Waypoint *> *sWaypoints;
    static int sSearchID;

    static Waypoint *Find(int);
    static DataNode OnWaypointFind(DataArray *);
    static DataNode OnWaypointNearest(DataArray *);
    static DataNode OnWaypointLast(DataArray *);

    float ShapeDeltaAng(float, float);
    void ShapeDeltaBox(Vector3 const &, float, float, Vector3 &);

protected:
    Waypoint();

    /** "Flags for this waypoint, should be a bitfield per app" */
    int mFlags; // 0xc0
    /** "Radius within we can stop from a walk, or be tethered to" */
    float mRadius; // 0xc4
    /** "If positive, makes this shape a box with radius the x axis X half width,
        and y_radius the Y axis half width" */
    float mYRadius; // 0xc8
    /** "Angular slop in degrees away from y axis" */
    float mAngRadius; // 0xcc
    int unkd0; // 0xd0
    /** "degrees beyond ang radius you can never rotate past, if >= 0".
        Ranges from -1 to 360. */
    float mStrictAngDelta; // 0xd4
    /** "how much beyond radius you will never leave,
        it will forcibly pull you back, ignored if <= 0" */
    float mStrictRadiusDelta; // 0xd8
    /** "Waypoints we can walk to" */
    ObjPtrVec<Waypoint> mConnections; // 0xdc
};
