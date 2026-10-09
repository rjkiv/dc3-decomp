#include "os/FileCache.h"
#include "macros.h"
#include "math/Utl.h"
#include "obj/DirLoader.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Utl.h"
#include "synth/Utl.h"
#include "utl/Cache.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include <list>

std::list<FileCache *> gCaches;
FileCacheHelper *FileCache::sResourceCacheHelper;
FileCacheHelper *FileCache::sWavCacheHelper;

class FileCacheEntry {
public:
    FileCacheEntry(FilePath const &, FilePath const &, int);
    FileCacheEntry(FilePath const &, char *, int);
    ~FileCacheEntry();

    bool ReadDone(bool);
    void StartRead(LoaderPos, bool);
    File *MakeFile();
    bool CheckSize() { return mSize > -1; }
    bool Fail() { return mSize == 0 && !mBuf; }
    void AddRef() {
        mRefCount++;
        mReads++;
    }
    void Release() { mRefCount--; }
    bool HasLoader() const { return mLoader; }
    // int Size() const { return mSize; }
    // int Priority() const { return mPriority; }
    // const char *Buf() const { return mBuf; }
    // const FilePath &FileName() const { return mFileName; }
    // FileLoader *Loader() const { return mLoader; }
    // int RefCount() const { return mRefCount; }
    // void SetPriority(int prio) { mPriority = prio; }
    // float LastRead() const { return mLastRead; }

    // it has to be here because the pool overload says "FileCache.cpp"
    POOL_OVERLOAD(FileCacheEntry, 0x5F);

    FilePath mFileName; // 0x0
    FilePath mReadFileName; // 0x8
    const char *mBuf; // 0x10
    FileLoader *mLoader; // 0x14
    int mSize; // 0x18
    int mRefCount; // 0x1c
    int mPriority; // 0x20
    int mReads; // 0x24
    float mLastRead; // 0x28
};

#pragma region FileCacheEntry

FileCacheEntry::FileCacheEntry(FilePath const &file, FilePath const &readFile, int priority)
    : mFileName(file), mReadFileName(readFile), mBuf(0), mLoader(0), mSize(-1),
      mRefCount(0), mPriority(priority), mReads(0), mLastRead(-kHugeFloat) {}

FileCacheEntry::FileCacheEntry(FilePath const &file, char *buffer, int size)
    : mFileName(file), mReadFileName(file), mBuf(buffer), mLoader(0), mSize(size), mRefCount(0),
      mPriority(-1), mReads(0), mLastRead(-kHugeFloat) {}

FileCacheEntry::~FileCacheEntry() {
    MILO_ASSERT(mRefCount == 0, 0x83);
    delete mLoader;
    MemFree((void *)mBuf);
}

