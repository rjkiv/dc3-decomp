#pragma once
#include "obj/Data.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "math/Sphere.h"
#include "utl/MemMgr.h"

class RndCam;

enum HighlightStyle {
    kHighlightWireframe,
    kHighlightSphere,
    kHighlightNone,
    kHighlightWireframeWithNormals,
    kNumHighlightStyles
};

/**
 * @brief An object that is drawable.
 * Original _objects description:
 * "Base class for drawable objects. Draw objects either
 * render polys or determine rendering state."
 */
class RndDrawable : public virtual RndHighlightable {
public:
    struct Collision {
        Collision() {}
        Collision(RndDrawable *o, float d, const Plane &p)
            : object(o), distance(d), plane(p) {}
        RndDrawable *object; // offset 0x0, size 0x4
        float distance; // offset 0x4, size 0x4
        Plane plane; // offset 0x10, size 0x10
    };

    OBJ_CLASSNAME(Draw);
    OBJ_SET_TYPE(Draw);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    /** "Calculates the bounding sphere for the object." */
    virtual void UpdateSphere() { mSphere.Zero(); }
    virtual float GetDistanceToPlane(const Plane &, Vector3 &) { return 0.0f; }
    virtual bool MakeWorldSphere(Sphere &, bool) { return 0; }
    /** Get the current camera to use. */
    virtual RndCam *CamOverride() { return 0; }
    virtual void Mats(std::list<class RndMat *> &, bool) {}
    virtual void Draw();
    virtual void DrawShowing() {}
    virtual void DrawShadow(const Transform &, float) {}
    /** Get the list of this Object's children that are drawable. */
    virtual void ListDrawChildren(std::list<RndDrawable *> &) {}
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &) { return 0; }
    virtual int CollidePlane(const Plane &);
    virtual void CollideList(const Segment &, std::list<Collision> &);
    virtual void DrawPreClear() {}
    virtual void UpdatePreClearState() {}
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x25);

    void SetShowing(bool);
    bool Showing() const { return mShowing; }
    RndDrawable *Collide(const Segment &, float &, Plane &);

    static void DumpLoad(BinStream &bs);
    static HighlightStyle GetHighlightStyle() { return sHighlightStyle; }
    static void SetHighlightStyle(HighlightStyle hs) { sHighlightStyle = hs; }
    static float GetNormalDisplayLength() { return sNormalDisplayLength; }
    static void SetNormalDisplayLength(float f) { sNormalDisplayLength = f; }
    static bool GetForceSubpartSelection() { return sForceSubpartSelection; }
    static void SetForceSubpartSelection(bool b) { sForceSubpartSelection = b; }

protected:
    RndDrawable();

    static HighlightStyle sHighlightStyle;
    static float sNormalDisplayLength;
    static bool sForceSubpartSelection;

    /** Handler to copy another RndDrawable's sphere to this one's.
     * @param [in] arr The supplied DataArray.
     * Expected DataArray contents:
     *     Node 2: the other RndDrawable.
     * Example usage: {$this copy_sphere other_obj}
     */
    DataNode OnCopySphere(const DataArray *arr);
    /** Handler to retrieve this RndDrawable's sphere properties.
     * @param [in] arr The supplied DataArray.
     * Expected DataArray contents:
     *     Nodes 2-5: vars to house this sphere's center X/Y/Z coordinates and radius.
     * Example usage: {$this get_sphere $x $y $z $radius}
     */
    DataNode OnGetSphere(const DataArray *arr);
    /** Handler to set whether or not this RndDrawable is showing.
     * @param [in] arr The supplied DataArray.
     * Expected DataArray contents:
     *     Node 2: a boolean for showing or hiding.
     * Example usage: {$this set_showing TRUE}
     */
    DataNode OnSetShowing(const DataArray *arr);
    /** Handler to get whether or not this RndDrawable is showing.
     * @returns True if showing, false if not.
     * Example usage: {$this showing}
     */
    DataNode OnShowing(const DataArray *);
    /** Handler to zero this RndDrawable's sphere.
     * Example usage: {$this zero_sphere}
     */
    DataNode OnZeroSphere(const DataArray *);
    DataNode OnGetDrawChildren(const DataArray *);

    /** "Whether the object and its Draw children are drawn or collided with." */
    bool mShowing; // 0x8
    /** "bounding sphere" */
    Sphere mSphere; // 0xc
    /** "Draw order within proxies, lower numbers are drawn first,
        so assign numbers from the outside-in (unless translucent), to minimize overdraw.
        In groups, draw_order will be ignored unless you explicitly
        click the sort button."*/
    float mOrder; // 0x20
    /** "List of up to 6 transformable objects which specify user-defined clip planes
        (for a given trans, the plane will run through the trans position,
        and use the rotated z-axis for the plane normal)." */
    ObjPtrVec<RndTransformable> mClipPlanes; // 0x24
};

class DrawPtrVec : public ObjPtrVec<RndDrawable> {
public:
    DrawPtrVec(Hmx::Object *owner) : ObjPtrVec<RndDrawable>(owner) {}

    void Draw() const;
    RndDrawable *CollideShowing(const Segment &, float &, Plane &) const;
    void CollideList(const Segment &, std::list<RndDrawable::Collision> &) const;
};
