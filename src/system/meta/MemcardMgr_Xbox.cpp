#include "MemcardMgr.h"
#include "meta/MemcardAction.h"
#include "meta/MemcardMgr.h"
#include "meta/Profile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memcard.h"
#include "os/Memcard_Xbox.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"
#include "ui/UI.h"
#include "utl/Locale.h"
#include "xdk/XAPILIB.h"

namespace {
    const char *kSaveFilename = "save.dat";
}

MemcardMgr::MemcardMgr()
    : mState(kS_None), mAction(0), mForceOverwrite(0), mSelectDevicePadOverride(-1),
      mSelectDeviceWaiting(0), mSelectDeviceCallBackObj(0), mPadNum(-1), mProfile(0) {}

MemcardMgr::~MemcardMgr() {}

BEGIN_HANDLERS(MemcardMgr)
    HANDLE_MESSAGE(DeviceChosenMsg)
    HANDLE_MESSAGE(NoDeviceChosenMsg)
    HANDLE_MESSAGE(UIChangedMsg)
    HANDLE_MESSAGE(StorageChangedMsg)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void MemcardMgr::Init() {
    SetName("memcardmgr", ObjectDir::Main());
    TheMC.SetContainerName("savegame");
    static Symbol title_name("title_name");
    for (int i = 0; i < 4; i++) {
        mIsDeviceValid[i] = false;
        mPContainerTbl[i] = nullptr;
        mCidTbl[i].Set(i, 0);
    }
    WCHAR wideName[128];
    MultiByteToWideChar(
        0, 0, Localize(title_name, nullptr, TheLocale), -1, wideName, DIM(wideName)
    );
    TheMC.SetContainerDisplayName(wideName);
    ThePlatformMgr.AddSink(this);
}

int MemcardMgr::ThreadStart() {
    int ret = 0;
    switch (mState) {
    case kS_None:
        MILO_FAIL("ThreadCall triggered MemcardMgr::ThreadStart with no mode set.\n");
        break;
    case kS_Search:
        ret = ThreadCall_SearchForDevice();
        break;
    case kS_CheckForSaveContainer:
        ret = ThreadCall_CheckForSaveContainer();
        break;
    case kS_SaveGame:
        ret = ThreadCall_SaveGame();
        break;
    case kS_LoadGame:
        ret = ThreadCall_LoadGame();
        break;
    case kS_DeleteSaves:
        ret = TheMC.DeleteContainer(mCidTbl[mPadNum]);
        break;
    }
    return ret;
}

void MemcardMgr::ThreadDone(int mcResult) {
    State oldState = mState;
    mState = kS_None;
    switch (oldState) {
    case kS_None:
        MILO_FAIL("ThreadCall triggered MemcardMgr::ThreadDone with no mode set.\n");
        break;
    case kS_Search:
    case kS_CheckForSaveContainer: {
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    case kS_SaveGame:
    case kS_LoadGame: {
        mAction->SetResult((MCResult)mcResult);
        mAction->PostAction();
        if (mcResult == kMCNoError && mAction->GetResult() != kMCNoError) {
            mcResult = mAction->GetResult();
        }
        mAction = nullptr;
        mPadNum = -1;
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    case kS_DeleteSaves: {
        mPadNum = -1;
        MCResultMsg msg((MCResult)mcResult);
        Export(msg, true);
        break;
    }
    default:
        break;
    }
}

MCResult MemcardMgr::ThreadCall_SaveGame() {
    MCContainer *c = mPContainerTbl[mPadNum];
    ULONGLONG size = XContentCalculateSize(mSaveDataBufferSize * sizeof(int), 0);
    u64 i7 = 0;
    MCResult res = c->Mount(kOsOpenExisting);

    switch (res) {
    case kMCNoError:
        if (!mForceOverwrite) {
            c->Unmount();
            res = kMCFileExists;
        } else {
            u64 num48 = 0;
            MCResult mountRes = c->GetPathFreeSpace("", &num48);
            if (mountRes != kMCNoError || (i7 = num48, mForceOverwrite)) {
                int newSize = -1;
                c->GetSize(kSaveFilename, &newSize);
                if (newSize > 0) {
                    if (newSize < size) {
                        size -= newSize;
                    } else {
                        size = 0;
                    }
                }
                goto thing;
            } else {
                c->Unmount();
                res = kMCFileExists;
            }
        }
        break;
    case kMCCorrupt: {
        if (mForceOverwrite) {
            res = TheMC.DeleteContainer(c->Cid());
            if (res != kMCNoError) {
                return res;
            } else {
                goto thing;
            }
        }
        break;
    }
    thing:
    case kMCFileNotFound: {
        u64 freeSpace = 0;
        res = c->GetDeviceFreeSpace(&freeSpace);
        if (res == kMCNoError) {
            if (size > freeSpace + i7) {
                if (c->IsMounted()) {
                    c->Unmount();
                }
                res = kMCNotEnoughSpace;
            } else if (c->IsMounted() || (res = c->Mount(kOsOpenAlways)), res == kMCNoError) {
                MCResult writeRes = PerformWrite(c);
                res = c->Unmount();
                if (writeRes != kMCNoError) {
                    res = writeRes;
                }
            }
        }
        break;
    }
    default:
        break;
    }
    return res;
}

MCResult MemcardMgr::ThreadCall_LoadGame() {
    MCContainer *c = mPContainerTbl[mPadNum];
    MCResult res = c->Mount(kOsOpenExisting);
    if (res == kMCNoError) {
        MCResult readRes = PerformRead(c);
        res = c->Unmount();
        if (readRes != kMCNoError) {
            res = readRes;
        }
    }
    return res;
}

bool MemcardMgr::IsStorageDeviceValid(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x122);
    int padNum = pProfile->GetPadNum();
    MILO_ASSERT(padNum != -1, 0x124);
    if (mIsDeviceValid[padNum]) {
        mIsDeviceValid[padNum] = TheMC.IsDeviceValid(mCidTbl[padNum]);
    }
    return mIsDeviceValid[padNum];
}

void MemcardMgr::OnCheckForSaveContainer(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x1BF);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0x1C3);
    mState = kS_CheckForSaveContainer;
    ThreadCall(this);
}

