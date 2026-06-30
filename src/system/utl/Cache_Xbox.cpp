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
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>
#include "xdk/XAPILIB.h"

#pragma region CacheIDXbox

CacheIDXbox::CacheIDXbox() { memset(&mContentData, 0, sizeof(XCONTENT_DATA)); }

const char *CacheIDXbox::GetCachePath(const char *strPath) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCachePath - mStrCacheName is empty.\n");
    }
    if (!strPath) {
        return MakeString("%s:\\", mStrCacheName.c_str());
    } else {
        String s = strPath;
        s.ReplaceAll('/', '\\');
        if (s.length() != 0 && s[0] == '\\') {
            s.erase(0, 1);
        }
        return MakeString("%s:\\%s", mStrCacheName.c_str(), s.c_str());
    }
}

const char *CacheIDXbox::GetCacheSearchPath(const char *strFilter) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCacheSearchPath() - mStrCacheName is empty.\n");
    }
    if (!strFilter) {
        return MakeString("%s:\\*", mStrCacheName.c_str());
    } else {
        return GetCachePath(strFilter);
    }
}

#pragma endregion
#pragma region CacheXbox

CacheXbox::CacheXbox(const CacheIDXbox &c)
    : mCacheID(c), mData(0), mSize(0), mCacheDirList(0), mCallbackObj(0) {}

bool CacheXbox::IsConnectedSync() {
    return XContentGetDeviceState(mCacheID.ContentData()->DeviceID, nullptr)
        == ERROR_SUCCESS;
}

bool CacheXbox::GetFileSizeAsync(const char *filename, unsigned int *ui, Hmx::Object *o) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!ui) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        mThreadStr = mCacheID.GetCachePath(filename);
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
        ULARGE_INTEGER freeBytes = { 0 };
        if (GetDiskFreeSpaceExA(
                mCacheID.GetCachePath(nullptr), &freeBytes, nullptr, nullptr
            )
            == 0U) {
            DWORD err = GetLastError();
            if (err != ERROR_NOT_READY && err != ERROR_MEDIA_CHANGED
                && err != ERROR_DEVICE_NOT_CONNECTED && err != ERROR_DEVICE_REMOVED
                && IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
                MILO_NOTIFY(
                    "CacheXbox::GetFreeSpaceSync(): Unhandled error %u returned from GetDiskFreeSpaceEx().\n",
                    err
                );
                mLastResult = kCache_ErrorUnknown;
            } else {
                mLastResult = kCache_ErrorStorageDeviceMissing;
            }
            return false;
        } else {
            XDEVICE_DATA deviceData;
            DWORD err =
                XContentGetDeviceData(mCacheID.ContentData()->DeviceID, &deviceData);

            if (err != ERROR_SUCCESS) {
                if (err != ERROR_ACCESS_DENIED && err != ERROR_NOT_READY
                    && err != ERROR_MEDIA_CHANGED && err != ERROR_DEVICE_NOT_CONNECTED
                    && err != ERROR_DEVICE_REMOVED
                    && IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
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
                *u = deviceData.ulDeviceFreeBytes + freeBytes.QuadPart;
                mLastResult = kCache_NoError;
                return true;
            }
        }
    }
}

bool CacheXbox::GetDirectoryAsync(
    const char *filter, std::vector<CacheDirEntry> *pDirList, Hmx::Object *obj
) {
    if (!IsDone()) {
        mLastResult = kCache_ErrorBusy;
        return false;
    } else if (!pDirList) {
        mLastResult = kCache_ErrorBadParam;
        return false;
    } else {
        MILO_ASSERT(mThreadStr.empty(), 0x108);
        mThreadStr = mCacheID.GetCacheSearchPath(filter);
        MILO_ASSERT(mCacheDirList == NULL, 0x10B);
        mCacheDirList = pDirList;
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
            if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
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

int CacheXbox::ThreadGetFileSize() {
    HANDLE file = CreateFileA(mThreadStr.c_str(), 0, 1, nullptr, 3, 0x80, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
            return kCache_ErrorStorageDeviceMissing;
        } else if (err == ERROR_FILE_NOT_FOUND) {
            return kCache_ErrorCacheNotFound;
        } else {
            MILO_NOTIFY(
                "CacheXbox::GetFileSizeAsync() - Unhandled error from CreateFile(): %d\n",
                err
            );
            return kCache_ErrorUnknown;
        }
    } else {
        CacheResult ret = kCache_NoError;
        DWORD fileSize = 0;
        DWORD res = GetFileSize(file, &fileSize);
        DWORD err;
        if (res == -1 && (err = GetLastError(), err != ERROR_SUCCESS)) {
            MILO_NOTIFY(
                "CacheXbox::GetFileSizeAsync() - Unhandled error from GetFileSize(): %d\n",
                err
            );
            ret = kCache_ErrorUnknown;
        } else {
            *(DWORD *)mData = res;
        }
        CloseHandle(file);
        if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
            return kCache_ErrorStorageDeviceMissing;
        } else {
            return ret;
        }
    }
}

