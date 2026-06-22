#include "utl/JobMgr.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Std.h"

namespace {
    static int gJobIDCounter = 0;
}

Job::Job() { mID = gJobIDCounter++; }

void JobMgr::Poll() {
    if (!mJobQueue.empty()) {
        if (mJobQueue.front()->IsFinished()) {
            Job *first = mJobQueue.front();
            mJobQueue.pop_front();
            mPreventStart = true;
            first->OnCompletion(mCallback);
            delete first;
            mPreventStart = false;
            if (!mJobQueue.empty()) {
                mJobQueue.front()->Start();
            }
        }
    }
}

void JobMgr::CancelJob(int id) {
    FOREACH (it, mJobQueue) {
        Job *toBeCancelled = *it;
        if (toBeCancelled->ID() == id) {
            int firstID = mJobQueue.front()->ID();
            auto erased = mJobQueue.erase(it);
            bool wasPreventingStart = mPreventStart;
            mPreventStart = true;
            toBeCancelled->Cancel(mCallback);
            mPreventStart = wasPreventingStart;
            if (firstID == id && !wasPreventingStart && erased != mJobQueue.end()) {
                (*erased)->Start();
            }
            delete toBeCancelled;
            return;
        }
    }
    MILO_NOTIFY("This job is not in the queue %i", id);
}

JobMgr::JobMgr(Hmx::Object *o) : mCallback(o), mJobQueue(), mPreventStart(0) {}

void JobMgr::QueueJob(Job *j) {
    mJobQueue.push_back(j);
    if (mJobQueue.size() == 1 && !mPreventStart) {
        mJobQueue.front()->Start();
    }
}

void JobMgr::CancelAllJobs() {
    std::list<Job *> list = mJobQueue;
    mJobQueue.clear();
    FOREACH (it, list) {
        (*it)->Cancel(mCallback);
        delete *it;
    }
}

JobMgr::~JobMgr() { CancelAllJobs(); }
