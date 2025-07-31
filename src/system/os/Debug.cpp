#include "os/Debug.h"

const char *kAssertStr = "File: %s Line: %d Error: %s\n";
Debug TheDebug;

void Debug::SetDisabled(bool d){
    mNoDebug = d;
}

void Debug::StopLog(){
    RELEASE(mLog);
}

Debug::Debug()
    : mNoDebug(0), mFailing(0), mExiting(0), mNoTry(0), mNoModal(0), mTry(0), mLog(0),
      mReflect(0), mModalCallback(0), mFailThreadMsg(0), mNotifyThreadMsg(0) {}

Debug::~Debug() { StopLog(); }
