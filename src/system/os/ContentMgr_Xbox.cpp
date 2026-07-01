#include "os/ContentMgr_Xbox.h"
#include "ContentMgr.h"
#include "ContentMgr_Xbox.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"

std::vector<String> gIgnoredContent;
XboxContentMgr gContentMgr;
const char *kContentRootFormat = "cnt%08x";

XboxContent::XboxContent(const XCONTENT_CROSS_TITLE_DATA &data, int i2, int i3, bool b4)
    : mOverlapped(0), mLicenseBits(0), mValidLicenseBits(0),
      mRoot(MakeString(kContentRootFormat, i2)), unk150(MakeString("%s:", mRoot.c_str())),
      mState(kUnmounted), mPadNum(i3), unk160(0), unk161(0), mLRM(0) {
    MILO_ASSERT(mRoot.size() < kContentRootMaxLength, 0x6F);
    MILO_ASSERT(mPadNum < kNumberOfBuffers, 0x70);
    mXData = data;
    char filename[XCONTENT_MAX_FILENAME_LENGTH + 1];
    memcpy(filename, mXData.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    filename[XCONTENT_MAX_FILENAME_LENGTH] = 0;
    mFilename = filename;
    mState = (State)(b4 != 0);
}

XboxContent::~XboxContent() {
    switch (mState) {
    case 0:
    case 1:
    case 3:
    case 5:
    case 6:
    case 8:
        break;
    case kMounting:
    case kMounted:
        XContentClose(mRoot.c_str(), nullptr);
        break;
    default:
        int state = mState;
        MILO_LOG("Unknown state: %d", state);
        break;
    }
}

ContentLocT XboxContent::Location() {
    XDEVICE_DATA deviceData;
    XContentGetDeviceData(mXData.DeviceID, &deviceData);
    if (deviceData.DeviceType == 2) {
        return kLocationRemovableMem;
    } else {
        if (deviceData.DeviceType != 1) {
            MILO_NOTIFY(
                "Unknown device type: %d - defaulting to HDD", deviceData.DeviceType
            );
        }
        return kLocationHDD;
    }
}

void XboxContent::Poll() {
    if (mState == 1) {
        MILO_LOG("Mounting content '%s'\n", FileName());
        int pad = mPadNum;
        if (pad == 4 || pad == 5) {
            pad = 0xFF;
        }
        mOverlapped = new XOVERLAPPED;
        memset(mOverlapped, 0, sizeof(XOVERLAPPED));
        ULARGE_INTEGER contentSize = { 0 };
        if (XContentCrossTitleCreate(
                pad,
                mRoot.c_str(),
                &mXData,
                3,
                nullptr,
                &mLicenseBits,
                0,
                contentSize,
                mOverlapped
            )
            != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
        } else {
            mState = kMounting;
        }
    }
    if (mState == 2 || mState == 3) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res == 0x3E4)
            return;
        if (res == 0) {
            mValidLicenseBits = true;
            mState = mState == kMounting ? kMounted : kUnmounted;
            if (unk160) {
                Delete();
            }
        } else {
            unsigned short err = XGetOverlappedExtendedError(mOverlapped);
            mState = kContentDeleting;
            unk161 = err == 0x570;
        }
        RELEASE(mOverlapped);
    }
    if (mState == 6) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res != 0x3E4) {
            RELEASE(mOverlapped);
            mState = res == 0 ? (State)7 : (State)8;
        }
    }
}

void XboxContent::Mount() {
    if (mState == kUnmounted) {
        mState = kNeedsMounting;
        static unsigned int count = 0;
        count++;
        mLRM = count;
    }
}

void XboxContent::Unmount() {
    if (mState == kMounted) {
        mOverlapped = new XOVERLAPPED;
        memset(mOverlapped, 0, sizeof(XOVERLAPPED));
        if (XContentClose(mRoot.c_str(), mOverlapped) != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
            return;
        }
        mState = kUnmounting;
    } else if (mState == kNeedsMounting || mState == kContentDeleting) {
        mState = kUnmounted;
    }
}

