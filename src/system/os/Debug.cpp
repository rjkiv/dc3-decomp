#include "os/Debug.h"

const char *kAssertStr = "File: %s Line: %d Error: %s\n";
Debug TheDebug;

typedef void ModalCallbackFunc(Debug::ModalType &, FixedString&, bool);

void Debug::SetDisabled(bool d){
    mNoDebug = d;
}

void Debug::StopLog(){
    RELEASE(mLog);
}

void DebugModal(enum Debug::ModalType &, class FixedString &, bool){

}

Debug::Debug()
    : mNoDebug(0), mFailing(0), mExiting(0), mNoTry(0), mNoModal(0), mTry(0), mLog(0),
      mReflect(0), mModalCallback(DebugModal), unk38(0), mFailThreadMsg(0), mNotifyThreadMsg(0), unk10c(0), unk110(0) {}

void Debug::RemoveExitCallback(ExitCallbackFunc *func) {
    // TODO:
    // target: ?remove@?$list@P6AXXZV?$StlNodeAlloc@P6AXXZ@stlpmtx_std@@@stlpmtx_std@@QAAXABQ6AXXZ@Z
    // base: ?remove@?$_List_impl@PAXV?$allocator@PAX@stlp_std@@@stlp_std@@QAAXABQAX@Z
    if (!mExiting) {
        mExitCallbacks.remove(func);
    }
}
      
Debug::~Debug() { StopLog(); }
