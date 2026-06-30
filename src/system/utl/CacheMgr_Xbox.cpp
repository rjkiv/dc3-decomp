#include "Cache.h"
#include "CacheMgr.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/Cache.h"
#include "utl/CacheMgr.h"
#include "utl/CacheMgr_Xbox.h"
#include "utl/Cache_Xbox.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"

bool IsDeviceConnected(DWORD deviceID) {
    return XContentGetDeviceState(deviceID, nullptr) == 0;
}

CacheMgrXbox::CacheMgrXbox()
    : mFile(INVALID_HANDLE_VALUE), mppCacheID(nullptr), mppCache(0), mCallback(0) {
    memset(&mContentData, 0, sizeof(XCONTENT_DATA));
    mContentData.DeviceID = 0;
}

CacheMgrXbox::~CacheMgrXbox() {}

void CacheMgrXbox::Poll() {
    switch (GetOp()) {
    case kOpNone:
        break;
    case kOpSearch:
        PollSearch();
        break;
    case kOpChoose:
        PollChoose();
        break;
    case kOpMount:
        PollMount();
        break;
    case kOpUnmount:
        PollUnmount();
        break;
    case kOpDelete:
        PollDelete();
        break;
    default:
        MILO_FAIL("Unknown OpType encountered in CacheMgr::Poll()\n");
        break;
    }
}

