#include "os/Archive.h"
#include "math/Sort.h"
#include "os/Block.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/HxGuid.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Option.h"
#include <cstdio>

Archive *TheArchive;
bool gDebugArkOrder;

#pragma region ArkHash

ArkHash::~ArkHash() {
    MemFree(mHeap);
    MemFree(mTable);
}

int ArkHash::AddString(const char *str) {
    int hashIdx = HashString(str, mTableSize);
    int idx = hashIdx;
    MILO_ASSERT(hashIdx < mTableSize, 0xB4);
    while (mTable[hashIdx]) {
        if (streq(mTable[hashIdx], str))
            return hashIdx;
        hashIdx++;
        if (hashIdx == mTableSize) {
            hashIdx = 0;
        }
        if (hashIdx == idx) {
            MILO_FAIL("ERROR: Hash table full!!!");
        }
    }
    int len = strlen(str);
    MILO_ASSERT(mFree + len + 1 < mHeapEnd, 200);
    memcpy(mFree, str, len);
    MILO_ASSERT(hashIdx < mTableSize, 0xCA);
    mTable[hashIdx] = mFree;
    mFree += len + 1;
    return hashIdx;
}

int ArkHash::GetHashValue(const char *str) const {
    int hashIdx = HashString(str, mTableSize);
    MILO_ASSERT(hashIdx < mTableSize, 0xD4);
    while (mTable[hashIdx]) {
        if (streq(mTable[hashIdx], str))
            return hashIdx;
        if (++hashIdx == mTableSize)
            hashIdx = 0;
    }
    return -1;
}

const char *ArkHash::operator[](int idx) const {
    MILO_ASSERT(idx < mTableSize, 0xE7);
    return mTable[idx];
}

void ArkHash::Read(BinStream &s, int heap_headroom) {
    MemFree(mHeap);
    MemFree(mTable);
    int i40;
    s >> i40;
    int iSizeBytes = i40 + heap_headroom;
    mHeap = (char *)MemAlloc(iSizeBytes, __FILE__, 0x112, "ArkHash");
    mFree = mHeap + i40;
    mHeapEnd = mHeap + iSizeBytes;
    s.Read(mHeap, i40);
    memset(mFree, 0, mHeapEnd - mFree);
    s >> mTableSize;
    mTable = (char **)MemAlloc(mTableSize * 4, __FILE__, 0x11A, "ArkHash");
    char **end = mTable + mTableSize;
    for (char **p = mTable; p != end; p++) {
        int offset;
        s >> offset;
        if (offset != 0) {
            *p = mHeap + offset;
        } else {
            *p = nullptr;
        }
    }
}

#pragma endregion
#pragma region Archive

bool Archive::DebugArkOrder() { return gDebugArkOrder; }

Archive::~Archive() {}

Archive::Archive(const char *basename, int heap_headroom)
    : mNumArkfiles(0), mBasename(basename), mMode(kRead), mMaxArkfileSize(0),
      mIsPatched(false), mArkfilePerms(0), mArkfilePermsCount(0) {
    Read(heap_headroom);
}

bool Archive::HasArchivePermission(int arkfileNum) const {
    for (int i = 0; i < mArkfilePermsCount; i++) {
        if (mArkfilePerms[i] == arkfileNum)
            return true;
    }
    return false;
}

void Archive::SetArchivePermission(int count, const int *arkfiles) {
    mArkfilePermsCount = count;
    mArkfilePerms = arkfiles;
}

void Archive::GetGuid(HxGuid &guid) const { guid = mGuid; }

BinStream &operator>>(BinStream &bs, FileEntry &entry) {
    bs >> entry.mOffset >> entry.mHashedName >> entry.mHashedPath >> entry.mSize
        >> entry.mUCSize;
    return bs;
}

