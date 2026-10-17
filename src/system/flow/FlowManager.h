#pragma once
#include "obj/Object.h"
#include "flow/FlowNode.h"
#include "rndobj/Overlay.h"
#include <map>

class FlowManager : public Hmx::Object {
public:
    struct PerfEventEntry {
        float ms; // 0x0
        const char *pep; // 0x4
        const char *name; // 0x8
    };

    FlowManager();
    virtual ~FlowManager();

    void AddPollable(FlowNode *);
    void RemovePollable(FlowNode *);
    void Poll();
    void AddEventTime(Symbol, float);
    void QueueCommand(FlowNode *, FlowNode::QueueState);
    void CancelCommand(FlowNode *);
    void AddReleaseTime(float ms) { mReleaseTime += ms; }

protected:
    bool mEditUpdate; // 0x2c
    bool mInPoll; // 0x2d
    std::map<FlowNode *, FlowNode::QueueState> mNodeList; // 0x30
    ObjPtrVec<FlowNode> mPollList; // 0x48
    std::map<Symbol, DataNode> mEventTimes; // 0x64
    float mReleaseTime; // 0x7c
    float mWorstFrame; // 0x80
    RndOverlay *mOverlay; // 0x84
    RndOverlay *mPeakOverlay; // 0x88
    RndOverlay *mTaskOverlay; // 0x8c
    RndOverlay *mEventOverlay; // 0x90
    int mFrameIdx; // 0x94
    float mAverages[60]; // 0x98
    float mCurAverage; // 0x188
    float mTaskTime; // 0x18c
    float mCurTaskTime; // 0x190
    DataNode mPeakFrameData; // 0x194
};

extern FlowManager *TheFlowMgr;
