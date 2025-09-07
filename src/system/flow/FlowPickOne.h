#pragma once
#include "flow/FlowNode.h"
#include "obj/Object.h"

/** "Picks certain children and activates them" */
class FlowPickOne : public FlowNode {
public:
    enum ChoiceType {
        /** "Picked in order" */
        kChoiceOrdered = 0,
        /** "Pick randomly (can repeat)" */
        kChoiceRandom = 1,
        /** "Pick randomly, but don't repeat" */
        kChoiceRandomNoRepeat = 2,
        /** "Only repeat when everything from the list has been chosen" */
        kChoiceRandomJukeBox = 3,
        /** "Use the index property to decide which one to pick" */
        kChoiceUseIndex = 4
    };
    // Hmx::Object
    virtual ~FlowPickOne();
    OBJ_CLASSNAME(FlowPickOne)
    OBJ_SET_TYPE(FlowPickOne)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();

    OBJ_MEM_OVERLOAD(0x1F)
    NEW_OBJ(FlowPickOne)

protected:
    FlowPickOne();

    void OnChoiceTypeChanged();

    ObjPtrVec<FlowNode> unk5c; // 0x5c
    /** "Style of choice made" */
    ChoiceType mChoiceType; // 0x78
    /** "The child to pick (0 is first child)" */
    int mIndex; // 0x7c
    /** "0 to 1 value representing the chance this node runs when activated" */
    float mChance; // 0x80
};
