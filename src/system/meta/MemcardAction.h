#pragma once

#include "os/Memcard.h"
class Profile;

class MemcardAction {
public:
    MemcardAction(Profile *);
    virtual ~MemcardAction() {}
    virtual void PreAction() = 0;
    virtual void PostAction() = 0;

    MCResult Result() const { return mResult; }
    void SetResult(MCResult res) { mResult = res; }

private:
    MCResult mResult; // 0x4
    int unk8;
    int unkc;
    Profile *unk10;
};
