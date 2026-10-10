#include "os/AsyncFile.h"
#include "HolmesClient.h"
#include "macros.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "os/Archive.h"
#include "os/AsyncFileHolmes_p.h"
#include "os/AsyncFile_Win.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

static int gBufferSize = 0x20000;

void PrintDiscFile(const char *file) {
    const char *gen = "gen/";
    const char *path = FileGetPath(file);
    const char *base = FileGetBase(file);
    const char *ext = FileGetExt(file);
    int miloCmp = strncmp(ext, "milo", 4);
    if (miloCmp == 0) {
        DataArray *found = SystemConfig()->FindArray("force_milo_inline", false);
        if (found) {
            for (int i = 1; i < found->Size(); i++) {
                if (FileMatch(file, found->Str(i)))
                    return;
            }
        }
    } else {
        if (strneq(ext, "wav", 3) || strneq(ext, "bmp", 3) || strneq(ext, "png", 3)) {
            if (!strstr(file, "_keep"))
                return;
        } else {
            if (strneq(ext, "dta", 3)) {
                ext = "dtb";
            } else
                gen = "";
        }
    }
    String fullPath(MakeString("%s/%s%s.%s", path, gen, base, ext));
    unsigned int last = fullPath.find_last_of('_');
    bool lastFound = last != FixedString::npos;
    if (lastFound) {
        Symbol plat = PlatformSymbol(TheLoadMgr.GetPlatform());
        lastFound = plat == fullPath.c_str() + last + 1;
    }
    fullPath = (lastFound) ? fullPath.substr(0, last) : fullPath;
    MILO_LOG("AsyncFile:   '%s'\n", fullPath);
    HolmesClientPrint(fullPath.c_str());
}

AsyncFile::AsyncFile(const char *iFilename, int iMode)
    : mMode(iMode), mFail(false), mFilename(iFilename), mTell(0), mOffset(0), mSize(0), mUCSize(0),
      mBuffer(0), mData(0), mBytesLeft(0), mBytesRead(0) {}

AsyncFile::~AsyncFile() {}

int AsyncFile::Read(void *iData, int iBytes) {
    ReadAsync(iData, iBytes);
    if (mFail)
        return 0;
    else
        while (!ReadDone(iBytes))
            ;
    return iBytes;
}

bool AsyncFile::ReadAsync(void *iData, int iBytes) {
    MILO_ASSERT(iBytes >= 0, 0x126);
    MILO_ASSERT(mMode & FILE_OPEN_READ, 0x128);
    if (mFail)
        return false;
    else {
        if (!mBuffer) {
            _ReadAsync(iData, iBytes);
        } else {
            if (mTell + iBytes > mSize) {
                iBytes = mSize - mTell;
            }
            MILO_ASSERT(iBytes >= 0, 0x139);
            mData = (char *)iData;
            mBytesLeft = iBytes;
            mBytesRead = 0;
            ReadDone(iBytes);
        }
        return mFail == 0;
    }
}

int AsyncFile::Write(const void *iData, int iBytes) {
    WriteAsync((void *)iData, iBytes);
    if (mFail)
        return 0;
    else
        while (!WriteDone(iBytes))
            ;
    return iBytes;
}

bool AsyncFile::WriteAsync(const void *iData, int iBytes) {
    MILO_ASSERT(mMode & FILE_OPEN_WRITE, 0x186);
    if (mFail)
        return false;
    else {
        if (!mBuffer) {
            _WriteAsync(iData, iBytes);
        } else {
            do {
                if (mOffset + iBytes > gBufferSize) {
                    int size = gBufferSize - mOffset;
                    memcpy(mBuffer + mOffset, iData, size);
                    mOffset = gBufferSize;
                    iData = (void *)((int)iData + size);
                    mTell += size;
                    Flush();
                    iBytes -= size;
                } else {
                    memcpy(mBuffer + mOffset, iData, iBytes);
                    mTell += iBytes;
                    mOffset += iBytes;
                    if (mSize < mTell)
                        mSize = mTell;
                    goto okthen;
                }

            } while (!mFail);
            return false;
        }
    okthen:
        return iBytes != 0;
    }
}

int AsyncFile::Seek(int iOffset, int iSeekType) {
    if (mFail)
        return mTell;
    else {
        if (mMode & FILE_OPEN_WRITE)
            Flush();
        else
            MILO_ASSERT(!mBytesLeft, 0x1CA);
        // stuff in between
        _SeekToTell();
        if (mBuffer && (mMode & FILE_OPEN_READ)) {
            mOffset = gBufferSize;
            FillBuffer();
        }
        return mTell;
    }
}

