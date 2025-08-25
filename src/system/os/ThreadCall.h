#pragma once

void ThreadCallTerminate();
void ThreadCallPreInit();
void ThreadCallPoll();
void ThreadCallInit();

typedef int ThreadCallFunc(void);
typedef void ThreadCallCallbackFunc(int);

void ThreadCall(ThreadCallFunc *, ThreadCallCallbackFunc *);
