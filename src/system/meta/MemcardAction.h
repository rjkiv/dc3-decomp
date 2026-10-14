#pragma once

#include "os/Memcard.h"
class Profile;

class MemcardAction {
public:
    MemcardAction(Profile *);
    virtual ~MemcardAction() {}
    virtual void PreAction() = 0;
    virtual void PostAction() = 0;

    MCResult GetResult() { return mResult; }
    void SetResult(MCResult res) { mResult = res; }
    char *GetBuffer() { return mBuffer; }
    int GetBufSize() { return mBufSize; }
    Profile *GetProfile() { return mProfile; }

private:
    MCResult mResult; // 0x4
    char *mBuffer; // 0x8
    int mBufSize; // 0xc
    Profile *mProfile; // 0x10
};
