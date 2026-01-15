#pragma once
#include "obj/Object.h"
#include "stdlib.h"
#include "utl/MemMgr.h"
#include "xdk/xapilibi/xbase.h"

class Job {
public:
    Job();
    virtual ~Job() {}
    virtual void Start() = 0;
    virtual bool IsFinished() = 0;
    virtual void Cancel(Hmx::Object *) = 0;
    virtual void OnCompletion(Hmx::Object *) {}

    int ID() const { return mID; }

    MEM_OVERLOAD(Job, 0x11);

private:
    int mID; // 0x4
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

class SingleItemEnumJob : public Job {
public:
    SingleItemEnumJob(Hmx::Object *, int, u64);
    virtual ~SingleItemEnumJob();
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);

protected:
    Hmx::Object *unk8;
    int unkc;
    u64 unk10;
    int unk18;
    bool unk1c;
    int unk20;
    int unk24;
    XOVERLAPPED unk28;
};

#include "obj/Msg.h"

DECLARE_MESSAGE(SingleItemEnumCompleteMsg, "single_item_enum_complete")
bool Success() const { return mData->Int(2); }
bool HasOfferID() const { return mData->Int(3); }
unsigned long long OfferID() const { return _strtoui64(mData->Str(4), 0, 16); }
END_MESSAGE
