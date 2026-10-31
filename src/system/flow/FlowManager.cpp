#include "flow/FlowManager.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "utl/MakeString.h"

FlowManager *TheFlowMgr;

FlowManager::FlowManager() : mEditUpdate(0), mInPoll(0), mPollList(this) {
    mNodeList.clear();
    mFrameIdx = 0;
    mReleaseTime = 0;
    mWorstFrame = 0;
    mTaskTime = 0;
    mCurTaskTime = 0;
    for (int i = 0; i < 60; i++) {
        mAverages[i] = 0;
    }
    mOverlay = RndOverlay::Find("flow", false);
    mPeakOverlay = RndOverlay::Find("flow_peak", false);
    mTaskOverlay = RndOverlay::Find("flow_task", false);
    mEventOverlay = RndOverlay::Find("flow_event", false);
}

FlowManager::~FlowManager() {}

void FlowManager::AddPollable(FlowNode *n) { mPollList.push_back(n); }
void FlowManager::RemovePollable(FlowNode *n) { mPollList.remove(n); }

void FlowManager::QueueCommand(FlowNode *n, FlowNode::QueueState q) {
    if (mInPoll && q != FlowNode::kAfter) {
        n->Execute(q);
    } else
        mNodeList[n] = q;
}

void FlowManager::CancelCommand(FlowNode *n) { mNodeList[n] = FlowNode::kQueueIgnore; }

void FlowManager::AddEventTime(Symbol s, float f1) {
    float fsub = f1 - mCurTaskTime;
    if (mEventTimes.find(s) != mEventTimes.end()) {
        DataNode &n = mEventTimes[s];
        float f7 = n.Array()->Float(0);
        int i5 = n.Array()->Int(1);
        float f8 = n.Array()->Float(2) + mCurTaskTime;
        i5++;
        n.Array()->Node(0) = f7 + fsub;
        n.Array()->Node(1) = i5;
        n.Array()->Node(2) = f8;
    } else {
        DataArrayPtr ptr(fsub, 1, mCurTaskTime);
        mEventTimes[s] = ptr;
    }
    mCurTaskTime = 0;
}

void FlowManager::Poll() {
    int numNodes = mNodeList.size();
    float f17 = 0;
    float oldTaskTime = mTaskTime;
    mTaskTime = 0;
    float release = mReleaseTime;

    Timer timer;
    timer.Reset();
    timer.Start();
    mInPoll = true;
    FOREACH (it, mNodeList) {
        if (it->second != FlowNode::kQueueIgnore) {
            it->first->Execute(it->second);
        }
    }
    mNodeList.clear();
    ObjPtrVec<FlowNode> polls(mPollList);
    FOREACH (it, polls) {
        (*it)->Execute(FlowNode::kPolling);
    }
    mInPoll = false;
    timer.Stop();

    float f0 = mTaskTime + oldTaskTime;
    mEditUpdate = false;
    float timerMs = timer.Ms() - mTaskTime;
    Symbol s114 = 0;
    Symbol s11c = 0;
    float f18 = -1;
    float f19 = -1;
    if (mEventTimes.size() != 0) {
        if (mEventOverlay->Showing()) {
            *mEventOverlay << "\n\n\n\n\n\n\n\n\n\n";
        }
        FOREACH (it, mEventTimes) {
            Symbol key = it->first;
            DataNode value = it->second;
            float f14 = value.Array()->Float(0);
            float f20 = value.Array()->Float(2);
            f17 += f14;
            if (f14 >= f18) {
                f18 = f14;
                s114 = key;
            }
            if (f20 >= f19) {
                f19 = f20;
                s11c = key;
            }
            if (mEventOverlay->Showing()) {
                float f134 = value.Array()->Float(2);
                float f138 = value.Array()->Float(0);
                int f13c = value.Array()->Int(1);
                *mEventOverlay << MakeString(
                    "%s    count: %i   time: %.3f ms   task: %.3f ms\n",
                    key.Str(),
                    f13c,
                    f138,
                    f134
                );
            }
        }
        if (mOverlay->Showing()) {
            *mOverlay << MakeString(
                "Worst:   FlowTime: %s  %.3f    TaskTime: %s  %.3f\n",
                s114.Str(),
                f18,
                s11c.Str(),
                f19
            );
        }
    }
    float totalEventMs = (timerMs + f17) + mReleaseTime;
    if (mOverlay->Showing()) {
        *mOverlay << MakeString(
            "Events: %.3f ms  %i Commands in %.3f ms  Release: %.3f ms  Tasks: %.3f ms\n",
            f17,
            numNodes,
            timerMs,
            release,
            f0
        );
    }
    mAverages[mFrameIdx] = totalEventMs;
    mFrameIdx++;
    if (totalEventMs > mWorstFrame) {
        mWorstFrame = totalEventMs;
        DataArrayPtr ptr(s114, f18, s11c, f19, totalEventMs);
        mPeakFrameData = ptr;
    }
    float peak = mWorstFrame;
    if (mFrameIdx >= 60) {
        mCurAverage = 0;
        for (int i = 0; i < 60; i++) {
            mCurAverage += mAverages[i];
        }
        mWorstFrame = 0;
        mFrameIdx = 0;
        mCurAverage /= 60;
        if (mPeakFrameData.Type() == kDataArray) {
            DataArray *a = mPeakFrameData.Array();
            if (a->Float(1) > 0 && mPeakOverlay->Showing()) {
                *mPeakOverlay << MakeString("%s %.3f ms\n", a->Sym(0), a->Float(1));
            }
            if (a->Float(3) > 0 && mTaskOverlay->Showing()) {
                *mTaskOverlay << MakeString("%s %.3f ms\n", a->Sym(2), a->Float(3));
            }
        }
    }
    if (mOverlay->Showing()) {
        *mOverlay << MakeString(
            "Average: %.3f ms   Peak: %.3f ms    Frame: %.3f ms\n",
            mCurAverage,
            peak,
            totalEventMs
        );
    }
    Timer *t = AutoTimer::GetTimer("flow");
    if (t) {
        t->SetLastMs(totalEventMs);
    }
    mEventTimes.clear();
    mReleaseTime = 0;
    mTaskTime = 0;
}
