#include "meta/MemcardAction.h"

MemcardAction::MemcardAction(Profile *p)
    : mResult(kMCGeneralError), mBuffer(0), mBufSize(-1), mProfile(p) {}