void AsyncFile::Flush() {
    if (!mFail && (mMode & FILE_OPEN_WRITE)) {
        _WriteAsync(mBuffer, mOffset);
        while (!_WriteDone())
            ;
        mOffset = 0;
    }
}

bool AsyncFile::Eof() { return mTell == mSize; }

bool AsyncFile::ReadDone(int &oBytes) {
    if (mFail) {
        oBytes = 0;
        return true;
    } else {
        if (mBuffer && mBytesLeft == 0) {
            oBytes = mBytesRead;
            return true;
        } else {
            if (!_ReadDone()) {
                oBytes = mBytesRead;
                return false;
            } else {
                if (!mBuffer)
                    return true;
                else {
                    if (mOffset + mBytesLeft > gBufferSize) {
                        int size = gBufferSize - mOffset;
                        memcpy(mData, mBuffer + mOffset, size);
                        mBytesRead += size;
                        mOffset = gBufferSize;
                        mTell += size;
                        mBytesLeft -= size;
                        mData += size;
                        FillBuffer();
                        oBytes = mBytesRead;
                        return false;
                    } else {
                        memcpy(mData, mBuffer + mOffset, mBytesLeft);
                        mOffset += mBytesLeft;
                        int ret = mBytesRead + mBytesLeft;
                        mTell += mBytesLeft;
                        mBytesLeft = 0;
                        mBytesRead = ret;
                        oBytes = ret;
                        return true;
                    }
                }
            }
        }
    }
}

bool AsyncFile::WriteDone(int &oBytes) {
    if (mBuffer)
        return true;
    else
        return _WriteDone();
}

void AsyncFile::FillBuffer() {
    if (!mFail && (mMode & FILE_OPEN_READ)) {
        if (mOffset != gBufferSize)
            _SeekToTell();
        int newsize = mSize - mTell;
        _ReadAsync(mBuffer, Min<unsigned int>(newsize, gBufferSize));
        mOffset = 0;
    }
}

void AsyncFile::Init() {
    if (!(mMode & 0x40000)) {
        mBuffer = (char *)_MemAllocTemp(gBufferSize, __FILE__, 0xBE, "AsyncFileBuf", 0);
    }
    MILO_ASSERT((mMode & (FILE_OPEN_READ | FILE_OPEN_WRITE)) != (FILE_OPEN_READ | FILE_OPEN_WRITE), 0xC2);
    if (mMode & FILE_OPEN_WRITE) {
        bool curCD = UsingCD();
        SetUsingCD(false);
        FileQualifiedFilename(mFilename, mFilename.c_str());
        SetUsingCD(curCD);
    } else {
        FileQualifiedFilename(mFilename, mFilename.c_str());
    }
    _OpenAsync();
    while (!_OpenDone())
        ;
    if (!mFail) {
        if (strcmp(FileGetExt(mFilename.c_str()), "z") == 0 && (mMode & FILE_OPEN_READ)
            && mSize >= 4) {
            mTell = mSize - 4;
            _SeekToTell();
            _ReadAsync(&mUCSize, 4);
            while (!_ReadDone())
                ;
            mTell = 0;
            _SeekToTell();
            EndianSwapEq(mUCSize);
            mSize -= 4;
            goto next;
        }
    }
    mUCSize = 0;
next:
    if (mMode & FILE_OPEN_READ && mBuffer) {
        mOffset = gBufferSize;
        FillBuffer();
    }
    if (mMode & 8) {
        Seek(0, 2);
    }
}

void AsyncFile::Terminate() {
    if (mMode & FILE_OPEN_WRITE) {
        Flush();
    }
    _Close();
    MemFree(mBuffer);
}

AsyncFile *AsyncFile::New(const char *filename, int mode) {
    if (Archive::DebugArkOrder()) {
        PrintDiscFile(filename);
    }
    if ((UsingHolmes(1) && (mode & 1U) != 0) && !FileIsLocal(filename)) {
        AsyncFileHolmes *syncFile = new AsyncFileHolmes(filename, mode);
        if (syncFile) {
            syncFile->Init();
            return syncFile;
        }
    } else if (!UsingCD() && !FileIsLocal(filename)) {
        char c[256];
        if (HolmesClientCacheFile(c, filename)) {
            filename = c;
        }
        AsyncFileHolmes *syncFile = new AsyncFileHolmes(filename, mode);
        if (syncFile) {
            syncFile->Init();
            return syncFile;
        }
    }
    AsyncFileWin *syncFile = new AsyncFileWin(filename, mode);
    syncFile->Init();
    return syncFile;
}
