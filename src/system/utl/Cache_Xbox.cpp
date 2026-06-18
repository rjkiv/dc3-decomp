#include "utl/Cache_Xbox.h"
#include "utl/Cache.h"
#include "utl/CacheMgr.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/ThreadCall.h"
#include "utl/Cache.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/minwinbase.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>
#include "xdk/XAPILIB.h"

CacheIDXbox::CacheIDXbox() { memset(&mContentData, 0, sizeof(XCONTENT_DATA)); }

const char *CacheIDXbox::GetCachePath(const char *c) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCachePath - mStrCacheName is empty.\n");
    }
    if (!c) {
        return MakeString("%s:\\", mStrCacheName.c_str());
    } else {
        String s = c;
        s.ReplaceAll('/', '\\');
        if (s.length() != 0 && s[0] == '\\') {
            s.erase(0, 1);
        }
        return MakeString("%s:\\%s", mStrCacheName.c_str(), s.c_str());
    }
}

const char *CacheIDXbox::GetCacheSearchPath(const char *c) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCacheSearchPath() - mStrCacheName is empty.\n");
    }
    if (!c) {
        return MakeString("%s:\\*", mStrCacheName.c_str());
    } else {
        return GetCachePath(c);
    }
}

CacheXbox::CacheXbox(const CacheIDXbox &c)
    : mCacheID(c), mData(0), mSize(0), mCacheDirList(0), mCallbackObj(0) {}

bool CacheXbox::IsConnectedSync() {
    return XContentGetDeviceState(mCacheID.DeviceID(), nullptr) == ERROR_SUCCESS;
}

int CacheXbox::ThreadStart() {
    MILO_ASSERT(!IsDone(), 0x197);
    switch (mOpCur) {
    case kOpDirectory:
        return ThreadGetDir(mThreadStr, "");
    case kOpFileSize:
        return ThreadGetFileSize();
    case kOpRead:
        return ThreadRead();
    case kOpWrite:
        return ThreadWrite();
    case kOpDelete:
        return ThreadDelete();
    default:
        MILO_ASSERT(false, 0x1AB);
        return 0;
    }
}

void CacheXbox::ThreadDone(int res) {
    MILO_ASSERT(!IsDone(), 0x1B4);
    OpType old = mOpCur;
    switch (old) {
    case kOpDirectory:
        mLastResult = (CacheResult)res;
        mThreadStr = gNullStr;
        mCacheDirList = nullptr;
        mCallbackObj = nullptr;
        break;
    case kOpFileSize:
        mLastResult = (CacheResult)res;
        mThreadStr = gNullStr;
        mData = nullptr;
        mCallbackObj = nullptr;
        break;
    case kOpRead:
        mLastResult = (CacheResult)res;
        mThreadStr = gNullStr;
        mData = nullptr;
        mSize = 0;
        mCallbackObj = nullptr;
        break;
    case kOpWrite:
        mLastResult = (CacheResult)res;
        mThreadStr = gNullStr;
        mData = nullptr;
        mSize = 0;
        if (mCallbackObj) {
            static Message msg("cache_write_result", GetLastResult());
            msg[0] = GetLastResult();
            mCallbackObj->Handle(msg, true);
        }
        mCallbackObj = nullptr;
        break;
    case kOpDelete:
        mLastResult = (CacheResult)res;
        mThreadStr = gNullStr;
        mCallbackObj = nullptr;
        break;
    default:
        MILO_ASSERT(false, 0x1E3);
        break;
    }
    mOpCur = kOpNone;
}

bool CacheXbox::GetFileSizeAsync(const char *cc, unsigned int *ui, Hmx::Object *o) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!ui) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        mThreadStr = mCacheID.GetCachePath(cc);
        mData = ui;
        mLastResult = kCache_NoError;
        mOpCur = kOpFileSize;
        ThreadCall(this);
        return true;
    }
}

bool CacheXbox::ReadAsync(const char *cc, void *v, unsigned int ui, Hmx::Object *o) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (cc && v && ui != 0) {
        mThreadStr = mCacheID.GetCachePath(cc);
        mData = v;
        mSize = ui;
        mLastResult = kCache_NoError;
        mOpCur = kOpRead;
        ThreadCall(this);
        return true;
    } else {
        mLastResult = kCache_ErrorBadParam;
        return false;
    }
}

bool CacheXbox::DeleteAsync(const char *cc, Hmx::Object *o) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!cc) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        mThreadStr = mCacheID.GetCachePath(cc);
        mLastResult = kCache_NoError;
        mOpCur = kOpDelete;
        ThreadCall(this);
        return true;
    }
}

