#include "HamMemcardAction.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta/MemcardAction.h"
#include "meta/MemcardMgr.h"
#include "os/Memcard.h"

SaveMemcardAction::SaveMemcardAction(Profile *profile) : MemcardAction(profile) {
    SetResult(kMCNoError);
}

LoadMemcardAction::LoadMemcardAction(Profile *profile) : MemcardAction(profile) {
    SetResult(kMCNoError);
}

void SaveMemcardAction::PreAction() {
    FixedSizeSaveableStream fsss(
        TheMemcardMgr.mSaveDataBuffer, TheMemcardMgr.mSaveDataBufferSize, true
    );
    int ver = 92;
    fsss << ver;
    fsss.InitializeTable();
    fsss.EnableWriteEncryption();
    fsss << *GetProfile();
    fsss.DisableEncryption();
    fsss.SaveTable();
    SetResult(fsss.Fail() ? kMCGeneralError : kMCNoError);
}

void SaveMemcardAction::PostAction() {}

void LoadMemcardAction::PreAction() {}

void LoadMemcardAction::PostAction() {
    if (GetResult() == kMCNoError) {
        FixedSizeSaveableStream fsss(
            TheMemcardMgr.mSaveDataBuffer, TheMemcardMgr.mSaveDataBufferSize, true
        );
        int ver;
        fsss >> ver;
        FixedSizeSaveable::sCurrentMemcardLoadVer = ver;
        if (ver <= 91) {
            SetResult(kMCObsoleteVersion);
            return;
        }
        if (ver > 92) {
            SetResult(kMCNewerVersion);
            return;
        }
        fsss.LoadTable(ver);
        fsss.EnableReadEncryption();
        fsss >> *GetProfile();
        fsss.DisableEncryption();
        FixedSizeSaveable::sCurrentMemcardLoadVer = 92;
        SetResult(fsss.Fail() ? kMCGeneralError : kMCNoError);
    }
}
