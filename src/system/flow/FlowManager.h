#pragma once
#include "obj/Object.h"
#include "flow/FlowNode.h"
#include "rndobj/Overlay.h"
#include <map>

class FlowManager : public Hmx::Object {
public:
    FlowManager();
    virtual ~FlowManager();

    void AddPollable(FlowNode *);
    void RemovePollable(FlowNode *);
    void Poll();
    void AddEventTime(Symbol, float);
    void QueueCommand(FlowNode *, FlowNode::QueueState);
    void CancelCommand(FlowNode *);

protected:
    bool unk2c;
    bool unk2d;
    std::map<FlowNode *, FlowNode::QueueState> mFlowQueue; // 0x30
    ObjPtrVec<FlowNode> mPollables; // 0x48
    std::map<Symbol, DataNode> unk64; // 0x64
    float unk7c; // 0x7c
    float unk80; // 0x80
    RndOverlay *mFlowOverlay; // 0x84
    RndOverlay *mFlowPeakOverlay; // 0x88
    RndOverlay *mFlowTaskOverlay; // 0x8c
    RndOverlay *mFlowEventOverlay; // 0x90
    int unk94;
    int unk98[60];
    int unk188;
    float unk18c;
    float unk190; // 0x190
    DataNode unk194;
};

extern FlowManager *TheFlowMgr;