bool CacheXbox::GetFreeSpaceSync(u64 *u) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!u) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        const char *path = mCacheID.GetCachePath(nullptr);
        ULARGE_INTEGER freeBytes;
        if (GetDiskFreeSpaceExA(path, &freeBytes, nullptr, nullptr) == 0U) {
            DWORD err = GetLastError();
            if (err != 0x15 && err != 0x456 && err != 0x48F && err != 0x651
                && IsDeviceConnected(mCacheID.DeviceID())) {
                MILO_NOTIFY(
                    "CacheXbox::GetFreeSpaceSync(): Unhandled error %u returned from GetDiskFreeSpaceEx().\n",
                    err
                );
                mLastResult = kCache_ErrorUnknown;
                return false;
            } else {
                mLastResult = kCache_ErrorStorageDeviceMissing;
                return false;
            }
        } else {
            XDEVICE_DATA deviceData;
            DWORD err = XContentGetDeviceData(mCacheID.DeviceID(), &deviceData);
            if (err != ERROR_SUCCESS) {
                if (err != 5 && err != 0x15 && err != 0x456 && err != 0x48F
                    && err != 0x651 && IsDeviceConnected(mCacheID.DeviceID())) {
                    MILO_NOTIFY(
                        "CacheXbox::GetFreeSpaceSync(): Unhandled error returned from GetDiskFreeSpaceEx().\n"
                    );
                    mLastResult = kCache_ErrorUnknown;
                    return false;
                } else {
                    mLastResult = kCache_ErrorStorageDeviceMissing;
                    return false;
                }
            } else {
                *u = deviceData.ulDeviceFreeBytes;
                mLastResult = kCache_NoError;
                return true;
            }
        }
    }
    return false;
}

bool CacheXbox::GetDirectoryAsync(
    const char *cc, std::vector<CacheDirEntry> *entries, Hmx::Object *obj
) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!entries) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        MILO_ASSERT(mThreadStr.empty(), 0x108);
        mThreadStr = mCacheID.GetCacheSearchPath(cc);
        MILO_ASSERT(mCacheDirList == NULL, 0x10B);
        mCacheDirList = entries;
        mLastResult = kCache_NoError;
        mOpCur = kOpDirectory;
        ThreadCall(this);
        return true;
    }
}

bool CacheXbox::DeleteSync(const char *cc) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!cc) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        String path = mCacheID.GetCachePath(cc);
        bool res = DeleteFileA(path.c_str());
        if (res) {
            path.erase(path.find_last_of('\\'));
            res = DeleteParentDirs(path);
        }
        XContentFlush(mCacheID.Name(), nullptr);
        if (!res) {
            DWORD err = GetLastError();
            if (!IsDeviceConnected(mCacheID.DeviceID())) {
                mLastResult = kCache_ErrorStorageDeviceMissing;
            } else {
                MILO_NOTIFY(
                    "CacheXbox::DeleteSync() - Unhandled error from DeleteFile(): %d\n",
                    err
                );
                mLastResult = kCache_ErrorUnknown;
            }
            return false;
        } else {
            mLastResult = kCache_NoError;
            return true;
        }
    }
}

bool CacheXbox::WriteAsync(const char *cc, void *v, unsigned int ui, Hmx::Object *obj) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        if (obj) {
            static Message msg("cache_write_result", GetLastResult());
            msg[0] = GetLastResult();
            obj->Handle(msg, true);
        } else
            return false;
    } else if (cc && v && ui != 0) {
        mThreadStr = mCacheID.GetCachePath(cc);
        mData = v;
        mSize = ui;
        mCallbackObj = obj;
        mLastResult = kCache_NoError;
        mOpCur = kOpWrite;
        ThreadCall(this);
        return true;
    } else {
        mLastResult = kCache_ErrorBadParam;
        if (obj) {
            static Message msg("cache_write_result", GetLastResult());
            msg[0] = GetLastResult();
            obj->Handle(msg, true);
        } else
            return false;
    }
    return false;
}

int CacheXbox::ThreadGetFileSize() {
    HANDLE file = CreateFileA(mThreadStr.c_str(), 0, 1, nullptr, 3, 0x80, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (!IsDeviceConnected(mCacheID.DeviceID())) {
            return 8;
        } else if (err == 2) {
            return 6;
        } else {
            MILO_NOTIFY(
                "CacheXbox::GetFileSizeAsync() - Unhandled error from CreateFile(): %d\n",
                err
            );
            return -1;
        }
    } else {
        int ret = 0;
        DWORD fileSize = 0;
        DWORD res = GetFileSize(file, &fileSize);
        if (res != -1) {
            DWORD err = GetLastError();
            if (err != 0) {
                MILO_NOTIFY(
                    "CacheXbox::GetFileSizeAsync() - Unhandled error from GetFileSize(): %d\n",
                    err
                );
                ret = -1;
            }
        } else {
            int *data = (int *)mData;
            *data = res;
        }
        CloseHandle(file);
        return !IsDeviceConnected(mCacheID.DeviceID()) ? 8 : ret;
    }
}

// int CacheXbox::ThreadRead() { return 1; }

// int CacheXbox::ThreadWrite() { return 1; }

// bool CacheXbox::DeleteParentDirs(String) { return 1; }

int CacheXbox::ThreadDelete() {
    mThreadStr.ReplaceAll('/', '\\');
    bool delFile = DeleteFileA(mThreadStr.c_str());
    if (delFile) {
        delFile = DeleteParentDirs(mThreadStr.erase(mThreadStr.find_last_of('\\')));
    }
    if (!delFile) {
        DWORD lastError = GetLastError();
        if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
            return 8;
        }
        MILO_NOTIFY(
            "CacheXbox::DeleteAsync() - Unhandled error from DeleteFile(): %d\n",
            lastError
        );
        return -1;
    }
    return 0;
}

// int CacheXbox::ThreadGetDir(String, String) { return 1; }
