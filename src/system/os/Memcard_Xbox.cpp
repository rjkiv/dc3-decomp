#include "os/Memcard_Xbox.h"
#include "Memcard.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "xdk/XAPILIB.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/minwinbase.h"
#include "xdk/xapilibi/timezoneapi.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"

MemcardXbox TheMC;

namespace {
    MCResult TranslateCommonWinErrorToMCResult(DWORD err) {
        switch (err) {
        case ERROR_NOT_READY:
        case ERROR_MEDIA_CHANGED:
        case ERROR_DEVICE_NOT_CONNECTED:
        case ERROR_DEVICE_REMOVED:
            return kMCNoCard;
        case ERROR_DISK_FULL:
            return kMCNotEnoughSpace;
        case ERROR_FILE_CORRUPT:
        case ERROR_DISK_CORRUPT:
            return kMCCorrupt;
        default:
            return kMCGeneralError;
        }
    }
}

void ContainerId::Set(int idx, DWORD dw) {
    mUserIndex = idx;
    unk8 = dw;
    mDeviceId = XCONTENTDEVICE_ANY;
}

#pragma region MCFileXbox

MCResult MCFileXbox::Open(const char *cc, AccessType at, CreateType ct) {
    MILO_ASSERT(mFile == INVALID_HANDLE_VALUE, 0x307);
    DWORD access = at == kAccessRead ? GENERIC_READ : GENERIC_WRITE;
    DWORD creationDisposition = 0;
    switch (ct) {
    case 0:
        creationDisposition = OPEN_EXISTING;
        break;
    case 1:
        creationDisposition = OPEN_ALWAYS;
        break;
    case 2:
        creationDisposition = CREATE_ALWAYS;
        break;
    default:
        break;
    }
    String path = mContainer->BuildPath(cc);
    mFile =
        CreateFileA(path.c_str(), access, 0, nullptr, creationDisposition, 0x80, nullptr);
    bool success = mFile != INVALID_HANDLE_VALUE;
    if (!success) {
        DWORD err = GetLastError();
        MCResult ret;
        if (err != ERROR_FILE_NOT_FOUND) {
            ret = TranslateCommonWinErrorToMCResult(err);
        } else {
            if (ct == 0) {
                return kMCFileNotFound;
            } else {
                return kMCGeneralError;
            }
        }
        return ret;
    } else {
        return kMCNoError;
    }
}

MCResult MCFileXbox::Read(void *data, int bytes) {
    MILO_ASSERT(mFile != INVALID_HANDLE_VALUE, 0x342);
    DWORD numRead = 0;
    if (ReadFile(mFile, data, bytes, &numRead, nullptr) == 0U) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else {
        return numRead == bytes ? kMCNoError : kMCCorrupt;
    }
}

MCResult MCFileXbox::Write(const void *data, int bytes) {
    MILO_ASSERT(mFile != INVALID_HANDLE_VALUE, 0x35B);
    DWORD numRead = 0;
    if (WriteFile(mFile, data, bytes, &numRead, nullptr) == 0U) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else {
        return numRead == bytes ? kMCNoError : kMCGeneralError;
    }
}

MCResult MCFileXbox::Seek(int x, SeekType st) {
    MILO_ASSERT(mFile != INVALID_HANDLE_VALUE, 0x374);
    DWORD dwMoveMethod;
    switch (st) {
    case kSeekBegin:
        dwMoveMethod = FILE_BEGIN;
        break;
    case kSeekCur:
        dwMoveMethod = FILE_CURRENT;
        break;
    case kSeekEnd:
        dwMoveMethod = FILE_END;
        break;
    default:
        MILO_ASSERT(false, 0x387);
        break;
    }
    if (SetFilePointer(mFile, x, nullptr, dwMoveMethod) == -1) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else
        return kMCNoError;
}

MCResult MCFileXbox::Close() {
    MILO_ASSERT(mFile != INVALID_HANDLE_VALUE, 0x39A);
    if (CloseHandle(mFile) == 0U) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else {
        mFile = INVALID_HANDLE_VALUE;
        return kMCNoError;
    }
}

bool MCFileXbox::IsOpen() { return mFile != INVALID_HANDLE_VALUE; }

