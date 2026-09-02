#pragma once
#include "os/AsyncFile.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"

class AsyncFileWin : public AsyncFile {
public:
    AsyncFileWin(const char *, int);
    virtual ~AsyncFileWin();

    MEM_OVERLOAD(AsyncFile, 0x17);

protected:
    virtual bool Truncate(int);
    virtual void _OpenAsync();
    virtual bool _OpenDone() { return true; }
    virtual void _WriteAsync(const void *, int);
    virtual bool _WriteDone();
    virtual void _SeekToTell();
    virtual void _ReadAsync(void *, int);
    virtual bool _ReadDone();
    virtual void _Close();

    int unk34; // 0x34
    HANDLE mFile; // 0x38
    int fildes; // 0x3c
    bool mReadInProgress; // 0x40
    bool mWriteInProgress; // 0x41
    OVERLAPPED mOverlapped; // 0x44
    bool unk58;
    void *unk5c;
    void *unk60;
    int unk64;
    int unk68;
};
