#include "ThreadCall.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "xdk/XAPILIB.h"
#include <process.h>

namespace {
    bool gReadyForNext = true;
    ThreadCallData gData[12];
    HANDLE gThreadHandle;
    HANDLE gThreadSema;
    bool gTerminate;
    bool gCallDone;
    int gCurCall;
    int gFreeCall;

    uint MyThreadFunc(void *) {
        DWORD res = WaitForSingleObject(gThreadSema, -1);
        while (!gTerminate) {
            switch (gData[gCurCall].mType) {
            case kTCDT_Func:
                gData[gCurCall].mArg = gData[gCurCall].mFunc();
                gCallDone = true;
                break;
            case kTCDT_Class:
                gData[gCurCall].mArg = gData[gCurCall].mClass->ThreadStart();
                gCallDone = true;
                break;
            default:
                MILO_ASSERT(false, 199);
                break;
            }
            res = WaitForSingleObject(gThreadSema, -1);
        }
        CloseHandle(gThreadSema);
        return 0;
    }
}

DWORD gMainThreadID = -1;

namespace JobQueue {
    struct Entry {
        int lol;
    };

    int shouldExit;
    int numWorkers;
    int maxJobs;
    volatile int numIdle;
    Entry *allEntries;
    Entry *idleEntries;
    Entry *waitingEntries;
    void **jobThreadHandle;
    CriticalSection jobQueueMutex;
}

void ThreadCallInit() {
    memset(gData, 0, sizeof(gData));
    gCurCall = 0;
    gFreeCall = 0;
    gThreadSema = CreateSemaphoreA(nullptr, 0, 1, nullptr);
    if (!gThreadSema) {
        MILO_LOG("CreateSemaphore() failed.(%d)\n", gThreadSema);
    } else {
        gThreadHandle =
            _beginthreadex(nullptr, 0x10000, MyThreadFunc, nullptr, 0, nullptr);
        if (!gThreadSema) {
            MILO_LOG("_beginthreadex() failed.(%d)\n", gThreadHandle);
        }
    }
}

void ThreadCall(ThreadCallFunc *func, ThreadCallCallbackFunc *callback) {
    ThreadCallData &data = gData[gFreeCall];
    MILO_ASSERT(data.mType == kTCDT_None, 0x6E);
    data.mFunc = func;
    data.mCallback = callback;
    data.mType = kTCDT_Func;
    data.mClass = nullptr;
    gFreeCall = (gFreeCall + 1) % 12;
}

void ThreadCall(ThreadCallback *callback) {
    ThreadCallData &data = gData[gFreeCall];
    MILO_ASSERT(data.mType == kTCDT_None, 0x7B);
    data.mType = kTCDT_Class;
    data.mFunc = nullptr;
    data.mCallback = nullptr;
    data.mClass = callback;
    gFreeCall = (gFreeCall + 1) % 12;
}

void ThreadCallPoll() {
    if (gCallDone) {
        ThreadCallData &data = gData[gCurCall];
        ThreadCallDataType oldType = data.mType;
        if (data.mType) {
            data.mType = kTCDT_None;
            gCallDone = false;
            gCurCall = (gCurCall + 1) % 12;
            gReadyForNext = true;
            switch (oldType) {
            case kTCDT_None:
                MILO_ASSERT(false, 0x97);
                break;
            case kTCDT_Func:
                data.mCallback(data.mArg);
                break;
            case kTCDT_Class:
                data.mClass->ThreadDone(data.mArg);
                break;
            }
        }
    }
    if (gReadyForNext && gData[gCurCall].mType != kTCDT_None) {
        gReadyForNext = false;
        ReleaseSemaphore(gThreadSema, 1, nullptr);
    }
}

void ThreadCallPreInit() { gMainThreadID = GetCurrentThreadId(); }

void ThreadCallTerminate() {
    if (gThreadHandle) {
        gTerminate = true;
        ReleaseSemaphore(gThreadSema, 1, nullptr);
    }
}