MCResult MCFileXbox::GetSize(int *iptr) {
    DWORD fileSize = 0;
    DWORD res = GetFileSize(mFile, &fileSize);
    if (res != -1) {
        return kMCNoError;
    } else {
        DWORD err = GetLastError();
        if (err != ERROR_SUCCESS) {
            return TranslateCommonWinErrorToMCResult(err);
        } else if (iptr) {
            if (fileSize == 0 && res < 0x80000000) {
                *iptr = res;
            } else {
                *iptr = 0x7FFFFFFF;
            }
        }
        return kMCNoError;
    }
}

#pragma endregion
#pragma region MCContainerXbox

MCContainerXbox::MCContainerXbox(const ContainerId &c) : MCContainer(c) {}

MCResult MCContainerXbox::Mount(CreateType ct) {
    MILO_ASSERT(!IsMounted(), 0x144);
    MILO_ASSERT(Cid().mDeviceId != XCONTENTDEVICE_ANY, 0x145);
    mDriveName = TheMC.GenerateDriveName(Cid().mDeviceId, Cid().mUserIndex);
    XCONTENT_DATA data;
    data.DeviceID = Cid().mDeviceId;
    data.dwContentType = 1;
    strncpy(data.szFileName, TheMC.FileName(), XCONTENT_MAX_FILENAME_LENGTH);
    wcsncpy(data.szDisplayName, TheMC.DisplayName(), XCONTENT_MAX_DISPLAYNAME_LENGTH);
    DWORD u5 = 0;
    switch (ct) {
    case 0:
        u5 = 3;
        break;
    case 1:
        u5 = 0x14;
        break;
    case 2:
        u5 = 0x12;
        break;
    }
    ULARGE_INTEGER u;
    u.u.HighPart = Cid().unk8;

    DWORD res = XContentCreateEx(
        Cid().mUserIndex, mDriveName.c_str(), &data, u5, nullptr, nullptr, 0, u, nullptr
    );
    if (res == ERROR_SUCCESS) {
        BOOL b = false;
        if (XContentGetCreator(Cid().mUserIndex, &data, &b, nullptr, nullptr) == 0) {
            if (!b) {
                XContentClose(mDriveName.c_str(), nullptr);
                res = ERROR_FILE_CORRUPT;
            } else {
                SetMounted(true);
                return kMCNoError;
            }
        }
    }
    switch (res) {
    case ERROR_PATH_NOT_FOUND:
        return kMCFileNotFound;
    case ERROR_ALREADY_EXISTS:
        return kMCCorrupt;
    default:
        return TranslateCommonWinErrorToMCResult(res);
    }
}

MCResult MCContainerXbox::Unmount() {
    MILO_ASSERT(IsMounted(), 0x1A8);
    XContentClose(mDriveName.c_str(), nullptr);
    SetMounted(false);
    return kMCNoError;
}

MCResult MCContainerXbox::GetPathFreeSpace(const char *cc, u64 *u) {
    MILO_ASSERT(IsMounted(), 0x1B4);
    ULARGE_INTEGER u1;
    ULARGE_INTEGER u2;
    ULARGE_INTEGER u3;
    u1.u.HighPart = 0;
    u2.u.HighPart = 0;
    u3.u.HighPart = 0;
    u1.u.LowPart = 0;
    u2.u.LowPart = 0;
    u3.u.LowPart = 0;
    String str = BuildPath(cc);
    bool success = GetDiskFreeSpaceExA(str.c_str(), &u1, &u2, &u3);
    if (success == 0U) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else if (u) {
        *u = u1.QuadPart;
    }
    return kMCNoError;
}

MCResult MCContainerXbox::GetDeviceFreeSpace(u64 *u) {
    XDEVICE_DATA data;
    DWORD res = XContentGetDeviceData(Cid().mDeviceId, &data);
    if (res != ERROR_SUCCESS) {
        return TranslateCommonWinErrorToMCResult(res);
    } else if (u) {
        *u = data.ulDeviceFreeBytes;
    }
    return kMCNoError;
}

MCResult MCContainerXbox::Delete(const char *cc) {
    MILO_ASSERT(IsMounted(), 0x1ED);
    String str = BuildPath(cc);
    if (!DeleteFileA(str.c_str())) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else
        return kMCNoError;
}

