#include "os/AsyncFile_Win.h"
#include "File.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/TextStream.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/ioapiset.h"
#include "xdk/xapilibi/minwinbase.h"
#include "xdk/xapilibi/winerror.h"
#include <cstring>
#include <errno.h>
#include "os/ContentMgr.h"
#include "os/File.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"
#include <io.h>

void ReadError(const char *cc) {
    DWORD err = GetLastError();
    String str;
    if (FileIsLocal(cc) && TheContentMgr.Contains(cc, str)) {
        MILO_LOG("ReadError in package '%s', err = 0x%08x\n", str, err);
        TheContentMgr.OnReadFailure(
            err == ERROR_FILE_CORRUPT || err == ERROR_DISK_CORRUPT, str.c_str()
        );
    } else if (UsingCD()) {
        ThePlatformMgr.SetDiskError(kDiskError);
    }
}

AsyncFileWin::AsyncFileWin(const char *filename, int mode)
    : AsyncFile(filename, mode), mFile(INVALID_HANDLE_VALUE), fildes(-1),
      mReadInProgress(0), mWriteInProgress(0) {}

AsyncFileWin::~AsyncFileWin() { Terminate(); }

bool AsyncFileWin::Truncate(int distanceToMove) {
    SetFilePointer(mFile, distanceToMove, nullptr, 0);
    return SetEndOfFile(mFile);
}

void AsyncFileWin::_OpenAsync() {
    mSize = 0;
    if (gFakeFileErrors) {
        SetLastError(0x20000002);
        ReadError(mFilename.c_str());
        mFail = true;
        return;
    }
    mSectorBytes = 0x800;
    if (((mMode & 0x7fffe) << 0x20 | mMode & 0x40002) == 0) {
        int _FileHandle = _open(mFilename.c_str(), mMode & 0xfffffffd | 0x8000, 0x180);
        fildes = _FileHandle;
        mFail = _FileHandle < 0;
        if (mFail)
            return;
        mSize = _lseeki64(fildes, 0, 2);
        if (mMode & 8)
            return;
        _lseek(fildes, 0, 0);
        return;
    }
    DWORD dwDesiredAccess;
    DWORD dwCreationDisposition;
    if (mMode & 2) {
        dwDesiredAccess = 0x80000000;
        dwCreationDisposition = 3;
    } else {
        dwDesiredAccess = 0x40000000;
        if (mMode & 0x200) {
            dwCreationDisposition = 2;
        } else {
            dwCreationDisposition = mMode & 0x100 ? 4 : 3;
        }
    }
    mFile = CreateFileA(
        mFilename.c_str(),
        dwDesiredAccess,
        3,
        nullptr,
        dwCreationDisposition,
        0x60000000,
        nullptr
    );
    if (mFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err != 2 && err != 3 && err != 0x15) {
            ReadError(mFilename.c_str());
        }
        mFail = true;
        return;
    } else {
        mFail = false;
        mSize = GetFileSize(mFile, nullptr);
    }
}

bool AsyncFileWin::_WriteDone() {
    if (!mWriteInProgress) {
        return true;
    } else {
        if (mOverlapped.Internal != 0x103) {
            mWriteInProgress = false;
            DWORD bytes[4];
            if (GetOverlappedResult(mFile, &mOverlapped, bytes, false)) {
                return true;
            }
            mFail = true;
        }
        return false;
    }
}

void AsyncFileWin::_SeekToTell() {
    if (!(mMode & FILE_OPEN_READ)) {
        if (fildes >= 0) {
            if (_lseek(fildes, mTell, 0) < 0) {
                mFail = true;
            }
        } else {
            while (!_WriteDone())
                ;
        }
    } else {
        while (!_ReadDone())
            ;
    }
}

