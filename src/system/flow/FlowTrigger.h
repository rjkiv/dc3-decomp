#pragma once
#include "flow/FlowPtr.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"
#include "obj/Object.h"

class FlowTrigger : public FlowQueueable {
public:
    struct PropTriggerDefn {
        PropTriggerDefn(Hmx::Object *);

        DataNode GetPathDisplay(DataArray *);

        /** "The object providing the properties" */
        FlowPtr<Hmx::Object> mProvider; // 0x0
        DataNode unk20; // 0x20
    };
    // Hmx::Object
    virtual ~FlowTrigger();
    OBJ_CLASSNAME(FlowTrigger)
    OBJ_SET_TYPE(FlowTrigger)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowTrigger
    virtual bool Activate() { return FlowQueueable::Activate(nullptr); }
    virtual bool ActivateWithParams(Hmx::Object *, DataArray *);

    OBJ_MEM_OVERLOAD(0x1D)
    NEW_OBJ(FlowTrigger)

protected:
    FlowTrigger();

    void RegisterEvents();
    void UnregisterEvents();

    /** "The Object which I listen to for events" */
    FlowPtr<Hmx::Object> mEventProvider; // 0x68
    /** "Events which run this flow" */
    std::list<Symbol> mTriggerEvents; // 0x8c
    /** "Events which stop this flow" */
    std::list<Symbol> mStopEvents; // 0x90
    ObjList<PropTriggerDefn> mTriggerProperties; // 0x98
    ObjList<PropTriggerDefn> mStopProperties; // 0xa4
    /** "force things to stop immediately?" */
    bool mHardStop; // 0xb0
    bool unkb1; // 0xb1
};

inline BinStream &operator<<(BinStream &bs, const FlowTrigger::PropTriggerDefn &defn) {
    bs << defn.mProvider << defn.unk20;
    return bs;
}