bool FileCacheEntry::ReadDone(bool silent) {
    if (!silent)
        mLastRead = SystemMs();

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

void FileCacheEntry::StartRead(LoaderPos pos, bool temp) {
    MILO_ASSERT(mLoader == NULL, 0x9b);
    MILO_ASSERT(!mBuf, 0x9c);
    MILO_ASSERT(mSize == -1, 0x9d);
    mLoader =
        new FileLoader(mReadFileName, mReadFileName.c_str(), pos, 0x20000, temp, false, 0, 0);
}

File *FileCacheEntry::MakeFile() {
    if (ReadDone(false)) {
        if (!Fail()) {
            MILO_LOG("making file from cache file %s\n", mFileName);
            return new FileCacheFile(this);
        }
    }
    return nullptr;
}

#pragma endregion
#pragma region FileCacheFile

FileCacheFile::FileCacheFile(FileCacheEntry *entry)
    : mParent(entry), mBytesRead(0), mData(0), mPos(0) {
    mParent->AddRef();
}

FileCacheFile::~FileCacheFile() { mParent->Release(); }

bool FileCacheFile::ReadDone(int &oBytes) {
    if (!mParent->ReadDone(false)) {
        oBytes = 0;
        return false;
    } else {
        if (mParent->Fail())
            return false;
        else {
            void *buf = mData;
            if (buf) {
                mData = 0;
                Read(buf, mBytesRead);
            }
            oBytes = mBytesRead;
            return true;
        }
    }
}

int FileCacheFile::Read(void *iData, int iBytes) {
    MILO_ASSERT(!mData, 0xFE);
    mBytesRead = iBytes;
    if (mParent->Fail())
        return 0;
    else {
        int bytesRead = Min(iBytes, mParent->mSize - mPos);
        memcpy(iData, mParent->mBuf + mPos, bytesRead);
        mBytesRead = bytesRead;
        mPos += bytesRead;
        return bytesRead;
    }
}

bool FileCacheFile::ReadAsync(void *iData, int iBytes) {
    MILO_ASSERT(!mData, 0x110);
    if (mParent->ReadDone(false)) {
        if (mParent->Fail())
            return false;
        else {
            Read(iData, iBytes);
            return true;
        }
    } else {
        mBytesRead = iBytes;
        mData = iData;
        return true;
    }
}

int FileCacheFile::Seek(int iOffset, int iType) {
    int ret;
    switch (iType) {
    case 0:
        ret = iOffset;
        break;
    case 1:
        ret = Tell() + iOffset;
        break;
    case 2:
        ret = mParent->mSize + iOffset;
        break;
    default:
        return mPos;
    }
    mPos = Clamp(0, mParent->mSize, ret);
    return mPos;
}

bool FileCacheFile::Eof() { return mParent->mSize <= mPos; }
bool FileCacheFile::Fail() { return mParent->Fail(); }
int FileCacheFile::Size() { return mParent->mSize; }

#pragma endregion
#pragma region FileCache

FileCache::FileCache(int size, LoaderPos pos, bool temp, bool notifyOnDump)
    : mMaxSize(size), mTryClear(0), mLoaderPos(pos), mTemp(temp), mNotifyOnDump(notifyOnDump) {
    gCaches.push_back(this);
    mEntries.reserve(0x200);
}

FileCache::~FileCache() {
    for (int i = 0; i < mEntries.size(); i++) {
        delete mEntries[i];
    }
    gCaches.erase(std::remove(gCaches.begin(), gCaches.end(), this), gCaches.end());
}

void FileCache::Init() {}
void FileCache::Terminate() {}

void FileCache::RegisterResourceCacheHelper(FileCacheHelper *iHelper) {
    MILO_ASSERT(iHelper, 0x183);
    sResourceCacheHelper = iHelper;
}

void FileCache::RegisterWavCacheHelper(FileCacheHelper *iHelper) {
    MILO_ASSERT(iHelper, 0x18A);
    sWavCacheHelper = iHelper;
}

bool FileCache::DoneCaching() {
    for (int i = 0; i < mEntries.size(); i++) {
        if (!mEntries[i]->ReadDone(true))
            return false;
    }
    return true;
}

File *FileCache::GetFileAll(const char *file) {
    FOREACH (it, gCaches) {
        File *fp = (*it)->GetFile(file);// TODO: find the proper name for this var in the pdb
        if (fp)
            return fp;
    }
    return nullptr;
}

bool FileCache::FileCached(const char *file) {
    FilePath path(DirLoader::CachedPath(file, 0));
    File *fp = GetFile(path.c_str()); // TODO: find the proper name for this var in the pdb
    if (fp) {
        delete fp;
        return true;
    } else
        return false;
}

void FileCache::StartSet(int priority) {
    mTryClear = false;
    for (int i = 0; i < mEntries.size(); i++) {
        FileCacheEntry *curEntry = mEntries[i];
        if ((!curEntry->CheckSize() || curEntry->Fail()) && !curEntry->HasLoader()
            && curEntry->mRefCount == 0) {
            delete curEntry;
            mEntries.erase(mEntries.begin() + i);
            i--;
        } else {
            mEntries[i]->mPriority = priority;
        }
    }
}

void FileCache::Clear() {
    mTryClear = true;
    for (int i = 0; i < mEntries.size();) {
        FileCacheEntry *curEntry = mEntries[i];
        if (!curEntry->HasLoader() && curEntry->mRefCount == 0) {
            delete curEntry;
            mEntries.erase(mEntries.begin() + i);
        } else
            i++;
    }
}

void FileCache::PollAll() {
    FOREACH (it, gCaches) {
        (*it)->Poll();
    }
}

struct Priority {
    bool operator()(FileCacheEntry *e1, FileCacheEntry *e2) const {
        return e1->mPriority > e2->mPriority;
    }
};

void FileCache::EndSet() {
    mTryClear = false;
    std::sort(mEntries.begin(), mEntries.end(), Priority());
    Poll();
}

void FileCache::SetSize(int size) {
    mMaxSize = size;
    EndSet();
}

File *FileCache::GetFile(const char *p) {
    FilePathTracker tracker(".");
    FilePath file(p);
    for (int i = 0; i < mEntries.size(); i++) {
        FileCacheEntry *curEntry = mEntries[i];
        if (curEntry->mFileName == file) {
            return curEntry->MakeFile();
        }
    }
    return nullptr;
}

int FileCache::CurSize() const {
    int size = 0;
    for (int i = 0; i < mEntries.size(); i++) {
        if (mEntries[i]->CheckSize())
            size += mEntries[i]->mSize;
    }
    return size;
}

void FileCache::Poll() {
    int i3 = 1;
    for (int i = 0; i < mEntries.size(); i++) {
        FileCacheEntry *cur = mEntries[i];
        cur->ReadDone(true);
        if (cur->HasLoader()) {
            i3--;
        }
    }
    int size = mTryClear ? 0 : mMaxSize;
    DumpOverSize(size);
    for (int i = 0; i < mEntries.size(); i++) {
        if (i3 <= 0) {
            return;
        }
        FileCacheEntry *cur = mEntries[i];
        if (cur->mSize <= -1 && !cur->HasLoader()) {
            cur->StartRead(mLoaderPos, mTemp);
            i3--;
        }
    }
}

void FileCache::PollUntilLoaded() {
    int old = mMaxSize;
    mMaxSize = 0x40000000;
    bool b3 = false;
    for (int i = 0; i < mEntries.size(); i++) {
        if (mEntries[i]->mSize <= -1) {
            b3 = true;
            break;
        }
    }
    while (b3) {
        TheLoadMgr.Poll();
        Poll();
        b3 = false;
        for (int i = 0; i < mEntries.size(); i++) {
            if (mEntries[i]->mSize <= -1) {
                b3 = true;
                break;
            }
        }
    }
    mMaxSize = old;
    TheLoadMgr.Poll();
    Poll();
}

void FileCache::DumpOverSize(int maxSize) {
    int i2 = CurSize();
    while (i2 > maxSize) {
        int u9 = -1;
        int i8 = 0;
        float f1 = 0;
        for (int i = 0; i < mEntries.size(); i++) {
            FileCacheEntry *curEntry = mEntries[i];
            if (curEntry->CheckSize() && !curEntry->HasLoader() && !curEntry->mRefCount
                && (u9 == -1 || curEntry->mPriority < i8
                    || (curEntry->mPriority == i8 && curEntry->mLastRead < f1))) {
                i8 = curEntry->mPriority;
                f1 = curEntry->mLastRead;
                u9 = i;
            }
        }
        if (u9 == -1)
            break;
        FileCacheEntry *delEntry = mEntries[u9];
        if (mNotifyOnDump) {
            int eSize = delEntry->mSize;
            MILO_NOTIFY(
                "Forced to dump entry with size %i (max size %i)", eSize, mMaxSize
            );
        }
        i2 -= delEntry->mSize;
        delete delEntry;
        mEntries.erase(mEntries.begin() + u9);
    }
}

void FileCache::Add(const FilePath &p, char *buffer, int size) {
    mTryClear = false;
    FilePath file(DirLoader::CachedPath(p.c_str(), 0));
    for (int i = 0; i < mEntries.size(); i++) {
        if (file == mEntries[i]->mFileName) {
            return;
        }
    }
    MILO_ASSERT(GetFileAll(file.c_str()) == NULL, 0x23D);
    mEntries.push_back(new FileCacheEntry(file, buffer, size));
}

void FileCache::Add(const FilePath &p, int priority, const FilePath &rFile) {
    mTryClear = false;
    FilePath file;
    const char *ext = FileGetExt(p.c_str());
    if (streq(ext, "milo")) {
        file.SetRoot(DirLoader::CachedPath(p.c_str(), 0));
    } else if (streq(ext, "png") || streq(ext, "bmp")) {
        if (sResourceCacheHelper) {
            file.SetRoot(sResourceCacheHelper->CacheFile(p.c_str()));
        } else {
            file = p;
        }
    } else if (streq(ext, "wav")) {
        if (sWavCacheHelper) {
            file.SetRoot(sWavCacheHelper->CacheFile(p.c_str()));
        } else {
            file = p;
        }
    } else {
        file = p;
    }

    for (int i = 0; i < mEntries.size(); i++) {
        if (file == mEntries[i]->mFileName) {
            MaxEq(mEntries[i]->mPriority, priority);
            return;
        }
    }
    MILO_ASSERT(GetFileAll(file.c_str()) == NULL, 0x21A);
    FilePath fp30;
    if (rFile.empty())
        fp30 = file;
    else
        fp30.SetRoot(DirLoader::CachedPath(rFile.c_str(), 0));
    mEntries.push_back(new FileCacheEntry(file, fp30, priority));
}
