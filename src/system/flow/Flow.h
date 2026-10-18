#pragma once
#include "flow/FlowLabel.h"
#include "flow/FlowLabelProvider.h"
#include "flow/FlowOutPort.h"
#include "flow/FlowQueueable.h"
#include "flow/FlowTrigger.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/Str.h"

/** "Top level flow" */
class Flow : public FlowQueueable,
             public ObjectDir,
             public FlowLabelProvider,
             public RndPollable {
public:
    enum StartMode {
        kStartNone = 0x0000,
        kStartOnEnter = 0x0001,
        kStartAfterGameCode = 0x0002
    };
    enum DynamicPropertyType {
        kInt,
        kFloat,
        kBool,
        kString,
        kColor,
        kObject,
        kSymbolList
    };

    struct DynamicPropertyEntry {
        DynamicPropertyEntry(Hmx::Object *);

        void SetName(DataNode &);
        void ResetDefaultValues();
        Symbol GetDefaultValueSymbol();
        void SetClassFilter(DataNode &);
        DataNode GetSymbolList();

        /** "Name for the property" */
        String mName; // 0x0
        /** "type of the property" */
        DynamicPropertyType mType; // 0x8
        DataNode mDefaultValue; // 0xc
        /** "Help string for the user" */
        String mHelp; // 0x14
        /** "Is this property exposed to the proxy using this flow?" */
        bool mExposed; // 0x1c
        Symbol mClassFilter; // 0x20
        DataNode mSymbolList; // 0x24
        Symbol mObjType; // 0x2c
    };
    // Hmx::Object
    virtual ~Flow();
    OBJ_CLASSNAME(Flow)
    OBJ_SET_TYPE(Flow)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects();
    // RndPollable
    virtual void Enter();
    virtual void Exit();
    // FlowQueueable
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual Flow *GetOwnerFlow();
    virtual bool ActivateTrigger();
    virtual bool Activate(Hmx::Object *);
    virtual bool Activate(Hmx::Object *, DataArray *);

    OBJ_MEM_OVERLOAD(0x39)
    NEW_OBJ(Flow)
    REGISTER_OBJ(Flow)

    int GetParamIdx() { return mParamIdx; }
    void RefreshPortLabelLists();
    FlowLabel *GetLabelForSym(Symbol);
    void ApplyParams(DataArray *, FlowTrigger *);

    ObjectDir *LoadingDir() {
        DirLoader *dl = Loader();
        return dl ? dl->ProxyDir() : Dir();
    }

    void StartOnEnter(bool start) {
        if (start)
            mStartMode = kStartAfterGameCode;
        else
            mStartMode = kStartNone;
    }

    void StartAfterGameCode(bool start) {
        if (mStartMode != kStartNone)
            mStartMode = start ? kStartAfterGameCode : kStartOnEnter;
    }

    const ObjVector<DynamicPropertyEntry> &DynamicPropEntries() const {
        return mDynamicProperties;
    }

protected:
    Flow();

    bool NoProxyFile() { return ProxyFile().empty(); }

    void ToggleRunning(int);
    void OnReflectedPropertyChanged(DataArray *);
    void OnInternalPropertyChanged(DataArray *);

    static bool sReflectingProperty;

    ObjVector<DynamicPropertyEntry> mDynamicProperties; // 0x10c
    ObjPtrVec<FlowLabel> mLabels; // 0x11c
    ObjPtrVec<FlowOutPort> mOutPorts; // 0x138
    ObjPtrVec<Hmx::Object> mStateListeners; // 0x154
    StartMode mStartMode; // 0x170
    /** "Are we hidden from run nodes?" */
    bool mPrivate; // 0x174
    /** "force things to stop immediately?" */
    bool mHardStop; // 0x175
    int mParamIdx; // 0x178
};

// FLOW_PROPANIM_COMMANDS_ENUM
// #define kFlowStart (0)
// #define kFlowStopImmediate (1)
// #define kFlowStopWhenAble (2)

void FlowInit();