int CacheXbox::ThreadRead() {
    DWORD err;
    HANDLE file =
        CreateFileA(mThreadStr.c_str(), GENERIC_READ, 1, nullptr, 3, 0x80, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        if (err >= ERROR_FILE_NOT_FOUND
            && (err <= ERROR_PATH_NOT_FOUND || err == ERROR_NOT_READY)) {
            return kCache_ErrorStorageDeviceMissing;
        } else if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
            return kCache_ErrorStorageDeviceMissing;
        } else {
            MILO_NOTIFY(
                "CacheXbox::ReadAsync() - Unhandled error from CreateFile(): %d\n", err
            );
            return kCache_ErrorUnknown;
        }
    } else {
        DWORD numRead = 0;
        bool res = ReadFile(file, mData, mSize, &numRead, nullptr);
        CloseHandle(file);
        if (!res) {
            err = GetLastError();
            if (IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
                MILO_NOTIFY(
                    "CacheXbox::ReadAsync() - Unhandled error %d from ReadFile()\n", err
                );
                return kCache_ErrorUnknown;
            } else {
                return kCache_ErrorStorageDeviceMissing;
            }
        }
    }
    return kCache_NoError;
}

int CacheXbox::ThreadWrite() {
    mThreadStr.ReplaceAll('/', '\\');
    BOOL b3 = true;
    unsigned int idx = mThreadStr.find('\\');
    while ((idx = mThreadStr.find('\\', idx + 1), idx != FixedString::npos)) {
        String subStr = mThreadStr.substr(0, idx);
        if (GetFileAttributesA(subStr.c_str()) == -1) {
            b3 = CreateDirectoryA(subStr.c_str(), nullptr);
            if (!b3) {
                break;
            }
        }
    }

    DWORD err;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (b3) {
        hFile =
            CreateFileA(mThreadStr.c_str(), GENERIC_WRITE, 0, nullptr, 2, 0x80, nullptr);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        if (err >= ERROR_FILE_NOT_FOUND
            && (err <= ERROR_PATH_NOT_FOUND || err == ERROR_NOT_READY)) {
            return kCache_ErrorStorageDeviceMissing;
        } else if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
            return kCache_ErrorStorageDeviceMissing;
        } else {
            MILO_NOTIFY(
                "CacheXbox::WriteAsync() - Unhandled error from CreateFile(): %d\n", err
            );
            return kCache_ErrorUnknown;
        }
    } else {
        DWORD bytesWritten = 0;
        if (WriteFile(hFile, mData, mSize, &bytesWritten, nullptr)) {
            CloseHandle(hFile);
            XContentFlush(mCacheID.Name(), nullptr);
            return kCache_NoError;
        } else {
            err = GetLastError();
            CloseHandle(hFile);
            XContentFlush(mCacheID.Name(), nullptr);
            if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
                return kCache_ErrorStorageDeviceMissing;
            } else {
                MILO_NOTIFY(
                    "CacheXbox::ThreadWrite() - Unhandled error %d from WriteFile()\n",
                    err
                );
                return kCache_ErrorUnknown;
            }
        }
    }

    return -1;
}

bool CacheXbox::DeleteParentDirs(String str) {
    str.ReplaceAll('/', '\\');
    String path = mCacheID.GetCachePath("");
    if (path.length() >= str.length()) {
        return true;
    } else {
        if (!RemoveDirectoryA(str.c_str())) {
            if (GetLastError() == 0x91) {
                return true;
            } else {
                return false;
            }
        } else {
            str.erase(str.find_last_of('\\'));
            return DeleteParentDirs(str);
        }
    }
}

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

int CacheXbox::ThreadGetDir(String str1, String str2) {
    WIN32_FIND_DATAA findData;
    memset(&findData, 0, sizeof(WIN32_FIND_DATAA));
    HANDLE hFile = FindFirstFileA(str1.c_str(), &findData);
    CacheDirEntry entry;
    DWORD err;
    if (hFile == INVALID_HANDLE_VALUE) {
        err = GetLastError();
    } else {
        do {
            if (findData.nFileSizeHigh == 0) {
                if (findData.dwFileAttributes & 0x10) {
                    unsigned int strLen = str1.length();
                    unsigned int strPos = str1.find_last_of('\\');
                    String dir1 = MakeString(
                        "%s%s%s",
                        str1.substr(0, strPos + 1),
                        findData.cFileName,
                        str1.substr(strPos, strLen - strPos)
                    );
                    String dir2 = MakeString("%s%s/", str2, findData.cFileName);
                    int res = ThreadGetDir(dir1, dir2);
                    if (res != 0) {
                        CloseHandle(hFile);
                        return res;
                    }
                } else {
                    entry.mFileSize = findData.nFileSizeLow;
                    entry.mFullFileName = str2 + findData.cFileName;
                    entry.mLastWriteTime.FromFileTime(findData.ftLastWriteTime);
                    mCacheDirList->push_back(entry);
                }
            }
        } while (FindNextFileA(hFile, &findData));
        err = GetLastError();
        CloseHandle(hFile);
    }
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_NO_MORE_FILES) {
        return kCache_NoError;
    } else if (err == ERROR_NOT_READY || err == ERROR_MEDIA_CHANGED
               || err == ERROR_DEVICE_NOT_CONNECTED || err == ERROR_DEVICE_REMOVED) {
        return kCache_ErrorStorageDeviceMissing;
    } else if (!IsDeviceConnected(mCacheID.ContentData()->DeviceID)) {
        return kCache_ErrorStorageDeviceMissing;
    } else {
        return kCache_ErrorUnknown;
    }
}

#pragma endregion
