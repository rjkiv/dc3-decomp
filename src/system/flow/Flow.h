#pragma once
#include "flow/FlowLabel.h"
#include "flow/FlowLabelProvider.h"
#include "flow/FlowOutPort.h"
#include "flow/FlowQueueable.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/Str.h"

/** "Top level flow" */
class Flow : public FlowQueueable,
             public ObjectDir,
             public FlowLabelProvider,
             public RndPollable {
public:
    struct DynamicPropertyEntry {
        enum PropType {
            kInt,
            kFloat,
            kBool,
            kString,
            kColor,
            kObject,
            kSymbolList
        };
        DynamicPropertyEntry(Hmx::Object *);

        void SetName(DataNode &);
        void ResetDefaultValues();
        Symbol GetDefaultValueSymbol();
        void SetClassFilter(DataNode &);
        DataNode GetSymbolList();

        /** "Name for the property" */
        String mName; // 0x0
        /** "type of the property" */
        PropType mType; // 0x8
        DataNode mDefaultVal; // 0xc
        /** "Help string for the user" */
        String mHelp; // 0x14
        /** "Is this property exposed to the proxy using this flow?" */
        bool mExposed; // 0x1c
        Symbol mObjectClass; // 0x20
        DataNode unk24; // 0x24
        Symbol mObjectType; // 0x2c
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

    int GetNumParams() const { return unk178; }
    void RefreshPortLabelLists();
    FlowLabel *GetLabelForSym(Symbol);

protected:
    Flow();

    void ToggleRunning(int);
    void OnReflectedPropertyChanged(DataArray *);
    void OnInternalPropertyChanged(DataArray *);

    ObjVector<DynamicPropertyEntry> mDynamicProperties; // 0x10c
    ObjPtrVec<FlowLabel> mFlowLabels; // 0x11c
    ObjPtrVec<FlowOutPort> mFlowOutPorts; // 0x138
    ObjPtrVec<Hmx::Object> mObjects; // 0x154
    int unk170; // 0x170
    /** "Are we hidden from run nodes?" */
    bool mPrivate; // 0x174
    /** "force things to stop immediately?" */
    bool mHardStop; // 0x175
    int unk178; // 0x178
};

// FLOW_PROPANIM_COMMANDS_ENUM
// #define kFlowStart (0)
// #define kFlowStopImmediate (1)
// #define kFlowStopWhenAble (2)
