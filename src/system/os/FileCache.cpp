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

FileCacheEntry::FileCacheEntry(FilePath const &path1, FilePath const &path2, int i)
    : mFileName(path1), mReadFileName(path2), mBuf(0), mLoader(0), mSize(-1),
      mRefCount(0), mPriority(i), mReads(0), mLastRead(-kHugeFloat) {}

FileCacheEntry::FileCacheEntry(FilePath const &path, char *c, int i)
    : mFileName(path), mReadFileName(path), mBuf(c), mLoader(0), mSize(i), mRefCount(0),
      mPriority(-1), mReads(0), mLastRead(-kHugeFloat) {}

FileCacheEntry::~FileCacheEntry() {
    MILO_ASSERT(mRefCount == 0, 0x83);
    delete mLoader;
    MemFree((void *)mBuf);
}

bool FileCacheEntry::ReadDone(bool b) {
    if (!b)
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

void FileCacheEntry::StartRead(LoaderPos lp, bool b) {
    MILO_ASSERT(mLoader == NULL, 0x9b);
    MILO_ASSERT(!mBuf, 0x9c);
    MILO_ASSERT(mSize == -1, 0x9d);
    mLoader =
        new FileLoader(mReadFileName, mReadFileName.c_str(), lp, 0x20000, b, false, 0, 0);
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

bool FileCacheFile::ReadDone(int &iref) {
    if (!mParent->ReadDone(false)) {
        iref = 0;
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
            iref = mBytesRead;
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

bool FileCacheFile::ReadAsync(void *v, int i) {
    MILO_ASSERT(!mData, 0x110);
    if (mParent->ReadDone(false)) {
        if (mParent->Fail())
            return false;
        else {
            Read(v, i);
            return true;
        }
    } else {
        mBytesRead = i;
        mData = v;
        return true;
    }
}

int FileCacheFile::Seek(int i1, int i2) {
    int ret;
    switch (i2) {
    case 0:
        ret = i1;
        break;
    case 1:
        ret = Tell() + i1;
        break;
    case 2:
        ret = mParent->mSize + i1;
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

FileCache::FileCache(int size, LoaderPos lp, bool b1, bool b2)
    : mMaxSize(size), mTryClear(0), unk14(lp), unk18(b1), unk19(b2) {
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

File *FileCache::GetFileAll(const char *cc) {
    FOREACH (it, gCaches) {
        File *file = (*it)->GetFile(cc);
        if (file)
            return file;
    }
    return nullptr;
}

bool FileCache::FileCached(const char *cc) {
    FilePath path(DirLoader::CachedPath(cc, 0));
    File *file = GetFile(path.c_str());
    if (file) {
        delete file;
        return true;
    } else
        return false;
}

void FileCache::StartSet(int iii) {
    mTryClear = false;
    for (int i = 0; i < mEntries.size(); i++) {
        FileCacheEntry *curEntry = mEntries[i];
        if ((!curEntry->CheckSize() || curEntry->Fail()) && !curEntry->HasLoader()
            && curEntry->mRefCount == 0) {
            delete curEntry;
            mEntries.erase(mEntries.begin() + i);
            i--;
        } else {
            mEntries[i]->mPriority = iii;
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

void FileCache::SetSize(int i) {
    mMaxSize = i;
    EndSet();
}

File *FileCache::GetFile(const char *cc) {
    FilePathTracker tracker(".");
    FilePath file(cc);
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
            cur->StartRead(unk14, unk18);
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

void FileCache::DumpOverSize(int iii) {
    int i2 = CurSize();
    while (i2 > iii) {
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
        if (unk19) {
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

void FileCache::Add(const FilePath &fp, char *c, int iii) {
    mTryClear = false;
    FilePath file(DirLoader::CachedPath(fp.c_str(), 0));
    for (int i = 0; i < mEntries.size(); i++) {
        if (file == mEntries[i]->mFileName) {
            return;
        }
    }
    MILO_ASSERT(GetFileAll(file.c_str()) == NULL, 0x23D);
    mEntries.push_back(new FileCacheEntry(file, c, iii));
}

void FileCache::Add(const FilePath &fp1, int iii, const FilePath &fp2) {
    mTryClear = false;
    FilePath file;
    const char *ext = FileGetExt(fp1.c_str());
    if (streq(ext, "milo")) {
        file.SetRoot(DirLoader::CachedPath(fp1.c_str(), 0));
    } else if (streq(ext, "png") || streq(ext, "bmp")) {
        if (sResourceCacheHelper) {
            file.SetRoot(sResourceCacheHelper->CacheFile(fp1.c_str()));
        } else {
            file = fp1;
        }
    } else if (streq(ext, "wav")) {
        if (sWavCacheHelper) {
            file.SetRoot(sWavCacheHelper->CacheFile(fp1.c_str()));
        } else {
            file = fp1;
        }
    } else {
        file = fp1;
    }

    for (int i = 0; i < mEntries.size(); i++) {
        if (file == mEntries[i]->mFileName) {
            MaxEq(mEntries[i]->mPriority, iii);
            return;
        }
    }
    MILO_ASSERT(GetFileAll(file.c_str()) == NULL, 0x21A);
    FilePath fp30;
    if (fp2.empty())
        fp30 = file;
    else
        fp30.SetRoot(DirLoader::CachedPath(fp2.c_str(), 0));
    mEntries.push_back(new FileCacheEntry(file, fp30, iii));
}
