#pragma once
#include "obj/Object.h"
#include "os/File.h"
#include "stl/_vector.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"

class FileCacheEntry {
public:
    FileCacheEntry(FilePath const &, FilePath const &, int);
    FileCacheEntry(FilePath const &, char *, int);
    ~FileCacheEntry();

    bool ReadDone(bool);
    void StartRead(LoaderPos, bool);
    File *MakeFile();
    bool CheckSize() { return mSize > -1; }

    FilePath unk0;
    FilePath unk8;
    const char *mBuf; // 0x10
    FileLoader *mLoader; // 0x14
    int mSize; // 0x18
    int mRefCount; // 0x1c
    int unk20;
    int unk24;
    float unk28;
};

class FileCacheFile : public File {
public:
    // File
    virtual int Read(void *, int);
    virtual bool ReadAsync(void *, int);
    virtual int Write(void const *, int);
    virtual int Seek(int, int);
    virtual bool Eof();
    virtual bool Fail();
    virtual int Size();
    virtual bool ReadDone(int &);

    FileCacheEntry *unk4;
    int unk8;
    void *unkc;
    int unk10;
};

class FileCache {
public:
    FileCache(int, LoaderPos, bool, bool);
    ~FileCache();

    static void RegisterResourceCacheHelper(class FileCacheHelper *);
    static void RegisterWavCacheHelper(class FileCacheHelper *);
    static File *GetFileAll(char const *);
    bool DoneCaching();
    bool FileCached(char const *);
    void StartSet(int);
    void Clear();
    void PollUntilLoaded();
    static void PollAll();
    void Add(FilePath const &, int, FilePath const &);
    void Add(FilePath const &, char *, int);
    void EndSet();
    void SetSize(int);

    int unk0;
    bool unk4;
    std::vector<FileCacheEntry *> unk8;
    LoaderPos unk14;
    bool unk18;
    bool unk19;

protected:
    File *GetFile(char const *);
    int CurSize() const;
    void DumpOverSize(int);
    void Poll();
};