void MemcardMgr::OnDeleteSaves(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x2EF);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0x2F4);
    mState = kS_DeleteSaves;
    ThreadCall(this);
}

void MemcardMgr::OnLoadGame(Profile *pProfile, MemcardAction *pAction) {
    MILO_ASSERT(pProfile, 0x2D9);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    mAction = pAction;
    MILO_ASSERT(mAction, 0x2DF);
    mAction->PreAction();
    MILO_ASSERT(mState == kS_None, 0x2E2);
    mState = kS_LoadGame;
    ThreadCall(this);
}

void MemcardMgr::OnSaveGame(Profile *pProfile, MemcardAction *pAction, int i3) {
    MILO_ASSERT(pProfile, 0x26D);
    mForceOverwrite = i3;
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    mAction = pAction;
    MILO_ASSERT(mAction, 0x274);
    mAction->PreAction();
    MCResult res = mAction->GetResult();
    if (res != kMCNoError) {
        MCResultMsg msg(res);
        Export(msg, true);
    } else {
        MILO_ASSERT(mState == kS_None, 0x281);
        mState = kS_SaveGame;
        ThreadCall(this);
    }
}

void MemcardMgr::OnSearchForDevice(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0xB5);
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    MILO_ASSERT(mState == kS_None, 0xBA);
    mState = kS_Search;
    ThreadCall(this);
}

void MemcardMgr::SelectDevice(
    Profile *pProfile, Hmx::Object *callbackObj, int i3, bool waiting
) {
    MILO_ASSERT(mSelectDeviceWaiting == false, 0x82);
    MILO_ASSERT(mSelectDeviceCallBackObj == NULL, 0x83);
    MILO_ASSERT(pProfile, 0x84);
    mProfile = pProfile;
    mSelectDeviceCallBackObj = callbackObj;
    mPadNum = mProfile->GetPadNum();
    if (ThePlatformMgr.GuideShowing()) {
        mSelectDevicePadOverride = i3;
        mSelectDeviceWaiting = true;
    } else {
        TheMC.ShowDeviceSelector(mCidTbl[mPadNum], this, i3, waiting);
    }
}

void MemcardMgr::SetDevice(unsigned int device) {
    mCidTbl[mPadNum].mDeviceId = (XCONTENTDEVICEID)device;
    if (mPContainerTbl[mPadNum]) {
        TheMC.DestroyContainer(mPContainerTbl[mPadNum]);
    }
    mPContainerTbl[mPadNum] = TheMC.CreateContainer(mCidTbl[mPadNum]);
    mIsDeviceValid[mPadNum] = TheMC.IsDeviceValid(mCidTbl[mPadNum]);
    mPadNum = -1;
}

MCResult MemcardMgr::PerformRead(MCContainer *container) {
    MCFile *file = container->CreateMCFile();
    int size = 0;
    MCResult res = container->GetSize(kSaveFilename, &size);
    if (res == kMCNoError) {
        if (size <= 0) {
            return kMCGeneralError;
        } else {
            if (size > mSaveDataBufferSize) {
                MILO_LOG(
                    "%s [MemcardMgrXbox] Found save file that is %d bytes, but we only have RAM for %d bytes.\n",
                    kSaveFilename,
                    size,
                    mSaveDataBufferSize
                );
            }
            MCResult openRes = file->Open(kSaveFilename, kOsModeRead, kOsOpenExisting);
            if (openRes != kMCNoError) {
                container->DestroyMCFile(file);
                return openRes;
            } else {
                int minSize = Min<int>(size, mSaveDataBufferSize);
                MCResult readRes = file->Read(mSaveDataBuffer, minSize);
                MCResult closeRes = file->Close();
                container->DestroyMCFile(file);
                return readRes == kMCNoError ? closeRes : readRes;
            }
        }
    }
    return res;
}

