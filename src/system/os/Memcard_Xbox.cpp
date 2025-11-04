#include "os/Memcard_Xbox.h"
#include "Memcard.h"
#include "os/Debug.h"
#include "xdk/XAPILIB.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/handleapi.h"

MemcardXbox TheMC;

namespace {
    MCResult TranslateCommonWinErrorToMCResult(unsigned long);
}

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
    String path = unk4->BuildPath(cc);
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
    //       local_20[0] = 0;
    //   DVar1 = GetFileSize(*(this + 8),local_20);
    //   if ((DVar1 == 0xffffffff) && (DVar2 = GetLastError(), DVar2 != 0)) {
    //     MVar3 = _anon_AAE002CA::TranslateCommonWinErrorToMCResult(DVar2);
    //   }
    //   else {
    //     if (param_1 != 0x0) {
    //       if ((local_20[0] == 0) && (DVar1 < 0x80000000)) {
    //         *param_1 = DVar1;
    //       }
    //       else {
    //         *param_1 = 0x7fffffff;
    //       }
    //     }
    //     MVar3 = 0;
    //   }
    //   return MVar3;
}
