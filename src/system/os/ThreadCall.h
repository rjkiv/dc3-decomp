#pragma once
#include "xdk/win_types.h"

extern DWORD gMainThreadID;

void ThreadCallTerminate();
void ThreadCallPreInit();
void ThreadCallPoll();
void ThreadCallInit();

typedef int ThreadCallFunc(void);
typedef void ThreadCallCallbackFunc(int);

enum ThreadCallDataType {
    kTCDT_None = 0,
    kTCDT_Func = 1,
    kTCDT_Class = 2
};

class ThreadCallback {
public:
    ThreadCallback() {}
    virtual ~ThreadCallback() {}
    virtual int ThreadStart() = 0;
    virtual void ThreadDone(int) = 0;
};

struct ThreadCallData {
    ThreadCallDataType mType; // 0x0
    ThreadCallFunc *mFunc; // 0x4
    ThreadCallCallbackFunc *mCallback; // 0x8
    ThreadCallback *mClass; // 0xc
    int mArg; // 0x10
};

void ThreadCall(ThreadCallFunc *, ThreadCallCallbackFunc *);
void ThreadCall(ThreadCallback *);