bool CacheMgrXbox::SearchAsync(const char *cc, CacheID **ppCacheID) {
    if (!IsDone()) {
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else if (ppCacheID && !*ppCacheID) {
        mStrCacheName = cc;
        if (mStrCacheName.empty()) {
            MILO_LOG("SearchAsync BAD PARAM: mStrCacheName is empty\n");
            SetLastResult(kCache_ErrorBadParam);
            return false;
        } else {
            DWORD bufferSize = 0;
            DWORD res = XContentCreateEnumerator(0xFE, 0, 1, 0, 1, &bufferSize, &mFile);
            if (res == 0x12) {
                mFile = INVALID_HANDLE_VALUE;
                SetLastResult(kCache_ErrorNoStorageDevice);
                return false;
            } else {
                if (res == 0 && mFile != INVALID_HANDLE_VALUE) {
                    MILO_ASSERT(bufferSize == sizeof(XCONTENT_DATA), 0x88);
                    memset(&mContentData, 0, sizeof(XCONTENT_DATA));
                    mContentData.DeviceID = 0;
                    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
                    DWORD enumRes =
                        XEnumerate(mFile, &mContentData, 0x134, nullptr, &mOverlapped);
                    if (enumRes != ERROR_IO_PENDING) {
                        MILO_NOTIFY(
                            "CacheMgrXbox::SearchAsync(): Unhandled error %u returned from XEnumerate().\n",
                            enumRes
                        );
                        EndSearch(kCache_ErrorUnknown);
                        return false;
                    } else {
                        mppCacheID = ppCacheID;
                        SetOp(kOpSearch);
                        SetLastResult(kCache_NoError);
                        return true;
                    }
                } else {
                    MILO_NOTIFY(
                        "CacheMgrXbox::SearchAsync(): Unhandled error %u returned from XContentCreateEnumerator().\n",
                        res
                    );
                    mFile = INVALID_HANDLE_VALUE;
                    SetLastResult(kCache_ErrorUnknown);
                    return false;
                }
            }
        }
    } else {
        MILO_LOG("SearchAsync BAD PARAM: ppCacheID = 0x%X", ppCacheID);
        if (ppCacheID) {
            MILO_LOG(", *ppCacheID = 0x%X", *ppCacheID);
        }
        MILO_LOG("\n");
        SetLastResult(kCache_ErrorBadParam);
        return false;
    }
}

bool CacheMgrXbox::ShowUserSelectUIAsync(
    LocalUser *user, u64 u, const char *cc1, const char *cc2, CacheID **ppCacheID
) {
    if (!IsDone()) {
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else if (ppCacheID && !*ppCacheID) {
        mStrCacheName = cc1;
        mUTF8CacheDescription = cc2;
        if (!mStrCacheName.empty() && !mUTF8CacheDescription.empty()) {
            DWORD padnum = !user ? 0xFF : user->GetPadNum();
            memset(&mContentData, 0, sizeof(XCONTENT_DATA));
            mContentData.DeviceID = 0;
            memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
            ULARGE_INTEGER ul;
            ul.QuadPart = u;
            if (ul.QuadPart != 0) {
                ul.QuadPart = XContentCalculateSize(ul.QuadPart, 1);
            }
            DWORD res = ThePlatformMgr.ShowDeviceSelectorUI(
                padnum, 1, 0, ul, &mContentData.DeviceID, &mOverlapped
            );
            switch (res) {
            case ERROR_ACCESS_DENIED:
                SetLastResult(kCache_Error360GuideAlreadyOut);
                return false;
            case ERROR_IO_PENDING:
                mppCacheID = ppCacheID;
                SetLastResult(kCache_NoError);
                SetOp(kOpChoose);
                return true;
            default:
                MILO_NOTIFY(
                    "CacheMgrXbox::ShowUserSelectUIAsync(): Unhandled error %u returned from XShowDeviceSelectorUI().\n",
                    res
                );
                SetLastResult(kCache_ErrorUnknown);
                return false;
                break;
            }
        } else {
            SetLastResult(kCache_ErrorBadParam);
            return false;
        }
    } else {
        SetLastResult(kCache_ErrorBadParam);
        return false;
    }
}

bool CacheMgrXbox::CreateCacheIDFromDeviceID(
    unsigned int ui, const char *cc1, const char *cc2, CacheID **ppCacheID
) {
    if (!IsDone()) {
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else if (ppCacheID && !*ppCacheID) {
        mStrCacheName = cc1;
        mUTF8CacheDescription = cc2;
        if (!mStrCacheName.empty() && !mUTF8CacheDescription.empty()) {
            memset(&mContentData, 0, sizeof(XCONTENT_DATA));
            mContentData.DeviceID = ui;
            mppCacheID = ppCacheID;
            CreateCacheIDForChosenDevice();
            SetLastResult(kCache_NoError);
            SetOp(kOpNone);
            return true;
        } else {
            SetLastResult(kCache_ErrorBadParam);
            return false;
        }
    } else {
        SetLastResult(kCache_ErrorBadParam);
        return false;
    }
}

bool CacheMgrXbox::MountAsync(CacheID *pCacheIDXbox, Cache **ppCache, Hmx::Object *o) {
    if (!IsDone()) {
        MILO_NOTIFY("MountAsync: !IsDone() current op is %i", GetOp());
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else if (ppCache && !*ppCache) {
        CacheIDXbox *myCacheXbox = dynamic_cast<CacheIDXbox *>(pCacheIDXbox);
        if (!myCacheXbox) {
            MILO_NOTIFY("pCacheIDXbox == NULL");
            SetLastResult(kCache_ErrorBadParam);
            return false;
        } else {
            ULARGE_INTEGER u = { 0 };
            memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
            DWORD res = XContentCreateEx(
                0xFF,
                myCacheXbox->Name(),
                myCacheXbox->ContentData(),
                4,
                nullptr,
                nullptr,
                0,
                u,
                &mOverlapped
            );
            if (res != ERROR_IO_PENDING) {
                if (XContentGetDeviceState(myCacheXbox->ContentData()->DeviceID, nullptr)
                    != ERROR_SUCCESS) {
                    SetLastResult(kCache_ErrorStorageDeviceMissing);
                    return false;
                } else {
                    MILO_NOTIFY(
                        "CacheMgrXbox::MountAsync(): Unhandled error %u returned from XContentCreateEx().\n",
                        res
                    );
                    SetLastResult(kCache_ErrorUnknown);
                    return false;
                }
            } else {
                mCacheIDXbox = myCacheXbox;
                mppCache = ppCache;
                mCallback = o;
                SetLastResult(kCache_NoError);
                SetOp(kOpMount);
                return true;
            }
        }
    } else if (!ppCache) {
        MILO_NOTIFY("ppCache == NULL");
        SetLastResult(kCache_ErrorBadParam);
        return false;
    } else {
        MILO_NOTIFY("*ppCache != NULL");
        SetLastResult(kCache_ErrorBadParam);
        return false;
    }
}

bool CacheMgrXbox::UnmountAsync(Cache **ppCache, Hmx::Object *o) {
    if (!IsDone()) {
        MILO_NOTIFY("MountAsync: !IsDone() current op is %i", GetOp());
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else if (ppCache && *ppCache) {
        memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
        const char *name = (*ppCache)->GetCacheName();
        DWORD res = XContentClose(name, &mOverlapped);
        if (res != ERROR_IO_PENDING) {
            if (XContentGetDeviceState(mCacheIDXbox->ContentData()->DeviceID, nullptr)
                != 0) {
                MILO_NOTIFY("UnmountAsync: device is not connected");
                SetLastResult(kCache_ErrorStorageDeviceMissing);
                return false;
            } else {
                MILO_NOTIFY(
                    "CacheMgrXbox::UnmountAsync(): Unhandled error %u returned from XContentClose().\n",
                    res
                );
                SetLastResult(kCache_ErrorUnknown);
                return false;
            }
        } else {
            mppCache = ppCache;
            mCallback = o;
            SetLastResult(kCache_NoError);
            SetOp(kOpUnmount);
            return true;
        }
    } else {
        MILO_LOG("UnmountAsync: ppCache is NULL (MU pull?)");
        SetLastResult(kCache_ErrorBadParam);
        return false;
    }
}

bool CacheMgrXbox::DeleteAsync(CacheID *id) {
    if (!IsDone()) {
        SetLastResult(kCache_ErrorBusy);
        return false;
    } else {
        CacheIDXbox *cacheXbox = dynamic_cast<CacheIDXbox *>(id);
        if (!cacheXbox) {
            SetLastResult(kCache_ErrorBadParam);
            return false;
        } else {
            memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
            DWORD res = XContentDelete(0xFF, cacheXbox->ContentData(), &mOverlapped);
            if (res != ERROR_IO_PENDING) {
                if (XContentGetDeviceState(mCacheIDXbox->ContentData()->DeviceID, nullptr)
                    != ERROR_SUCCESS) {
                    SetLastResult(kCache_ErrorStorageDeviceMissing);
                    return false;
                } else {
                    MILO_NOTIFY(
                        "CacheMgrXbox::DeleteAsync(): Unhandled error %u returned from XContentClose().\n",
                        res
                    );
                    SetLastResult(kCache_ErrorUnknown);
                    return false;
                }
            } else {
                mCacheIDXbox = cacheXbox;
                SetLastResult(kCache_NoError);
                SetOp(kOpDelete);
                return true;
            }
        }
    }
}

void CacheMgrXbox::EndSearch(CacheResult res) {
    CloseHandle(mFile);
    mFile = INVALID_HANDLE_VALUE;
    mppCacheID = nullptr;
    mStrCacheName = gNullStr;
    SetLastResult(res);
    SetOp(kOpNone);
}

void CacheMgrXbox::CreateCacheIDForChosenDevice() {
    mContentData.dwContentType = 1;
    strncpy(mContentData.szFileName, mStrCacheName.c_str(), XCONTENT_MAX_FILENAME_LENGTH);
    MILO_ASSERT(UTF8StrLen(mUTF8CacheDescription.c_str()) < XCONTENT_MAX_DISPLAYNAME_LENGTH, 0x25B);
    UTF8toWChar_t(mContentData.szDisplayName, mUTF8CacheDescription.c_str());
    MILO_ASSERT(mppCacheID != NULL, 0x261);
    MILO_ASSERT(*mppCacheID == NULL, 0x262);
    CacheIDXbox *cacheXbox = new CacheIDXbox();
    *mppCacheID = cacheXbox;
    memcpy(cacheXbox->ContentData(), &mContentData, sizeof(XCONTENT_DATA));
    cacheXbox->SetName(mStrCacheName);
}

void CacheMgrXbox::PollChoose() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        if (mContentData.DeviceID == 0) {
            SetLastResult(kCache_ErrorUserCancel);
        } else {
            CreateCacheIDForChosenDevice();
            SetLastResult(kCache_NoError);
        }
        mppCacheID = nullptr;
        mStrCacheName = gNullStr;
        SetOp(kOpNone);
    }
}

void CacheMgrXbox::PollDelete() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD dw;
        DWORD res = XGetOverlappedResult(&mOverlapped, &dw, false);
        if (res != ERROR_SUCCESS) {
            if (res == ERROR_NOT_READY || res == ERROR_MEDIA_CHANGED
                || res == ERROR_DEVICE_NOT_CONNECTED || res == ERROR_DEVICE_REMOVED) {
                SetLastResult(kCache_ErrorStorageDeviceMissing);
            } else if (XContentGetDeviceState(
                           mCacheIDXbox->ContentData()->DeviceID, nullptr
                       )
                       != ERROR_SUCCESS) {
                SetLastResult(kCache_ErrorStorageDeviceMissing);
            } else {
                MILO_NOTIFY(
                    "CacheMgrXbox::PollDelete(): Unhandled error %u returned from XContentDelete().\n",
                    res
                );
                SetLastResult(kCache_ErrorUnknown);
            }
        } else {
            SetLastResult(kCache_NoError);
        }
        mCacheIDXbox = nullptr;
        SetOp(kOpNone);
    }
}

void CacheMgrXbox::PollSearch() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD numFound = 0;
        DWORD res = XGetOverlappedResult(&mOverlapped, &numFound, false);
        if (res != 0 && res != 0x65B) {
            MILO_FAIL("CacheMgrXbox::PollSearch() encountered unknown error %u.\n", res);
            EndSearch(kCache_ErrorCacheNotFound);
        } else if (numFound != 0) {
            MILO_ASSERT(numFound == 1, 0x1FB);
            mContentData.szFileName[0] &= 0x7F;
            int cmp = mStrCacheName.compare(
                0, strlen(mContentData.szFileName), mContentData.szFileName
            );
            if (cmp == 0) {
                MILO_ASSERT(mppCacheID != NULL, 0x20C);
                MILO_ASSERT(*mppCacheID == NULL, 0x20D);
                CacheIDXbox *cacheXbox = new CacheIDXbox();
                *mppCacheID = cacheXbox;
                memcpy(cacheXbox->ContentData(), &mContentData, sizeof(XCONTENT_DATA));
                cacheXbox->SetName(mStrCacheName);
                EndSearch(kCache_NoError);
            } else {
                memset(&mContentData, 0, sizeof(XCONTENT_DATA));
                mContentData.DeviceID = 0;
                memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
                DWORD enumRes =
                    XEnumerate(mFile, &mContentData, 0x134, nullptr, &mOverlapped);
                if (enumRes != ERROR_IO_PENDING) {
                    MILO_NOTIFY(
                        "CacheMgrXbox::PollSearch(): Unhandled error %u returned from XEnumerate().\n",
                        enumRes
                    );
                    EndSearch(kCache_ErrorUnknown);
                }
            }
        }
    }
}

