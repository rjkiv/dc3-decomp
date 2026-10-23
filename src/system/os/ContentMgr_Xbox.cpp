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

std::vector<String> gIgnoredContent;
XboxContentMgr gContentMgr;
const char *kContentRootFormat = "cnt%08x";

XboxContent::XboxContent(
    const XCONTENT_CROSS_TITLE_DATA &data, int id, int padNum, bool discover
)
    : mOverlapped(0), mLicenseBits(0), mHasValidLicenseBits(0),
      mRoot(MakeString(kContentRootFormat, id)), mDrive(MakeString("%s:", mRoot.c_str())),
      mState(kUnmounted), mPadNum(padNum), mDelete(0), mCorrupt(0), mLRM(0) {
    MILO_ASSERT(mRoot.size() < kContentRootMaxLength, 0x6F);
    MILO_ASSERT(mPadNum < kNumberOfBuffers, 0x70);
    memcpy(&mData, &data, sizeof(mData));
    char filename[XCONTENT_MAX_FILENAME_LENGTH + 1];
    memcpy(filename, mData.szFileName, XCONTENT_MAX_FILENAME_LENGTH);
    filename[XCONTENT_MAX_FILENAME_LENGTH] = 0;
    mFilename = filename;
    mState = (State)(discover != 0);
}

XboxContent::~XboxContent() {
    switch (mState) {
    case kUnmounted:
    case kNeedsMounting:
    case kUnmounting:
    case kAlwaysMounted:
    case kContentDeleting:
    case kFailed:
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
    XContentGetDeviceData(mData.DeviceID, &deviceData);
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
    if (mState == kNeedsMounting) {
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
                &mData,
                3,
                nullptr,
                &mLicenseBits,
                0,
                contentSize,
                mOverlapped
            )
            != ERROR_IO_PENDING) {
            RELEASE(mOverlapped);
            mState = kFailed;
        } else {
            mState = kMounting;
        }
    }
    if (mState == kMounting || mState == kUnmounting) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res == ERROR_IO_INCOMPLETE)
            return;
        if (res == ERROR_SUCCESS) {
            mHasValidLicenseBits = true;
            mState = mState == kMounting ? kMounted : kUnmounted;
            if (mDelete) {
                Delete();
            }
        } else {
            unsigned short err = XGetOverlappedExtendedError(mOverlapped);
            mState = kFailed;
            mCorrupt = err == ERROR_FILE_CORRUPT;
        }
        RELEASE(mOverlapped);
    }
    if (mState == kContentDeleting) {
        DWORD res = XGetOverlappedResult(mOverlapped, nullptr, false);
        if (res != ERROR_IO_INCOMPLETE) {
            RELEASE(mOverlapped);
            mState = res == ERROR_SUCCESS ? kDeleted : kFailed;
        }
    }
}

void XboxContent::Mount() {
    if (mState == kUnmounted) {
        mState = kNeedsMounting;
        static unsigned int lrmCounter = 0;
        lrmCounter++;
        mLRM = lrmCounter;
    }
}

void XboxContent::Unmount() {
    if (mState == kMounted) {
        mOverlapped = new XOVERLAPPED;
        memset(mOverlapped, 0, sizeof(XOVERLAPPED));
        if (XContentClose(mRoot.c_str(), mOverlapped) != ERROR_IO_PENDING) {
            RELEASE(mOverlapped);
            mState = kFailed;
            return;
        }
        mState = kUnmounting;
    } else if (mState == kNeedsMounting || mState == kFailed) {
        mState = kUnmounted;
    }
}