MCResult MemcardMgr::PerformWrite(MCContainer *container) {
    MCFile *file = container->CreateMCFile();
    MCResult res = file->Open(kSaveFilename, kOsModeWrite, kOsOpenAlways);
    if (res != kMCNoError) {
        container->DestroyMCFile(file);
        return res;
    } else {
        MCResult writeRes = file->Write(mSaveDataBuffer, mSaveDataBufferSize);
        MCResult closeRes = file->Close();
        container->DestroyMCFile(file);
        return writeRes == kMCNoError ? closeRes : writeRes;
    }
}

MCResult MemcardMgr::ThreadCall_CheckForSaveContainer() {
    MCContainer *container = mPContainerTbl[mPadNum];
    MCResult res = container->Mount(kOsOpenExisting);
    if (res == kMCNoError) {
        MCFile *file = container->CreateMCFile();
        MCResult openRes = file->Open(kSaveFilename, kOsModeWrite, kOsOpenExisting);
        container->DestroyMCFile(file);
        res = container->Unmount();
        if (openRes != kMCNoError) {
            res = openRes;
        }
    }
    return res;
}

MCResult MemcardMgr::ThreadCall_SearchForDevice() {
    MILO_ASSERT(mSelectDeviceWaiting == false, 0x9D);
    MILO_ASSERT(mProfile, 0x9E);
    MILO_ASSERT(mPadNum != -1, 0x9F);
    mCidTbl[mPadNum].mDeviceId = XCONTENTDEVICE_ANY;
    MCResult res = TheMC.FindValidUnit(&mCidTbl[mPadNum]);
    if (res == kMCFileExists) {
        if (mPContainerTbl[mPadNum]) {
            TheMC.DestroyContainer(mPContainerTbl[mPadNum]);
        }
        mPContainerTbl[mPadNum] = TheMC.CreateContainer(mCidTbl[mPadNum]);
        mIsDeviceValid[mPadNum] = TheMC.IsDeviceValid(mCidTbl[mPadNum]);
    }
    return res;
}

DataNode MemcardMgr::OnMsg(const DeviceChosenMsg &msg) {
    SetDevice(msg.Device());
    if (mSelectDeviceCallBackObj) {
        mSelectDeviceCallBackObj->Handle(msg, true);
        mSelectDeviceCallBackObj = nullptr;
    }
    return 0;
}

DataNode MemcardMgr::OnMsg(const NoDeviceChosenMsg &msg) {
    mPadNum = -1;
    if (mSelectDeviceCallBackObj) {
        mSelectDeviceCallBackObj->Handle(msg, true);
        mSelectDeviceCallBackObj = nullptr;
    }
    return 0;
}

DataNode MemcardMgr::OnMsg(const UIChangedMsg &msg) {
    if (mSelectDeviceWaiting) {
        if (!msg.Showing()) {
            mSelectDeviceWaiting = false;
            TheMC.ShowDeviceSelector(
                mCidTbl[mPadNum], this, mSelectDevicePadOverride, false
            );
        }
    }
    return 0;
}

DataNode MemcardMgr::OnMsg(const StorageChangedMsg &msg) {
    for (int i = 0; i < 4; i++) {
        if (mPContainerTbl[i]) {
            mIsDeviceValid[i] = TheMC.IsDeviceValid(mCidTbl[i]);
        }
    }
    return 0;
}

DataNode MemcardMgr::OnMsg(const SigninChangedMsg &msg) {
    if (mSelectDeviceWaiting) {
        if (ThePlatformMgr.HasPadNumsSigninChanged(mProfile->GetPadNum())) {
            mSelectDeviceWaiting = false;
            if (mSelectDeviceCallBackObj) {
                static NoDeviceChosenMsg msg;
                mSelectDeviceCallBackObj->Handle(msg, true);
                mSelectDeviceCallBackObj = nullptr;
            }
        }
    }
    int mask = msg.GetMask();
    for (int i = 0; i < 4; i++) {
        if (!(mask & 1 << i)) {
            if (mPContainerTbl[i] && !mPContainerTbl[i]->IsMounted()) {
                TheMC.DestroyContainer(mPContainerTbl[i]);
                mPContainerTbl[i] = nullptr;
            }
            mIsDeviceValid[i] = false;
        }
    }
    return 0;
}
