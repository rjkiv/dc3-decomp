#include "os/FileCache.h"
#include "macros.h"
#include "obj/DirLoader.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "stl/_algo.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include <list>

std::list<FileCache *> gCaches;

FileCacheEntry::FileCacheEntry(FilePath const &path1, FilePath const &path2, int i)
    : unk0(path1), unk8(path2), mBuf(0), mLoader(0), mSize(-1), mRefCount(0), unk20(i),
      unk24(0), unk28(-1.0e30f) {}

FileCacheEntry::FileCacheEntry(FilePath const &path, char *c, int i)
    : unk0(path), unk8(path), mBuf(c), mLoader(0), mSize(i), mRefCount(0), unk20(-1),
      unk24(0), unk28(-1.0e30f) {}

FileCacheEntry::~FileCacheEntry() {
    MILO_ASSERT(mRefCount == 0, 0x83);
    delete mLoader;
    MemFree((void *)mBuf);
}

bool FileCacheEntry::ReadDone(bool b) {
    if (!b)
        unk28 = SystemMs();

    if (mSize > -1)
        return true;

    if (!mLoader || !mLoader->IsLoaded())
        return false;
    else {
        mSize = mLoader->GetSize();
        mBuf = mLoader->GetBuffer(0);
        RELEASE(mLoader);
        return true;
    }
    return false;
}

void FileCacheEntry::StartRead(LoaderPos lp, bool b) {
    MILO_ASSERT(mLoader == NULL, 0x9b);
    MILO_ASSERT(!mBuf, 0x9c);
    MILO_ASSERT(mSize == -1, 0x9d);
    mLoader = new FileLoader(unk8, unk8.c_str(), lp, 0x20000, b, false, 0, 0);
}

File *FileCacheEntry::MakeFile() { return nullptr; }

bool FileCacheFile::ReadDone(int &) { return false; }

bool FileCacheFile::Eof() { return unk4->mSize <= unk10; }

int FileCacheFile::Size() { return unk4->mSize; }

bool FileCacheFile::Fail() { return !unk4->mSize && !unk4->mBuf; }

int FileCacheFile::Write(void const *, int) { return 1; }

int FileCacheFile::Read(void *, int) { return 1; }

bool FileCacheFile::ReadAsync(void *, int) { return false; }

int FileCacheFile::Seek(int i, int j) { return 1; }

FileCache::FileCache(int i, LoaderPos lp, bool b1, bool b2)
    : unk0(i), unk4(0), unk14(lp), unk18(b1), unk19(b2) {
    gCaches.push_back(this);
    unk8.reserve(0x200);
}

FileCache::~FileCache() {}

void FileCache::RegisterResourceCacheHelper(class FileCacheHelper *) {}

void FileCache::RegisterWavCacheHelper(class FileCacheHelper *) {}

bool FileCache::DoneCaching() {
    for (int i = 0; i < unk8.size(); i++) {
        if (!unk8[i]->ReadDone(true))
            return false;
    }
    return true;
}

File *FileCache::GetFileAll(const char *c) { return nullptr; }

bool FileCache::FileCached(const char *c) { return false; }

void FileCache::StartSet(int) {}

void FileCache::Clear() {}

void FileCache::PollUntilLoaded() {}

void FileCache::PollAll() {
    for (std::list<FileCache *>::iterator it = gCaches.begin(); it != gCaches.end();
         ++it) {
        (*it)->Poll();
    }
}

void FileCache::Add(FilePath const &, int, FilePath const &) {}

void FileCache::Add(FilePath const &, char *, int) {}

struct Priority {
    bool operator()(FileCacheEntry *e1, FileCacheEntry *e2) const {
        return e1->unk20 > e2->unk20;
    }
};

void FileCache::EndSet() {
    unk4 = false;
    std::sort(unk8.begin(), unk8.end(), Priority());
    Poll();
}

void FileCache::SetSize(int i) { unk0 = i; }

File *FileCache::GetFile(char const *) { return 0; }

int FileCache::CurSize() const {
    int size = 0;
    for (int i = 0; i < unk8.size(); i++) {
        if (unk8[i]->CheckSize())
            size += unk8[i]->mSize;
    }
    return size;
}

void FileCache::DumpOverSize(int) {}

void FileCache::Poll() {}
