#pragma once
#include "obj/Object.h"
// #include "flow/DrivenPropertyEntry.h"
#include "utl/MemMgr.h"

class Flow;
class DrivenPropertyEntry;

/** "A flow node" */
class FlowNode : public virtual Hmx::Object {
public:
    enum QueueState {
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
    // Hmx::Object
    virtual ~FlowNode();
    OBJ_CLASSNAME(FlowNode)
    OBJ_SET_TYPE(FlowNode)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual const char *FindPathName();
    // FlowNode
    virtual void SetParent(FlowNode *, bool);
    virtual FlowNode *GetParent() { return mParent; }
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState) {}
    virtual bool IsRunning();
    virtual Flow *GetOwnerFlow();
    virtual void MiloPreRun();
    virtual void MoveIntoDir(ObjectDir *, ObjectDir *);
    virtual void UpdateIntensity();

    OBJ_MEM_OVERLOAD(0x9F)
    NEW_OBJ(FlowNode)

    static FlowNode *DuplicateChild(FlowNode *);

    bool HasRunningNode(FlowNode *);
    DrivenPropertyEntry *GetDrivenEntry(Symbol);
    DrivenPropertyEntry *GetDrivenEntry(DataArray *);

protected:
    FlowNode();

    static bool sPushDrivenProperties;
    static float sIntensity;

    void ActivateChild(FlowNode *);
    void PushDrivenProperties(void);

    bool mDebugOutput; // 0x8
    String mDebugComment; // 0xc
    ObjPtrVec<FlowNode> mVec1; // 0x14
    ObjPtrList<FlowNode> mChildren; // 0x30
    FlowNode *mParent; // 0x44
    ObjVector<DrivenPropertyEntry> unk48; // 0x48
    bool unk58; // 0x58
};

#define FLOW_LOG(...)                                                                    \
    if (mDebugOutput) {                                                                  \
        MILO_LOG("%s: %s", ClassName(), MakeString(__VA_ARGS__));                        \
        if (!mDebugComment.empty()) {                                                    \
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());                      \
        }                                                                                \
    }
