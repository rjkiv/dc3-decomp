#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "stdlib.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "xdk/win_types.h"
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
    JobMgr(Hmx::Object *);
    ~JobMgr();
    void Poll();
    void CancelJob(int);
    void QueueJob(Job *);

    MEM_OVERLOAD(JobMgr, 0x2A);

private:
    void CancelAllJobs();

    Hmx::Object *mCallback; // 0x0
    std::list<Job *> mJobQueue; // 0x4
    bool mPreventStart; // 0xc
};

class SingleItemEnumJob : public Job {
public:
    SingleItemEnumJob(Hmx::Object *, int, QWORD);
    virtual ~SingleItemEnumJob();
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);

protected:
    void Poll();

    Hmx::Object *mCallback; // 0x8
    int mPadNum; // 0xc
    QWORD mOfferID; // 0x10
    int mState; // 0x18
    bool unk1c; // 0x1c - purchase made?
    void *unk20; // 0x20 - enumeration buffer?
    HANDLE mEnum; // 0x24
    XOVERLAPPED mOverlapped; // 0x28
};

class PostPurchaseEnumJob : public SingleItemEnumJob {
public:
    PostPurchaseEnumJob(Hmx::Object *, int, QWORD, Symbol, unsigned int);
    virtual void OnCompletion(Hmx::Object *);

private:
    Symbol mSource; // 0x48
    unsigned int mPurchaser; // 0x4c
};

class MultipleItemsEnumJob : public Job {
public:
    MultipleItemsEnumJob(Hmx::Object *, int, std::vector<QWORD> &);
    virtual ~MultipleItemsEnumJob();
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);

protected:
    void Poll();

    Hmx::Object *mCallback; // 0x8
    int mPadNum; // 0xc
    std::vector<QWORD> mOfferIDs; // 0x10
    std::vector<bool> unk1c; // 0x1c - purchased?
    int mState; // 0x30
    bool unk34;
    void *unk38; // 0x38 - enumeration buffer? - array of 0x68 sized structs?
    HANDLE mEnum; // 0x3c
    XOVERLAPPED mOverlapped; // 0x40
};

class MultipleItemsPostPurchaseEnumJob : public MultipleItemsEnumJob {
public:
    MultipleItemsPostPurchaseEnumJob(
        Hmx::Object *, int, std::vector<QWORD> &, Symbol, unsigned int
    );
    virtual void OnCompletion(Hmx::Object *);

private:
    Symbol mSource; // 0x5c
    unsigned int mPurchaser; // 0x60
};

#include "obj/Msg.h"

DECLARE_MESSAGE(SingleItemEnumCompleteMsg, "single_item_enum_complete")
SingleItemEnumCompleteMsg(bool success, bool hasOffer, const String &id)
    : Message(Type(), success, hasOffer, id) {}
void SetSuccess(bool success) { mData->Node(2) = success; }
void SetPurchaseMade(bool made) { mData->Node(3) = made; }
void SetOfferID(const String &id) { mData->Node(4) = id; }
bool Success() const { return mData->Int(2); }
bool HasOfferID() const { return mData->Int(3); }
QWORD OfferID() const { return _strtoui64(mData->Str(4), 0, 16); }
END_MESSAGE

DECLARE_MESSAGE(MultipleItemsEnumCompleteMsg, "multiple_items_enum_complete")
MultipleItemsEnumCompleteMsg(bool success, bool hasOffer, int numOffers, const String &str)
    : Message(Type(), success, hasOffer, numOffers, DataArrayPtr(), DataArrayPtr()) {}
void SetSuccess(bool success) { mData->Node(2) = success; }
void SetPurchaseMade(bool made) { mData->Node(3) = made; }
void SetNumOfferIDs(int num) {
    mData->Node(4) = num;
    mData->Array(5)->Resize(num);
    mData->Array(6)->Resize(num);
}
void SetOfferID(int i1, const String &str) { mData->Array(5)->Node(i1) = str; }
void SetPurchased(int i1, bool b1) { mData->Array(6)->Node(i1) = b1; }
QWORD OfferID(int) const;
END_MESSAGE