void Archive::Enumerate(
    const char *dir,
    void (*cb)(const char *, const char *),
    bool recurse,
    const char *pattern
) {
    char dtbPath[256];
    char folderPath[256];
    bool b3 = false;
    if (pattern && strstr(pattern, ".dta")) {
        b3 = true;
        sprintf(dtbPath, "%s/gen/%s.dtb", FileGetPath(pattern), FileGetBase(pattern));
        pattern = dtbPath;
        if (!recurse) {
            sprintf(folderPath, "%s/gen", dir);
            dir = folderPath;
        }
    }
    int dirLen = strlen(dir);
    const char *last = nullptr;
    bool b2 = false;
    FOREACH (it, mFileEntries) {
        const char *path = mHashTable[it->mHashedPath];
        if (last != path) {
            last = path;
            if (recurse) {
                b2 = strneq(path, dir, dirLen);
            } else {
                b2 = streq(path, dir);
            }
        }
        if (b2) {
            const char *name = mHashTable[it->mHashedName];
            if (pattern) {
                if (!FileMatch(MakeString("%s/%s", path, name), pattern))
                    continue;
            }
            if (b3) {
                path = FileGetPath(path);
                name = MakeString("%s.dta", FileGetBase(name));
            }
            cb(path, name);
        }
    }
}

int Archive::GetArkfileCachePriority(int arkfileNum) const {
    MILO_ASSERT(arkfileNum < mArkfileCachePriority.size(), 0x332);
    return mArkfileCachePriority[arkfileNum];
}

int Archive::GetArkfileNumBlocks(int arkfileNum) const {
    return (mArkfileSizes[arkfileNum] - 1) / kArkBlockSize + 1;
}

void Archive::SetLocationHardDrive() {
    for (int i = 0; i < mArkfileNames.size(); i++) {
        String path = TheContentMgr.TitleContentPath();
        mArkfileNames[i] = MakeString("%s/%s", path.c_str(), mArkfileNames[i].c_str());
    }
}

const char *Archive::GetArkfileName(int filenum) const {
    MILO_ASSERT(filenum < mArkfileNames.size(), 0x356);
    return mArkfileNames[filenum].c_str();
}

bool Archive::GetFileInfo(
    const char *file,
    int &arkfileNum,
    unsigned long long &byteOffset,
    int &fileSize,
    int &fileUCSize
) {
    if (!file || !*file) {
        return false;
    }

    String name(FileGetName(file));
    String path(FileGetPath(file));
    int nameValue = mHashTable.GetHashValue(name.c_str());
    int pathValue = mHashTable.GetHashValue(path.c_str());
    if (nameValue == -1 || pathValue == -1) {
        return false;
    }

    FileEntry entry;
    entry.mHashedName = nameValue;
    entry.mHashedPath = pathValue;
    auto it = std::lower_bound(mFileEntries.begin(), mFileEntries.end(), entry);
    if (it == mFileEntries.end() || it->mHashedName != nameValue
        || it->mHashedPath != pathValue) {
        arkfileNum = 0;
        byteOffset = 0;
        fileSize = 0;
        fileUCSize = 0;
        return false;
    }

    arkfileNum = 0;
    u64 u7 = 0;
    for (; arkfileNum < mNumArkfiles; arkfileNum++) {
        u64 u6 = mArkfileSizes[arkfileNum] + u7;
        if (it->mOffset < u6)
            break;
        u7 = u6;
    }
    MILO_ASSERT(arkfileNum < mNumArkfiles, 0x183);
    byteOffset = it->mOffset - u7;
    fileSize = it->mSize;
    fileUCSize = it->mUCSize;
    return true;
}

