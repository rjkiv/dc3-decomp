#pragma once

#include "obj/Object.h"

class Job {
public:
    Job();
    virtual ~Job() {}
    virtual void Start() = 0;
    virtual bool IsFinished() = 0;
    virtual void Cancel(Hmx::Object *) = 0;
    virtual void OnCompletion(Hmx::Object *) {}

    int ID() const { return mID; }

    int mID;
};

class JobMgr {
public:
    void Poll();
    void CancelJob(int);
    JobMgr(Hmx::Object *);
    void QueueJob(Job *);
    ~JobMgr();

    Hmx::Object *mCallback; // 0x0
    std::list<Job *> mJobQueue; // 0x4
    bool mPreventStart; // 0xc

private:
    void CancelAllJobs();
};
