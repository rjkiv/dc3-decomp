#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "rndobj/Overlay.h"

FlowManager *TheFlowMgr;

FlowManager::FlowManager() : unk2c(0), unk2d(0), mPollables(this) {
    mFlowQueue.clear();
    unk94 = 0;
    unk7c = 0;
    unk80 = 0;
    unk18c = 0;
    unk190 = 0;
    for (int i = 0; i < 60; i++) {
        unk98[i] = 0;
    }
    mFlowOverlay = RndOverlay::Find("flow", false);
    mFlowPeakOverlay = RndOverlay::Find("flow_peak", false);
    mFlowTaskOverlay = RndOverlay::Find("flow_task", false);
    mFlowEventOverlay = RndOverlay::Find("flow_event", false);
}

FlowManager::~FlowManager() {}

void FlowManager::AddPollable(FlowNode *n) { mPollables.push_back(n); }
void FlowManager::RemovePollable(FlowNode *n) { mPollables.remove(n); }

void FlowManager::QueueCommand(FlowNode *n, FlowNode::QueueState q) {
    if (unk2d && q != FlowNode::kQueueOne) {
        n->Execute(q);
    } else
        mFlowQueue[n] = q;
}

void FlowManager::CancelCommand(FlowNode *n) { mFlowQueue[n] = FlowNode::kImmediate; }

void FlowManager::AddEventTime(Symbol s, float f1) {
    float fsub = f1 - unk190;
    if (unk64.find(s) != unk64.end()) {
        DataNode &n = unk64[s];
        float f7 = n.Array()->Float(0);
        int i5 = n.Array()->Int(1);
        float f8 = n.Array()->Float(2) + unk190;
        i5++;
        n.Array()->Node(0) = f7 + fsub;
        n.Array()->Node(1) = i5;
        n.Array()->Node(2) = f8;
    } else {
        DataArrayPtr ptr(fsub, 1, unk190);
        unk64[s] = ptr;
    }
    unk190 = 0;
}

void FlowManager::Poll() {
    float ms = 0;
    unk18c = 0;
    Timer timer;
    timer.Reset();
    timer.Start();
    unk2d = true;
    for (std::map<FlowNode *, FlowNode::QueueState>::iterator it = mFlowQueue.begin();
         it != mFlowQueue.end();
         ++it) {
        if (it->second != FlowNode::kImmediate) {
            it->first->Execute(it->second);
        }
    }
    mFlowQueue.clear();
    ObjPtrVec<FlowNode> polls(mPollables);
    for (ObjPtrVec<FlowNode>::iterator it = polls.begin(); it != polls.end(); ++it) {
        (*it)->Execute(FlowNode::kWhenAble);
    }
    unk2d = false;
    timer.Stop();
    unk2c = false;
}
