#pragma once
#include "rndobj/Trans.h"
#include "rndobj/Draw.h"
#include "rndobj/Anim.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include <vector>

/**
 * @brief: A group of objects that receive messages.
 * Original _objects description:
 * "Represents a group of objects to which to propogate
 * animation and messages."
 */
class RndGroup : public RndAnimatable, public RndDrawable, public RndTransformable {
public:
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Group)
    OBJ_SET_TYPE(Group)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    virtual void StartAnim();
    virtual void EndAnim();
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;

    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Draw();
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void CollideList(const Segment &, std::list<Collision> &);

    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x18);

    void Merge(const RndGroup *);
    /** Remove an object from this group.
     * @param [in] obj The object to remove.
     */
    void RemoveObject(Hmx::Object *obj);
    /** Add an object to this group.
     * @param [in] add The object to add.
     * @param [in] after The object currently in the group to place add after. If null,
     * add will be appended.
     */
    void AddObject(Hmx::Object *add, Hmx::Object *after = nullptr);
    /** Completely empty the group. */
    void ClearObjects();
    // /** "Sort objects by draw_order and material" */
    // void SortDraws();
    // /** Add an object to the front of the group.
    //  * @param [in] obj The object to add.
    //  */
    // void AddObjectAtFront(Hmx::Object *obj);

    /** Handler to get all RndDrawables in this RndGroup.
     * @returns A DataNode housing a DataArray of each RndDrawable.
     * Example usage: {$this get_draws}
     */
    DataNode OnGetDraws(DataArray *);

protected:
    RndGroup();

    /** Update RndGroup's class members based on the contents of the group. */
    void Update();

    /** The objects of this group. */
    ObjPtrList<Hmx::Object> mObjects; // 0x110
    /** "if set, only draws this member of the group" */
    ObjPtr<RndDrawable> mDrawOnly; // 0x124
    /** The animatable objects of this group. */
    std::vector<RndAnimatable *> mAnims; // 0x138
    /** The drawable objects of this group. */
    std::vector<RndDrawable *> mDraws; // 0x144
    /** "Sort by distance to current camera per frame.
     * This has a CPU cost if there are many objects."
     */
    bool mSortInWorld; // 0x150
};
