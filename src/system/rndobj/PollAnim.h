#pragma once
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "obj/Object.h"

/**
 * @brief: An object that polls animatables at a set time.
 * Original _objects description:
 * "Class that drives Anims with time based on their rate."
 */
class RndPollAnim : public virtual RndAnimatable,
                    public virtual RndPollable,
                    public virtual Hmx::Object {
public:
    OBJ_CLASSNAME(PollAnim);
    OBJ_SET_TYPE(PollAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();

protected:
    RndPollAnim();

    /** The list of animatables to poll.
     * "List of anims that will have SetFrame called on them according to their rate and
     * the TheTaskMgr.Seconds or Beat"
     */
    ObjPtrList<RndAnimatable> mAnims; // 0x4
};