void XboxContent::Delete() {
    unk160 = true;
    if (mState == 4 || mState == 1) {
        Unmount();
    } else if (mState == 0) {
        mOverlapped = new XOVERLAPPED;
        memset(mOverlapped, 0, sizeof(XOVERLAPPED));
        if (XContentCrossTitleDelete(0xFF, &mXData, mOverlapped) != 0x3E5) {
            RELEASE(mOverlapped);
            mState = kContentDeleting;
        } else {
            mState = kNeedsBackup;
        }
    }
}

BEGIN_HANDLERS(XboxContentMgr)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_MESSAGE(StorageChangedMsg)
    HANDLE_MESSAGE(ContentInstalledMsg)
    HANDLE_SUPERCLASS(ContentMgr)
END_HANDLERS

void XboxContentMgr::Init() {
    unk934 = 0;
    unk938 = 0;
    for (int i = 0; i < kNumberOfBuffers; i++) {
        mOverlappeds[i] = nullptr;
    }
    ThePlatformMgr.AddSink(this);
    DataArray *cfg = SystemConfig("content_mgr");
    cfg->FindData("enumerate_save_game_exports", mEnumerateSaveGameExports);
    DataArray *ignored = cfg->FindArray("ignored_content");
    for (int i = 1; i < ignored->Size(); i++) {
        gIgnoredContent.push_back(ignored->Str(i));
    }
    ContentMgr::Init();
}

void XboxContentMgr::Terminate() { ThePlatformMgr.RemoveSink(this); }

void XboxContentMgr::StartRefresh() {
    bool b10 = mDirty || (unk70 && unk71);
    if (b10) {
        mDirty = false;
        unk70 = false;
        unk71 = false;
        if (mState == 2) {
            for (int i = 0; i < kNumberOfBuffers; i++) {
                if (mOverlappeds[i]) {
                    XCancelOverlapped(mOverlappeds[i]);
                    RELEASE(mOverlappeds[i]);
                    CloseHandle(unk74[i]);
                }
            }
        } else if (mState != 1 && mState != 0) {
            RELEASE(mLoader);
            mCallbackFiles.clear();
        }
        FOREACH (it, mContents) {
            if ((*it)->GetState() == 4) {
                NotifyUnmounted(*it);
            }
        }
        DeleteAll(mContents);
        unk938 = 0;
        mRootLoaded = 0;
        DataArray *cfg = SystemConfig("content_mgr", "roots");
        for (int i = 1; i < cfg->Size(); i++) {
            mContents.push_back(new RootContent(cfg->Str(i)));
            mRootLoaded++;
        }
        FOREACH (it, mExtraContents) {
            mContents.push_back(new RootContent(it->c_str()));
            mRootLoaded++;
        }
        mState = kDiscoveryMounting;
        FOREACH (it, mCallbacks) {
            (*it)->ContentStarted();
        }
        for (int i = 0; i < kNumberOfBuffers; i++) {
            if ((i >= 4 || ThePlatformMgr.IsSignedIn(i))
                && (i != 5 || mEnumerateSaveGameExports)) {
                DWORD res;
                if (i == 4) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 2, 0, 1, nullptr, &unk74[i]
                    );
                } else if (i == 5) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 1, 0, 1, nullptr, &unk74[i]
                    );
                } else if (i == 6) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 0x7000, 0, 1, nullptr, &unk74[i]
                    );
                } else {
                    res = XContentCreateCrossTitleEnumerator(
                        i, 0, 2, 0, 1, nullptr, &unk74[i]
                    );
                }
                if (res == 0) {
                    mOverlappeds[i] = new XOVERLAPPED;
                    memset(mOverlappeds[i], 0, sizeof(XOVERLAPPED));
                    if (XEnumerateCrossTitle(
                            unk74[i], &mXDatas[i], 0x138, nullptr, mOverlappeds[i]
                        )
                        != 0x3E5) {
                        RELEASE(mOverlappeds[i]);
                        CloseHandle(unk74[i]);
                    }
                }
            }
        }
    }
}

