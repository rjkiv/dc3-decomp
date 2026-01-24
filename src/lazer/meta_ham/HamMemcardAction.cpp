#include "HamMemcardAction.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta/MemcardAction.h"
#include "meta/MemcardMgr.h"
#include "os/Memcard.h"

SaveMemcardAction::SaveMemcardAction(Profile *profile) : MemcardAction(profile) {
    mResult = kMCNoError;
}

LoadMemcardAction::LoadMemcardAction(Profile *profile) : MemcardAction(profile) {
    mResult = kMCNoError;
}

void SaveMemcardAction::PreAction() {
    FixedSizeSaveableStream fsss(TheMemcardMgr.unk34, TheMemcardMgr.unk38, true);
    int ver = 92;
    fsss << ver;
    fsss.InitializeTable();
    fsss.EnableWriteEncryption();
    fsss << *unk10;
    fsss.DisableEncryption();
    fsss.SaveTable();
    mResult = fsss.Fail() ? kMCGeneralError : kMCNoError;
}

void SaveMemcardAction::PostAction() {}

void LoadMemcardAction::PreAction() {}

void LoadMemcardAction::PostAction() {
    if (mResult == kMCNoError) {
        FixedSizeSaveableStream fsss(TheMemcardMgr.unk34, TheMemcardMgr.unk38, true);
        int ver;
        fsss >> ver;
        FixedSizeSaveable::sCurrentMemcardLoadVer = ver;
        if (ver <= 91) {
            mResult = kMCObsoleteVersion;
            return;
        }
        if (ver > 92) {
            mResult = kMCNewerVersion;
            return;
        }
        fsss.LoadTable(ver);
        fsss.EnableReadEncryption();
        fsss >> *unk10;
        fsss.DisableEncryption();
        FixedSizeSaveable::sCurrentMemcardLoadVer = 92;
        mResult = fsss.Fail() ? kMCGeneralError : kMCNoError;
    }
}
