#pragma once
#include "os/CritSec.h"
#include "utl/FileStream.h"
#include "utl/MemStream.h"
#include "utl/Str.h"
#include "xdk/win_types.h"
#include <vector>

class HDCache {
public:
    HDCache();
    ~HDCache();
    bool LockCache();
    void UnlockCache();
    bool ReadFail();
    bool ReadDone();
    bool WriteDone();
    void Poll();
    bool ReadAsync(int, int, void *);
    bool WriteAsync(int, int, void const *);
    void Init();

private:
    int HdrSize();
    FileStream *OpenHeader();
    void WriteHdr();
    void OpenFiles(int);
    void Flush();
    File *ReadFile() { return mReadArkFiles[mReadFileIdx]; }
    File *WriteFile() { return mWriteArkFiles[mWriteFileIdx]; }

    int **mBlockState; // 0x0
    std::vector<File *> mReadArkFiles; // 0x4
    std::vector<File *> mWriteArkFiles; // 0x10
    int mWriteFileIdx; // 0x1c
    int mWriteBlock; // 0x20
    bool mWritingHeader; // 0x24
    int mReadFileIdx; // 0x28
    int mDirtyCache; // 0x2c
    int mLastHdrWriteMs; // 0x30
    int mLastCacheWriteMs; // 0x34
    DWORD mLockId; // 0x38
    int mLockCount; // 0x3c
    CriticalSection *mLockCrit; // 0x40
    int mHdrIdx; // 0x44
    File *mHdr[2]; // 0x48
    MemStream *mHdrBuf; // 0x50
    String mHdrFmt; // 0x54
    String mFileFmt; // 0x5c
    bool mUsingPriorData; // 0x64
};

extern HDCache TheHDCache;
