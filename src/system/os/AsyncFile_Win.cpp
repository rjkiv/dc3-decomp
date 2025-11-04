#include "os/AsyncFile_Win.h"
#include "File.h"
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
        bool b3 = err == ERROR_FILE_CORRUPT || err == ERROR_DISK_CORRUPT;
        TheContentMgr.OnReadFailure(b3, str.c_str());

    } else if (UsingCD()) {
        ThePlatformMgr.SetDiskError(kDiskError);
    }
}

AsyncFileWin::AsyncFileWin(const char *filename, int mode)
    : AsyncFile(filename, mode), mFile(INVALID_HANDLE_VALUE), unk3c(-1),
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
    } else {
        unk34 = 0x800;
        if (((mMode & 0x7fffe) << 0x20 | mMode & 0x40002) == 0) {
            int _FileHandle =
                _open(mFilename.c_str(), mMode & 0xfffffffd | 0x8000, 0x180);
            unk3c = _FileHandle;
            mFail = !unk3c;
            if (_FileHandle < 0)
                return;
            mUCSize = _lseeki64(unk3c, 0, 2);
            if (mMode & 8)
                return;
            _lseek((int)mFile, 0, 0);
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
                dwCreationDisposition = mMode & 0x20;
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
        if (mFile == (HANDLE)-1) {
            DWORD err = GetLastError();
            if (err == 2 || err == 3 || err == 0x15) {
                mFail = true;
            }
        } else {
            mFail = false;
            mSize = GetFileSize(mFile, nullptr);
        }
        return;
    }
    ReadError(mFilename.c_str());
    mFail = true;
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
        if (unk3c >= 0) {
            if (_lseek(unk3c, mTell, 0) < 0) {
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
        if (unk3c >= 0) {
            _close(unk3c);
        }
        if (mFile == INVALID_HANDLE_VALUE)
            return;
        while (!_WriteDone())
            ;
    }
    CloseHandle(mFile);
    mFile = INVALID_HANDLE_VALUE;
}