void Archive::Read(int heap_headroom) {
    MILO_LOG("Reading the archive\n");
    FileStream arkhdr(MakeString("%s.hdr", mBasename), FileStream::kReadNoArk, true);
    MILO_ASSERT(!arkhdr.Fail(), 0x265);
    arkhdr.EnableReadEncryption();
    int version;
    arkhdr >> version;
    if (version != 6) {
        MILO_FAIL(" ERROR: %s  unsupported archive version %d", mBasename, version);
    } else {
        arkhdr >> mGuid;
        arkhdr >> mNumArkfiles;
        arkhdr >> mArkfileSizes;
        if (version == 3) {
            for (int i = 0; i < mArkfileSizes.size(); i++) {
                mArkfileNames.push_back(MakeString("%s_%d.ark", mBasename, i));
            }
        } else
            arkhdr >> mArkfileNames;

        if (version > 5)
            arkhdr >> mArkfileCachePriority;
        else {
            for (int i = 0; i < mArkfileSizes.size(); i++) {
                mArkfileCachePriority.push_back(-1);
            }
        }

        mHashTable.Read(arkhdr, heap_headroom);
        arkhdr >> mFileEntries;
        arkhdr.DisableEncryption();
    }
}

void Archive::Merge(Archive &shadow) {
    std::vector<FileEntry> extraFileEntries;
    MILO_ASSERT(shadow.mNumArkfiles == 1, 0x3C0);
    u64 totalSize = 0;
    for (int i = 0; i < mArkfileSizes.size(); i++) {
        totalSize += mArkfileSizes[i];
    }
    auto &shadowEntries = shadow.mFileEntries;
    auto &hashTable = shadow.mHashTable;
    FOREACH (it, shadowEntries) {
        const char *name = hashTable[it->mHashedName];
        const char *path = hashTable[it->mHashedPath];
        FileEntry entry;
        entry.mHashedName = mHashTable.AddString(name);
        entry.mHashedPath = mHashTable.AddString(path);
        auto fileIt = std::lower_bound(mFileEntries.begin(), mFileEntries.end(), entry);
        if (fileIt != mFileEntries.end() && fileIt->mHashedName == entry.mHashedName
            && fileIt->mHashedPath == entry.mHashedPath) {
            fileIt->mOffset = it->mOffset + totalSize;
            fileIt->mSize = it->mSize;
            fileIt->mUCSize = it->mUCSize;
        } else {
            extraFileEntries.push_back(FileEntry(
                entry.mHashedName,
                entry.mHashedPath,
                it->mOffset + totalSize,
                it->mSize,
                it->mUCSize
            ));
        }
    }
    mNumArkfiles++;
    mArkfileSizes.push_back(shadow.mArkfileSizes[0]);
    mArkfileNames.push_back(shadow.mArkfileNames[0]);
    mArkfileCachePriority.push_back(shadow.mArkfileCachePriority[0]);
    mFileEntries.insert(
        mFileEntries.end(), extraFileEntries.begin(), extraFileEntries.end()
    );
    std::sort(mFileEntries.begin(), mFileEntries.end());
    mIsPatched = true;
}

void ArchiveInit() {
    if (UsingCD() || OptionBool("force_ark", false)) {
        Symbol plat = PlatformSymbol(TheLoadMgr.GetPlatform());
        bool hardDrive = false;
        bool b4 = false;
        const char *hdrName;
        if (UsingCD()) {
            String titlePath(TheContentMgr.TitleContentPath());
            if (!titlePath.empty()) {
                hardDrive = true;
                b4 = true;
                hdrName = MakeString("%s/gen/patch_%s", titlePath.c_str(), plat);
            }
        } else {
            hdrName = MakeString("gen/patch_%s", plat);
            b4 = true;
        }
        if (b4 && FileExists(MakeString("%s.hdr", hdrName), 0x10000, nullptr)) {
            Archive archive(hdrName, 0);
            if (hardDrive) {
                archive.SetLocationHardDrive();
            }
            TheArchive =
                new Archive(MakeString("gen/main_%s", plat), archive.GetHeapSize());
            TheArchive->Merge(archive);
        } else {
            TheArchive = new Archive(MakeString("gen/main_%s", plat), 0);
        }
        static int preinitArk = 1;
        TheArchive->SetArchivePermission(1, &preinitArk);
    }
    gDebugArkOrder = OptionBool("debug_arkorder", false);
    TheBlockMgr.Init();
}
