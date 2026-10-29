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
    enum BreakOnFunction {
        kBreakNone = 0x0000,
        kBreakActivate = 0x0001,
        kBreakDeactivate = 0x0002,
        kBreakRequestStop = 0x0003,
        kBreakRequestStopCancel = 0x0004,
        kBreakChildFinished = 0x0005,
        kBreakReactivate = 0x0006,
        kBreakExecute = 0x0007,
        kBreakAll = 0x0008,
    };
    enum ChildType {
        kNoChildren = 0x0000,
        kFlowNode = 0x0001,
        kFlowSwitchCase = 0x0002,
        kFlowValueCase = 0x0003,
        kFlowLabelCase = 0x0004,
    };
    enum QueueState {
        kRequestStop = 0x0000,
        kStarting = 0x0001,
        kAfter = 0x0002,
        kQueueIgnore = 0x0003,
        kPolling = 0x0004,
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
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual const char *FindPathName();
    // FlowNode
    virtual void SetParent(FlowNode *node, bool b);
    virtual FlowNode *GetParent() { return mFlowParent; }
    virtual bool Activate();
    virtual void Deactivate(bool isDeleting);
    virtual void ChildFinished(FlowNode *node);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState state) {}
    virtual bool IsRunning() { return !mRunningNodes.empty(); }
    virtual Flow *GetOwnerFlow();
    virtual void MiloPreRun();
    virtual void MoveIntoDir(ObjectDir *d, ObjectDir *src);
    virtual void UpdateIntensity();

    OBJ_MEM_OVERLOAD(0x9F)
    NEW_OBJ(FlowNode)
    REGISTER_OBJ(FlowNode)

    static FlowNode *DuplicateChild(FlowNode *node);
    static Hmx::Object *LoadObjectFromMainOrDir(BinStream &d, ObjectDir *dir);

    bool HasRunningNode(FlowNode *node);
    DrivenPropertyEntry *GetDrivenEntry(Symbol);
    DrivenPropertyEntry *GetDrivenEntry(DataArray *);
    Flow *GetTopFlow();
    ObjVector<DrivenPropertyEntry> &GetDrivenProperties() { return mDrivenProperties; }
    const ObjPtrVec<FlowNode> &GetChildNodes() const { return mChildNodes; }
    void AddChild(FlowNode *n) { mChildNodes.push_back(n); }
    void RemoveChild(FlowNode *n) { mChildNodes.remove(n); }

protected:
    FlowNode();

    static bool sPushDrivenProperties;
    static float sIntensity;

    void ActivateChild(FlowNode *node);
    void PushDrivenProperties();
    void ActivateLabel(FlowLabel *node);

    bool mDebugOutput; // 0x8
    String mDebugComment; // 0xc
    ObjPtrVec<FlowNode> mChildNodes; // 0x14
    ObjPtrList<FlowNode> mRunningNodes; // 0x30
    FlowNode *mFlowParent; // 0x44
    ObjVector<DrivenPropertyEntry> mDrivenProperties; // 0x48
    bool mRequestingStop; // 0x58
};

#define FLOW_LOG(...)                                                                    \
    if (mDebugOutput) {                                                                  \
        MILO_LOG("%s: %s", ClassName(), MakeString(__VA_ARGS__));                        \
        if (!mDebugComment.empty()) {                                                    \
            MILO_LOG("Debug comment: %s\n", mDebugComment.c_str());                      \
        }                                                                                \
    }

#define FLOW_TIMED_RELEASE_FROM_PARENT                                                   \
    {                                                                                    \
        FLOW_LOG("Timed Release From Parent \n");                                        \
        Timer timer;                                                                     \
        timer.Reset();                                                                   \
        timer.Start();                                                                   \
        mFlowParent->ChildFinished(this);                                                \
        timer.Stop();                                                                    \
        TheFlowMgr->AddReleaseTime(timer.Ms());                                          \
    }