bool XboxContentMgr::IsMounted(Symbol name) {
    bool ret = false;
    FOREACH (it, mContents) {
        if (name == (*it)->FileName()) {
            ret = (*it)->GetState() == Content::kMounted;
            break;
        }
    }
    return ret;
}

bool XboxContentMgr::IsCorrupt(Symbol contentName, const char *&displayName) {
    bool ret = false;
    FOREACH (it, mContents) {
        if (contentName == (*it)->FileName()) {
            ret = (*it)->IsCorrupt();
            displayName = (*it)->DisplayName();
            break;
        }
    }
    return ret;
}

bool XboxContentMgr::DeleteContent(Symbol contentName) {
    bool notFound = true;
    FOREACH (it, mContents) {
        if (contentName == (*it)->FileName()) {
            notFound = false;
            (*it)->Delete();
            mState = kContentMgrState6;
            mDirty = true;
            break;
        }
    }
    if (notFound) {
        MILO_NOTIFY("\"%s\" not found to delete.", contentName.Str());
    }
    return notFound;
}

bool XboxContentMgr::IsDeleteDone(Symbol contentName) {
    bool ret = false;
    FOREACH (it, mContents) {
        Content *cur = *it;
        if (contentName == cur->FileName()) {
            Content::State state = cur->GetState();
            ret = state == 7 || state == 8;
            break;
        }
    }
    return ret;
}

bool XboxContentMgr::GetLicenseBits(Symbol contentName, unsigned long &licenseBits) {
    FOREACH (it, mContents) {
        Content *cur = *it;
        if (contentName == cur->FileName()) {
            licenseBits = cur->LicenseBits();
            return cur->HasValidLicenseBits();
        }
    }
    return false;
}

void XboxContentMgr::NotifyMounted(Content *c) {
    XboxContent *xc = dynamic_cast<XboxContent *>(c);
    MILO_ASSERT(xc, 0x2C1);
    FOREACH (it, mCallbacks) {
        (*it)->ContentMounted(xc->FileName().Str(), xc->Root());
    }
}

void XboxContentMgr::NotifyUnmounted(Content *c) {
    XboxContent *xc = dynamic_cast<XboxContent *>(c);
    MILO_ASSERT(xc, 0x2CB);
    FOREACH (it, mCallbacks) {
        (*it)->ContentUnmounted(xc->FileName().Str());
    }
}

void XboxContentMgr::NotifyDeleted(Content *c) {
    XboxContent *xc = dynamic_cast<XboxContent *>(c);
    MILO_ASSERT(xc, 0x2D5);
}

void XboxContentMgr::NotifyFailed(Content *c) {
    XboxContent *xc = dynamic_cast<XboxContent *>(c);
    MILO_ASSERT(xc, 0x2E0);
    if (!RefreshDone()) {
        unk71 = true;
    }
    FOREACH (it, mCallbacks) {
        (*it)->ContentFailed(xc->FileName().Str());
    }
}

DataNode XboxContentMgr::OnMsg(const SigninChangedMsg &msg) {
    for (int i = 0; i < 4; i++) {
        if ((msg.GetChangedMask() >> i) & 1) {
            if (ThePlatformMgr.IsSignedIn(i)) {
                unk70 = true;
            }
        }
    }
    return 0;
}

DataNode XboxContentMgr::OnMsg(const ConnectionStatusChangedMsg &msg) {
    if (msg.Connected()) {
        unk70 = true;
    }
    return 0;
}

DataNode XboxContentMgr::OnMsg(const StorageChangedMsg &msg) {
    mDirty = true;
    return 0;
}

DataNode XboxContentMgr::OnMsg(const ContentInstalledMsg &msg) {
    mDirty = true;
    return 0;
}
