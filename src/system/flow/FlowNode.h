#pragma once
#include "obj/Object.h"
// #include "flow/DrivenPropertyEntry.h"
#include "utl/MemMgr.h"

class Flow;
class DrivenPropertyEntry;
class FlowLabel;

/** "A flow node" */
class FlowNode : public virtual Hmx::Object {
public:
    enum QueueState {
        /** "If we're activated, ignore the activation" */
        kIgnore = 0,
        /** "New activations go in the queue and are executed when this one finishes" */
        kQueue = 1,
        /** "New activations go into a one deep queue and are executed when this one
         * finishes" */
        kQueueOne = 2,
        /** "Forcably stop what we're doing and restart" */
        kImmediate = 3,
        /** "Ask our children to stop, then run again when they finish" */
        kWhenAble = 4
    };
    enum StopMode {
        /** "Stop immediately." */
        kStopImmediate = 0,
        /** "Stop at the end of this action." */
        kStopLastFrame = 1,
        /** "Stop at the next stop marker (stop) or at the end of this action." */
        kStopOnMarker = 2,
        /** "Stop if between a stop and no_stop marker, or at the end of the action." */
        kStopBetweenMarkers = 3,
        /** "When asked to stop, release but leave the sound playing." */
        kReleaseAndContinue = 4
    };
    enum OperatorType {
        kEqual = 0,
        kNotEqual = 1,
        kGreaterThan = 2,
        kGreaterThanOrEqual = 3,
        kLessThan = 4,
        kLessThanOrEqual = 5,
        kTransition = 6
    };
    // Hmx::Object
    virtual ~FlowNode();
    OBJ_CLASSNAME(FlowNode)
    OBJ_SET_TYPE(FlowNode)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual const char *FindPathName();
    // FlowNode
    virtual void SetParent(FlowNode *, bool);
    virtual FlowNode *GetParent() { return mFlowParent; }
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState) {}
    virtual bool IsRunning() { return !mRunningNodes.empty(); }
    virtual Flow *GetOwnerFlow();
    virtual void MiloPreRun();
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual void UpdateIntensity();

    OBJ_MEM_OVERLOAD(0x9F)
    NEW_OBJ(FlowNode)

    static FlowNode *DuplicateChild(FlowNode *);
    static Hmx::Object *LoadObjectFromMainOrDir(BinStream &, ObjectDir *);

    bool HasRunningNode(FlowNode *);
    DrivenPropertyEntry *GetDrivenEntry(Symbol);
    DrivenPropertyEntry *GetDrivenEntry(DataArray *);
    Flow *GetTopFlow();
    const ObjVector<DrivenPropertyEntry> &DrivenPropEntries() const {
        return mDrivenPropEntries;
    }

protected:
    FlowNode();

    static bool sPushDrivenProperties;
    static float sIntensity;

    void ActivateChild(FlowNode *);
    void PushDrivenProperties(void);
    void ActivateLabel(FlowLabel *);

    bool mDebugOutput; // 0x8
    String mDebugComment; // 0xc
    ObjPtrVec<FlowNode> mChildNodes; // 0x14
    ObjPtrList<FlowNode> mRunningNodes; // 0x30
    FlowNode *mFlowParent; // 0x44
    ObjVector<DrivenPropertyEntry> mDrivenPropEntries; // 0x48
    bool unk58; // 0x58
};

#define FLOW_LOG(...)                                                                    \
    if (mDebugOutput) {                                                                  \
        MILO_LOG("%s: %s", ClassName(), MakeString(__VA_ARGS__));                        \
        if (!mDebugComment.empty()) {                                                    \
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());                      \
        }                                                                                \
    }
