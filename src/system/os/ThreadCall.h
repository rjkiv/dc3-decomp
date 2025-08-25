#pragma once
#include <types.h>

extern u32 gMainThreadID;

void ThreadCallTerminate();
void ThreadCallPreInit();
void ThreadCallPoll();
void ThreadCallInit();

typedef int ThreadCallFunc(void);
typedef void ThreadCallCallbackFunc(int);

void ThreadCall(ThreadCallFunc *, ThreadCallCallbackFunc *);
void ThreadCall(class ThreadCallback *);