void CacheMgrXbox::PollMount() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD err;
        DWORD res = XGetOverlappedResult(&mOverlapped, &err, false);
        if (res == 0) {
            MILO_ASSERT(mppCache != NULL, 0x293);
            MILO_ASSERT(*mppCache == NULL, 0x294);
            MILO_ASSERT(mCacheIDXbox, 0x295);
            CacheXbox *cacheXbox = new CacheXbox(*mCacheIDXbox);
            *mppCache = cacheXbox;
            SetLastResult(kCache_NoError);
        } else if (res == 0x65B) {
            DWORD extErr = XGetOverlappedExtendedError(&mOverlapped);
            if (XContentGetDeviceState(mContentData.DeviceID, nullptr)) {
                MILO_NOTIFY(
                    "CacheMgrXbox::PollMount(): error %u (0x%08X) occurred, but the device is no longer connected, so changing to %u.\n",
                    extErr,
                    extErr,
                    0x48F
                );
                extErr = 0x48F;
            }
            if (extErr != 0x15) {
                if (extErr == 0xB7) {
                    SetLastResult(kCache_ErrorCorrupt);
                } else {
                    if (extErr != 0x456) {
                        MILO_NOTIFY(
                            "CacheMgrXbox::PollMount(): Unhandled error %u %u %u returned from XContentCreateEx().\n",
                            res,
                            err,
                            extErr
                        );
                        SetLastResult(kCache_ErrorUnknown);
                    }
                }
            } else {
                SetLastResult(kCache_ErrorStorageDeviceMissing);
            }
        } else {
            MILO_NOTIFY(
                "CacheMgrXbox::PollMount(): Unhandled error %u %u returned from XContentCreateEx().\n",
                res,
                err
            );
            SetLastResult(kCache_ErrorUnknown);
        }
    }
    mCacheIDXbox = nullptr;
    mppCache = nullptr;
    SetOp(kOpNone);
    if (mCallback) {
        static Message msg("cache_mgr_mount_result", GetLastResult());
        msg[0] = GetLastResult();
        mCallback->Handle(msg, true);
        mCallback = nullptr;
    }
}