MCResult MCContainerXbox::RemoveDir(const char *cc) {
    MILO_ASSERT(IsMounted(), 0x201);
    String str = BuildPath(cc);
    if (!RemoveDirectoryA(str.c_str())) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else
        return kMCNoError;
}

MCResult MCContainerXbox::MakeDir(const char *cc) {
    MILO_ASSERT(IsMounted(), 0x215);
    String str = BuildPath(cc);
    if (!CreateDirectoryA(str.c_str(), nullptr)) {
        return TranslateCommonWinErrorToMCResult(GetLastError());
    } else
        return kMCNoError;
}

MCResult MCContainerXbox::GetSize(const char *cc, int *iptr) {
    MCFileXbox *file = new MCFileXbox(this);
    MCResult res = file->Open(cc, kAccessRead, (CreateType)0);
    if (res == kMCNoError) {
        res = file->GetSize(iptr);
        file->Close();
        DestroyMCFile(file);
        return res;
    }
    return res;
}

MCFile *MCContainerXbox::CreateMCFile() { return new MCFileXbox(this); }

String MCContainerXbox::BuildPath(const char *cc) {
    String out = mDriveName + ':';
    if (*cc != '\\' && *cc != '/') {
        out += '\\';
    }
    out += cc;
    for (int i = 0; i < out.length(); i++) {
        if (out[i] == '/') {
            out[i] = '\\';
        }
    }
    return out;
}

MCResult MCContainerXbox::PrintDir(const char *cc, bool b2) {
    String str(cc);
    str += "/*";
    str = BuildPath(str.c_str());
    WIN32_FIND_DATAA findData;
    HANDLE file = FindFirstFileA(str.c_str(), &findData);
    if (file == INVALID_HANDLE_VALUE) {
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            return kMCGeneralError;
        } else {
            MILO_LOG("%s:\n", cc);
            MILO_LOG("%-20s %10s %30s\n", "name", "bytes", "modified");
            MILO_LOG("------------------------------------------------------------\n");
            MILO_LOG("total: 0\n");
            return kMCNoError;
        }
    } else {
        int num = 0;
        MILO_LOG("%s:\n", cc);
        MILO_LOG("%-20s %10s %30s\n", "name", "bytes", "modified");
        MILO_LOG("------------------------------------------------------------\n");
        std::list<String> strings;
        do {
            String fileName(findData.cFileName);
            if (fileName == "." || fileName == "..")
                continue;
            MILO_ASSERT(findData.nFileSizeHigh == 0, 0x2A1);
            FILETIME localFileTime;
            FileTimeToLocalFileTime(&findData.ftLastWriteTime, &localFileTime);
            SYSTEMTIME systemTime;
            FileTimeToSystemTime(&localFileTime, &systemTime);
            DateTime dt;
            dt.mYear = systemTime.wYear + 148;
            dt.mMonth = systemTime.wMonth;
            dt.mDay = systemTime.wDay;
            dt.mHour = systemTime.wHour;
            dt.mMin = systemTime.wMinute;
            dt.mSec = systemTime.wSecond;
            String dateString;
            dt.ToString(dateString);
            MILO_LOG(
                "%-20s %10i %30s\n",
                fileName.c_str(),
                findData.nFileSizeLow,
                dateString.c_str()
            );
            if (b2 && findData.dwFileAttributes & 0x10) {
                strings.push_back(fileName);
            }
            num++;
        } while (FindNextFileA(file, &findData));
        DWORD err = GetLastError();
        MILO_LOG("total: %i\n", num);
        if (err != ERROR_NO_MORE_FILES) {
            return kMCGeneralError;
        } else if (b2) {
            FOREACH (it, strings) {
                String str1218;
                if (*cc != '\0') {
                    str1218 = cc;
                    str1218 += "/";
                }
                str1218 += *it;
                MCResult res = PrintDir(str1218.c_str(), true);
                if (res != kMCNoError) {
                    return res;
                }
            }
        }
        return kMCNoError;
    }
    return kMCGeneralError;
}

#pragma endregion
#pragma region MemcardXbox

void MemcardXbox::Poll() {
    Memcard::Poll();
    if (unk156 && mXOverlapped.InternalLow != 0x3E5) {
        unk156 = false;
        if (unk158) {
            if (unk15c == 0) {
                NoDeviceChosenMsg msg;
                unk158->Handle(msg, false);
                unk158 = nullptr;
            } else {
                DeviceChosenMsg msg(unk15c);
                unk158->Handle(msg, false);
                unk158 = nullptr;
            }
        }
    }
}