void XboxContent::Delete() {
    mDelete = true;
    if (mState == kMounted || mState == kNeedsMounting) {
        Unmount();
    } else if (mState == kUnmounted) {
        mOverlapped = new XOVERLAPPED;
        memset(mOverlapped, 0, sizeof(XOVERLAPPED));
        if (XContentCrossTitleDelete(0xFF, &mData, mOverlapped) != ERROR_IO_PENDING) {
            RELEASE(mOverlapped);
            mState = kFailed;
        } else {
            mState = kContentDeleting;
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
    mId = 0;
    mContentsToDiscover = 0;
    for (int i = 0; i < kNumberOfBuffers; i++) {
        mOverlapped[i] = nullptr;
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
    bool b10 = mDirty || (mPermissionsIncreased && mNeedsPermissions);
    if (b10) {
        mDirty = false;
        mPermissionsIncreased = false;
        mNeedsPermissions = false;
        if (mState == kDiscoveryEnumerating) {
            for (int i = 0; i < kNumberOfBuffers; i++) {
                if (mOverlapped[i]) {
                    XCancelOverlapped(mOverlapped[i]);
                    RELEASE(mOverlapped[i]);
                    CloseHandle(mContentEnum[i]);
                }
            }
        } else if (mState != kDone && mState != kInit) {
            RELEASE(mLoader);
            mCallbackFiles.clear();
        }
        FOREACH (it, mContents) {
            if ((*it)->GetState() == Content::kMounted) {
                NotifyUnmounted(*it);
            }
        }
        DeleteAll(mContents);
        mContentsToDiscover = 0;
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
        mState = kDiscoveryEnumerating;
        FOREACH (it, mCallbacks) {
            (*it)->ContentStarted();
        }
        for (int i = 0; i < kNumberOfBuffers; i++) {
            if ((i >= 4 || ThePlatformMgr.IsSignedIn(i))
                && (i != 5 || mEnumerateSaveGameExports)) {
                DWORD res;
                if (i == 4) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 2, 0, 1, nullptr, &mContentEnum[i]
                    );
                } else if (i == 5) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 1, 0, 1, nullptr, &mContentEnum[i]
                    );
                } else if (i == 6) {
                    res = XContentCreateCrossTitleEnumerator(
                        0xFF, 0, 0x7000, 0, 1, nullptr, &mContentEnum[i]
                    );
                } else {
                    res = XContentCreateCrossTitleEnumerator(
                        i, 0, 2, 0, 1, nullptr, &mContentEnum[i]
                    );
                }
                if (res == 0) {
                    mOverlapped[i] = new XOVERLAPPED;
                    memset(mOverlapped[i], 0, sizeof(XOVERLAPPED));
                    if (XEnumerateCrossTitle(
                            mContentEnum[i], &mBuffer[i], 0x138, nullptr, mOverlapped[i]
                        )
                        != ERROR_IO_PENDING) {
                        RELEASE(mOverlapped[i]);
                        CloseHandle(mContentEnum[i]);
                    }
                }
            }
        }
    }
}

void XboxContentMgr::PollRefresh() {
    if (mState == kDiscoveryEnumerating) {
        mState = kDiscoveryMounting;
        for (int i = 0; i < kNumberOfBuffers; i++) {
            if (mOverlapped[i]) {
                DWORD dwResult;
                DWORD res = XGetOverlappedResult(mOverlapped[i], &dwResult, false);
                if (res != ERROR_IO_INCOMPLETE) {
                    if (res == ERROR_SUCCESS) {
                        for (int j = 0; j < dwResult; j++) {
                            auto &curXData = mBuffer[i + j];
                            if (std::find(
                                    gIgnoredContent.begin(),
                                    gIgnoredContent.end(),
                                    curXData.szFileName
                                )
                                == gIgnoredContent.end()) {
                                bool b3 = false;
                                if (curXData.dwContentType == 0x7000) {
                                    FOREACH (it, mCallbacks) {
                                        bool cntRes =
                                            !(*it)->ContentTitleDiscovered(
                                                curXData.dwTitleId, curXData.szFileName
                                            )
                                            || b3;
                                        b3 = cntRes;
                                    }
                                } else {
                                    FOREACH (it, mCallbacks) {
                                        bool cntRes =
                                            !(*it)->ContentDiscovered(curXData.szFileName)
                                            || b3;
                                        b3 = cntRes;
                                    }
                                }
                                if (b3) {
                                    mContentsToDiscover++;
                                }
                                mContents.push_back(
                                    new XboxContent(curXData, mId++, i, b3)
                                );
                            } else {
                                break;
                            }
                        }
                        memset(mOverlapped[i], 0, sizeof(XOVERLAPPED));
                        if (XEnumerateCrossTitle(
                                mContentEnum[i],
                                &mBuffer[i],
                                0x138,
                                nullptr,
                                mOverlapped[i]
                            )
                            == ERROR_IO_PENDING) {
                            mState = kDiscoveryEnumerating;
                            return;
                        }
                    } else {
                        DWORD err = XGetOverlappedExtendedError(mOverlapped[i]);
                        if ((WORD)err != ERROR_NO_MORE_FILES) {
                            MILO_NOTIFY("XEnumerateCrossTitle (%d) error: %d", i, err);
                        }
                    }
                    RELEASE(mOverlapped[i]);
                    CloseHandle(mContentEnum[i]);
                } else {
                    mState = kDiscoveryEnumerating;
                    return;
                }
            }
        }
        FOREACH (it, mCallbacks) {
            (*it)->ContentMountBegun(mContentsToDiscover);
        }
    } else if (mState == kDiscoveryMounting) {
        int count = 0;
        FOREACH (it, mContents) {
            if ((*it)->GetState() == Content::kMounted) {
                count++;
            }
        }
        if (count >= 6) {
            mState = kDiscoveryLoading;
        }
    } else if (mState == kDiscoveryCheckIfDone) {
        bool b3 = true;
        FOREACH (it, mContents) {
            Content::State cState = (*it)->GetState();
            if (cState == Content::kMounted) {
                (*it)->Unmount();
                b3 = false;
            } else {
                b3 = (cState != Content::kNeedsMounting) ? b3 : false;
            }
        }
        if (!b3) {
            mState = kDiscoveryMounting;
        }
    }
    ContentMgr::PollRefresh();
}

