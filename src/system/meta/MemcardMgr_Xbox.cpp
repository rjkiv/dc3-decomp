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
#include "xdk/xapilibi/xbase.h"

namespace {
    const char *kSaveFilename = "save.dat";
}

MemcardMgr::MemcardMgr()
    : mState(kS_None), mAction(0), unk40(0), unk88(-1), mSelectDeviceWaiting(0),
      mSelectDeviceCallBackObj(0), mPadNum(-1), mProfile(0) {}

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
        mValidDevices[i] = false;
        mContainers[i] = nullptr;
        mContainerIDs[i].Set(i, 0);
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
        ret = TheMC.DeleteContainer(mContainerIDs[mPadNum]);
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
        if (mcResult == 0) {
            MCResult res = mAction->Result();
            if (res != kMCNoError) {
                mcResult = res;
            }
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

bool MemcardMgr::IsStorageDeviceValid(Profile *pProfile) {
    MILO_ASSERT(pProfile, 0x122);
    int padNum = pProfile->GetPadNum();
    MILO_ASSERT(padNum != -1, 0x124);
    if (mValidDevices[padNum]) {
        mValidDevices[padNum] = TheMC.IsDeviceValid(mContainerIDs[padNum]);
    }
    return mValidDevices[padNum];
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
    unk40 = i3;
    mProfile = pProfile;
    mPadNum = mProfile->GetPadNum();
    mAction = pAction;
    MILO_ASSERT(mAction, 0x274);
    mAction->PreAction();
    MCResult res = mAction->Result();
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
        unk88 = i3;
        mSelectDeviceWaiting = true;
    } else {
        TheMC.ShowDeviceSelector(mContainerIDs[mPadNum], this, i3, waiting);
    }
}

void MemcardMgr::SetDevice(unsigned int device) {
    mContainerIDs[mPadNum].mDeviceId = (XCONTENTDEVICEID)device;
    if (mContainers[mPadNum]) {
        TheMC.DestroyContainer(mContainers[mPadNum]);
    }
    mContainers[mPadNum] = TheMC.CreateContainer(mContainerIDs[mPadNum]);
    mValidDevices[mPadNum] = TheMC.IsDeviceValid(mContainerIDs[mPadNum]);
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
            if (size > unk38) {
                MILO_LOG(
                    "%s [MemcardMgrXbox] Found save file that is %d bytes, but we only have RAM for %d bytes.\n",
                    kSaveFilename,
                    size,
                    unk38
                );
            }
            MCResult openRes = file->Open(kSaveFilename, kAccessRead, (CreateType)0);
            if (openRes != kMCNoError) {
                container->DestroyMCFile(file);
                return openRes;
            } else {
                int minSize = Min(size, unk38);
                MCResult readRes = file->Read(unk34, minSize);
                MCResult closeRes = file->Close();
                container->DestroyMCFile(file);
                if (readRes == kMCNoError) {
                    return closeRes;
                } else {
                    return readRes;
                }
            }
        }
    }
}

MCResult MemcardMgr::PerformWrite(MCContainer *container) {
    MCFile *file = container->CreateMCFile();
    MCResult res = file->Open(kSaveFilename, kAccessWrite, (CreateType)1);
    if (res != kMCNoError) {
        container->DestroyMCFile(file);
        return res;
    } else {
        MCResult writeRes = file->Write(unk34, unk38);
        MCResult closeRes = file->Close();
        container->DestroyMCFile(file);
        if (writeRes != kMCNoError) {
            closeRes = writeRes;
        }
        return closeRes;
    }
}

MCResult MemcardMgr::ThreadCall_CheckForSaveContainer() {
    MCContainer *container = mContainers[mPadNum];
    MCResult res = container->Mount((CreateType)0);
    if (res == kMCNoError) {
        MCFile *file = container->CreateMCFile();
        MCResult openRes = file->Open(kSaveFilename, kAccessWrite, (CreateType)0);
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
    mContainerIDs[mPadNum].mDeviceId = XCONTENTDEVICE_ANY;
    MCResult res = TheMC.FindValidUnit(&mContainerIDs[mPadNum]);
    if (res == kMCFileExists) {
        if (mContainers[mPadNum]) {
            TheMC.DestroyContainer(mContainers[mPadNum]);
        }
        mContainers[mPadNum] = TheMC.CreateContainer(mContainerIDs[mPadNum]);
        mValidDevices[mPadNum] = TheMC.IsDeviceValid(mContainerIDs[mPadNum]);
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
            TheMC.ShowDeviceSelector(mContainerIDs[mPadNum], this, unk88, false);
        }
    }
    return 0;
}

DataNode MemcardMgr::OnMsg(const StorageChangedMsg &msg) {
    for (int i = 0; i < 4; i++) {
        if (mContainers[i]) {
            mValidDevices[i] = TheMC.IsDeviceValid(mContainerIDs[i]);
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
            if (mContainers[i] && !mContainers[i]->IsMounted()) {
                TheMC.DestroyContainer(mContainers[i]);
                mContainers[i] = nullptr;
            }
            mValidDevices[i] = false;
        }
    }
    return 0;
}