void MemcardXbox::SetContainerName(const char *name) {
    strncpy(mFileName, name, XCONTENT_MAX_FILENAME_LENGTH - 1);
    mFileName[XCONTENT_MAX_FILENAME_LENGTH - 1] = '\0';
}

void MemcardXbox::SetContainerDisplayName(const wchar_t *name) {
    wcsncpy(mDisplayName, name, XCONTENT_MAX_DISPLAYNAME_LENGTH - 1);
    mDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH - 1] = 0;
}

void MemcardXbox::ShowDeviceSelector(
    const ContainerId &c, Hmx::Object *o, int i3, bool b4
) {
    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
    unk158 = o;
    unk15c = 0;
    int i1 = 0;
    if (b4) {
        i1 = 0x200;
    }
    if (i3 == -1) {
        i3 = c.mUserIndex;
    }
    ULARGE_INTEGER u;
    u.QuadPart = 0;
    if (ThePlatformMgr.ShowDeviceSelectorUI(i3, 1, i1, u, &unk15c, &mXOverlapped)
        == 0x3E5) {
        unk156 = true;
    } else if (unk158) {
        NoDeviceChosenMsg msg;
        unk158->Handle(msg, false);
        unk158 = nullptr;
    }
}

bool MemcardXbox::IsDeviceValid(const ContainerId &cid) {
    return XContentGetDeviceState(cid.mDeviceId, nullptr) == ERROR_SUCCESS;
}

MCResult MemcardXbox::DeleteContainer(const ContainerId &cid) {
    XCONTENT_DATA data;
    data.DeviceID = cid.mDeviceId;
    data.dwContentType = 1;
    strncpy(data.szFileName, mFileName, XCONTENT_MAX_FILENAME_LENGTH);
    wcsncpy(data.szDisplayName, mDisplayName, XCONTENT_MAX_DISPLAYNAME_LENGTH);
    DWORD res = XContentDelete(cid.mUserIndex, &data, nullptr);
    if (res != ERROR_SUCCESS) {
        return TranslateCommonWinErrorToMCResult(res);
    } else {
        return kMCNoError;
    }
}

MCContainer *MemcardXbox::CreateContainer(const ContainerId &cid) {
    return new MCContainerXbox(cid);
}

String MemcardXbox::GenerateDriveName(DWORD deviceId, int i) {
    return MakeString("s%08xp%d", deviceId, i);
}

MCResult MemcardXbox::FindValidUnit(ContainerId *pCid) {
    MILO_ASSERT(pCid, 0xBE);
    bool i5;
    if (pCid->mDeviceId == 0) {
        i5 = true;
    } else {
        XDEVICE_DATA data;
        data.DeviceID = pCid->mDeviceId;
        i5 = XContentGetDeviceData(pCid->mDeviceId, &data) == ERROR_SUCCESS;
    }
    if (i5 == 0) {
        return kMCNoCard;
    } else {
        HANDLE h1c0;
        DWORD enumRes = XContentCreateEnumerator(
            pCid->mUserIndex, pCid->mDeviceId, 1, 0x1000, 1, nullptr, &h1c0
        );
        switch (XContentCreateEnumerator(
            pCid->mUserIndex, pCid->mDeviceId, 1, 0x1000, 1, nullptr, &h1c0
        )) {
        case 0: {
            int num = 0;
            DWORD dw1bc = 0;
            char buffer[64];
            while (XEnumerate(h1c0, &buffer, 0x134, &dw1bc, nullptr) == 0) {
                if (strncmp(buffer, TheMC.FileName(), 0x2A) == 0) {
                    num++;
                    pCid->mDeviceId = (XCONTENTDEVICEID)buffer[0];
                }
            }
            CloseHandle(h1c0);
            if (num == 0) {
                return kMCMultipleFilesFound;
            } else if (num == 1) {
                return kMCFileExists;
            } else {
                return kMCFileNotFound;
            }
        }
        case 0x12:
            return kMCFileNotFound;
        default:
            return kMCGeneralError;
        }
    }
}