void CacheMgrXbox::PollUnmount() {
    if (mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD dw;
        DWORD res = XGetOverlappedResult(&mOverlapped, &dw, false);
        if (res != ERROR_SUCCESS) {
            if (res == ERROR_NOT_READY || res == ERROR_MEDIA_CHANGED
                || res == ERROR_DEVICE_NOT_CONNECTED || res == ERROR_DEVICE_REMOVED) {
                SetLastResult(kCache_ErrorStorageDeviceMissing);
            } else if (XContentGetDeviceState(
                           mCacheIDXbox->ContentData()->DeviceID, nullptr
                       )
                       != ERROR_SUCCESS) {
                SetLastResult(kCache_ErrorStorageDeviceMissing);
            } else {
                MILO_NOTIFY(
                    "CacheMgrXbox::PollUnmount(): Unhandled error returned from XContentClose().\n"
                );
                SetLastResult(kCache_ErrorUnknown);
            }
        } else {
            SetLastResult(kCache_NoError);
        }
        RELEASE(*mppCache);
        mppCache = nullptr;
        SetOp(kOpNone);
        if (mCallback) {
            static Message msg("cache_mgr_unmount_result", GetLastResult());
            msg[0] = GetLastResult();
            mCallback->Handle(msg, true);
            mCallback = nullptr;
        }
    }
}