void AsyncFileWin::_Close() {
    if (mMode & FILE_OPEN_READ) {
        if (mFile == INVALID_HANDLE_VALUE)
            return;
        while (!_ReadDone())
            ;
    } else {
        if (fildes >= 0) {
            _close(fildes);
        }
        if (mFile == INVALID_HANDLE_VALUE)
            return;
        while (!_WriteDone())
            ;
    }
    CloseHandle(mFile);
    mFile = INVALID_HANDLE_VALUE;
}

void AsyncFileWin::_WriteAsync(const void *data, int count) {
    if (fildes >= 0) {
        int wrote = _write(fildes, data, count);
        if (wrote >= count)
            return;
        if (wrote == -1 && errno == ENOSPC) {
            MILO_NOTIFY("AsyncFileWin::_Write: out of disk space");
        }
        mFail = true;
        return;
    } else {
        MILO_ASSERT(!mWriteInProgress && !mReadInProgress, 229);
        MILO_ASSERT(count >= 0, 230);
        if (count != 0) {
            mWriteInProgress = true;
            memset(&mOverlapped, 0, sizeof(OVERLAPPED));
            bool aligned;
            aligned = ((int)data & 3) == 0 && Tell() % mSectorBytes == 0
                && count % mSectorBytes == 0;
            MILO_ASSERT(aligned, 245);
            mOverlapped.Offset = Tell();
            if (!WriteFile(mFile, data, count, 0, &mOverlapped)
                && GetLastError() != ERROR_IO_PENDING) {
                mFail = true;
            }
        }
    }
}

bool AsyncFileWin::_ReadDone() {
    if (gFakeFileErrors) {
        SetLastError(0x20000002);
        ReadError(mFilename.c_str());
        mReadInProgress = false;
        mFail = true;
        return false;
    } else if (mReadInProgress == false) {
        return true;
    } else if (mOverlapped.Internal == 0x103) {
        return false;
    } else {
        DWORD btrans;
        if (GetOverlappedResult(mFile, &mOverlapped, &btrans, false) == false) {
            ReadError(mFilename.c_str());
            mReadInProgress = false;
            mFail = true;
            return false;
        }
        if (unk58) {
            memcpy(unk5c, static_cast<char *>(unk60) + unk68, unk64);
            MemFree(unk60);
        }
        mReadInProgress = false;
        return true;
    }
}

void AsyncFileWin::_ReadAsync(void *v, int count) {
    MILO_ASSERT(!mReadInProgress && !mWriteInProgress, 0x139);
    MILO_ASSERT(count >= 0, 0x13a);

    if (gFakeFileErrors) {
        SetLastError(0x20000002);
        ReadError(mFilename.c_str());
        mFail = true;
    } else {
        if (count != 0) {
            bool aligned;
            mReadInProgress = true;
            memset(&mOverlapped, 0, sizeof(OVERLAPPED));
            unk5c = v;
            unk64 = count;
            aligned = ((int)v & 3) == 0 && Tell() % mSectorBytes == 0
                && unk64 % mSectorBytes == 0;

            int bytesToRead;
            unk58 = aligned;
            if (aligned) {
                mOverlapped.Offset = Tell();
                bytesToRead = unk64;
                unk60 = unk5c;
            } else {
                mOverlapped.Offset = (Tell() / mSectorBytes) * mSectorBytes;
                int i =
                    ((Tell() + unk64 + mSectorBytes - 1) / mSectorBytes) * mSectorBytes;
                bytesToRead = i - mOverlapped.Offset;
                MILO_ASSERT(bytesToRead%mSectorBytes == 0, 0x16a);
                unk60 =
                    _MemAllocTemp(bytesToRead, __FILE__, 0x16d, "AsyncFileTempBuf", 0);
                unk68 = Tell() - mOverlapped.Offset;
            }

            if (!ReadFile(mFile, unk60, bytesToRead, 0, &mOverlapped)
                && GetLastError() != ERROR_IO_PENDING) {
                ReadError(mFilename.c_str());
                mFail = true;
            }
        }
    }
}
