#include "os/Debug.h"
#include <vector>

const char *kAssertStr = "File: %s Line: %d Error: %s\n";
Debug TheDebug;
std::vector<String> gNotifies;

typedef void ModalCallbackFunc(Debug::ModalType &, FixedString&, bool);

void Debug::SetDisabled(bool d){
    mNoDebug = d;
}

void Debug::StopLog(){
    RELEASE(mLog);
}

ModalCallbackFunc* Debug::SetModalCallback(ModalCallbackFunc* func){
    if (mNoModal)
        return nullptr;
    ModalCallbackFunc *oldFunc = mModalCallback;
    mModalCallback = func;
    if (!gNotifies.empty()) {
        for (int i = 0; i < gNotifies.size(); i++) {
            MILO_WARN("%s\n", gNotifies[i].Str());
        }
        gNotifies.clear();
    }
    return oldFunc;
}

void DebugModal(enum Debug::ModalType &ty, class FixedString &str, bool b3){
    if(ty == 2){
        str += "\n\n-- Program ended --\n";
    }
    else {
        gNotifies.push_back(str.Str());
    }
    MILO_LOG("%s\n", str.Str());
}

Debug::Debug()
    : mNoDebug(0), mFailing(0), mExiting(0), mNoTry(0), mNoModal(0), mTry(0), mLog(0), mAlwaysFlush(0),
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
