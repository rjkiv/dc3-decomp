#pragma once
#include "obj/Object.h"
#include "utl/JobMgr.h"
#include "xdk/XAPILIB.h"

class XboxSessionJob : public Job {
public:
    XboxSessionJob(void *);
    virtual ~XboxSessionJob();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void CheckError(DWORD, XOVERLAPPED *);

protected:
    XOVERLAPPED mXOverlapped; // 0x8
    HANDLE mSession; // 0x24
    bool mSuccess; // 0x28
};

class StartSessionJob : public XboxSessionJob {
public:
    StartSessionJob(void *v);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);
};

class EndSessionJob : public XboxSessionJob {
public:
    EndSessionJob(void *v);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);
};

class WriteCareerLeaderboardJob : public XboxSessionJob {
public:
    WriteCareerLeaderboardJob(void *, int, int, u64, u64);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);

protected:
    XUID mXUID; // 0x30;
    XUSER_PROPERTY mUserProp; // 0x38
    XSESSION_VIEW_PROPERTIES mSessionViewProp; // 0x50
};

class MakeSessionJob : public Job {
public:
    MakeSessionJob(HANDLE *, DWORD, int);
    virtual void Start();
    virtual bool IsFinished();
    virtual void Cancel(Hmx::Object *);
    virtual void OnCompletion(Hmx::Object *);
    virtual void CheckError(DWORD, XOVERLAPPED *);

protected:
    HANDLE *mSession; // 0x8
    DWORD mSessionFlags; // 0xc
    int mUserIndex; // 0x10
    XSESSION_INFO mSessionInfo; // 0x14
    XOVERLAPPED mXOverlapped; // 0x50
    bool mSuccess; // 0x6c
};

class DeleteSessionJob : public XboxSessionJob {
public:
    DeleteSessionJob(void *v);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);
};

class AddLocalPlayerJob : public XboxSessionJob {
public:
    AddLocalPlayerJob(void *, int, bool);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);

protected:
    DWORD mUserIndex; // 0x2c
    BOOL mPrivateSlot; // 0x30
};

class RemoveLocalPlayerJob : public XboxSessionJob {
public:
    RemoveLocalPlayerJob(void *, int);
    virtual void Start();
    virtual void OnCompletion(Hmx::Object *);

protected:
    DWORD mUserIndex; // 0x2c
};