bool XboxContentMgr::MountContent(Symbol contentName) {
    bool ret = false;
    bool found = false;
    FOREACH (it, mContents) {
        if (contentName == (*it)->FileName()) {
            found = true;
            (*it)->Mount();
            mState = kMounting;
            if ((*it)->GetState() == 4) {
                ret = true;
            }
            break;
        }
    }
    if (!found) {
        MILO_NOTIFY("\"%s\" not found to mount.", contentName.Str());
    }
    int i11 = 0;
    int i4 = 0;
    bool cond = false;
    while (!cond) {
        Content *cnt = nullptr;
        unsigned int i9 = -1;
        FOREACH (it, mContents) {
            Content::State cState = (*it)->GetState();
            if (cState == Content::kMounted || cState == Content::kMounting
                || cState == Content::kNeedsMounting) {
                i11++;
                if (contentName != (*it)->FileName() && (*it)->GetLRM() < i9
                    && (*it)->GetState() != Content::kMounting) {
                    cnt = *it;
                    i9 = cnt->GetLRM();
                }
            }
        }
        if (i11 == i4) {
            cond = true;
        } else if (i11 > 6 && cnt) {
            cnt->Unmount();
            mState = kMounting;
        }
        i4 = i11;
        i11 = 0;
    }
    return ret;
}

bool XboxContentMgr::IsMounted(Symbol contentName) {
    bool ret = false;
    FOREACH (it, mContents) {
        if (contentName == (*it)->FileName()) {
            ret = (*it)->GetState() == Content::kMounted;
            break;
        }
    }
    return ret;
}

bool XboxContentMgr::IsCorrupt(Symbol contentName, const char *&packageName) {
    bool ret = false;
    FOREACH (it, mContents) {
        if (contentName == (*it)->FileName()) {
            ret = (*it)->IsCorrupt();
            packageName = (*it)->DisplayName();
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
            mState = kDeleting;
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
            ret = state == Content::kDeleted || state == Content::kFailed;
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
        mNeedsPermissions = true;
    }
    FOREACH (it, mCallbacks) {
        (*it)->ContentFailed(xc->FileName().Str());
    }
}

DataNode XboxContentMgr::OnMsg(const SigninChangedMsg &msg) {
    for (int i = 0; i < 4; i++) {
        if (((unsigned int)msg.GetChangedMask() >> i) & 1) {
            if (ThePlatformMgr.IsSignedIn(i)) {
                mPermissionsIncreased = true;
            }
        }
    }
    return 0;
}

DataNode XboxContentMgr::OnMsg(const ConnectionStatusChangedMsg &msg) {
    if (msg.Connected()) {
        mPermissionsIncreased = true;
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
