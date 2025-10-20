#pragma once

#include "os/CritSec.h"
#include "stl/_vector.h"
#include "utl/FileStream.h"
#include "utl/Str.h"
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

    int unk0;
    std::vector<File *> unk4;
    std::vector<File *> unkc;
    int unk14;
    int unk18;
    int unk1c;
    int unk20;
    bool unk24;
    int unk28;
    int unk2c;
    int unk30;
    int unk34;
    int mLockId; // 0x38
    int unk3c;
    CriticalSection *unk40;
    int unk44;
    int unk50;
    String unk54;
    String unk5c;
    bool unk64;

private:
    int HdrSize();
    FileStream *OpenHeader();
    void WriteHdr();
    void OpenFiles(int);
};

extern HDCache TheHDCache;
