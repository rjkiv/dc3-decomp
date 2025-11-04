#include "utl/JobMgr.h"
#include "obj/Object.h"
#include "os/Debug.h"

namespace {
    int gJobIDCounter;
}

Job::Job() { mID = gJobIDCounter++; }

void JobMgr::Poll() {}

void JobMgr::CancelJob(int id) {
    for (std::list<Job *>::iterator it = mJobQueue.begin(); it != mJobQueue.end(); ++it) {
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
    for (std::list<Job *>::const_iterator it = list.begin(); it != list.end(); ++it) {
        (*it)->Cancel(mCallback);
        delete *it;
    }
}

JobMgr::~JobMgr() { CancelAllJobs(); }
