#include "os/HDCache.h"
#include "math/SHA1.h"
#include "math/Utl.h"
#include "os/Archive.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/HxGuid.h"
#include "utl/MemStream.h"
#include "utl/Option.h"

HDCache TheHDCache;

HDCache::HDCache()
    : mBlockState(0), mWriteFileIdx(0), mWriteBlock(-1), mWritingHeader(false),
      mReadFileIdx(0), mDirtyCache(0), mLastHdrWriteMs(-1), mLastCacheWriteMs(-1),
      mLockId(-1), mLockCount(0), mLockCrit(nullptr), mHdrIdx(0), mHdrBuf(nullptr),
      mUsingPriorData(false) {}

HDCache::~HDCache() {}

bool HDCache::LockCache() {
    CritSecTracker cst(mLockCrit);
    if (mLockId == -1 || mLockId == GetCurrentThreadId()) {
        mLockId = GetCurrentThreadId();
        mLockCount++;
        return true;
    } else {
        return false;
    }
}

void HDCache::UnlockCache() {
    CritSecTracker cst(mLockCrit);
    MILO_ASSERT(mLockId == CurrentThreadId(), 0xfa);
    mLockCount--;
    if (mLockCount == 0)
        mLockId = -1;
}

int HDCache::HdrSize() {
    int size = 32;
    int numArkfiles = TheArchive->NumArkFiles();
    for (int i = 0; i < numArkfiles; i++) {
        if (TheArchive->GetArkfileCachePriority(i) >= 0) {
            int numBlocks = TheArchive->GetArkfileNumBlocks(i);
            size += ((numBlocks + 31) / 32 + 1) * 4;
        }
    }
    size += 0x100;
    if (size % 0x1000 != 0) {
        size = size - size % 0x1000 + 0x1000;
    }
    return size;
}

bool HDCache::ReadFail() {
    if (ReadFile() && ReadFile()->Fail()) {
        MILO_LOG("HDCache Read %d failed\n", mReadFileIdx);
        return true;
    } else
        return false;
}

bool HDCache::ReadDone() {
    if (!ReadFile()) {
        return true;
    }
    return ReadFile()->ReadDone();
}

bool HDCache::WriteDone() {
    if (mWriteBlock >= 0) {
        if (WriteFile()->WriteDone()) {
            MILO_ASSERT(
                mReadArkFiles[mWriteFileIdx]->Size() == mWriteArkFiles[mWriteFileIdx]->Size(),
                499
            );
            UnlockCache();
            if (WriteFile()->Fail()) {
                MILO_LOG("HDCache Write %d.%d failed\n", mWriteFileIdx, mWriteBlock);
            } else {
                int word = mWriteBlock / 32;
                int bit = 1 << (mWriteBlock % 32);
                if (++mDirtyCache == 1) {
                    mLastHdrWriteMs = SystemMs();
                }
                mBlockState[mWriteFileIdx][word] |= bit;
            }
            mWriteBlock = -1;
        }
    }
    return mWriteBlock == -1;
}

// void HDCache::Flush() {}

void HDCache::Poll() {
    if (mWritingHeader) {
        if (mHdr[mHdrIdx]->WriteDone()) {
            UnlockCache();
            if (mHdr[mHdrIdx]->Fail()) {
                MILO_LOG("HDCache Write Header Failed\n");
            }
            Flush();
            mWritingHeader = false;
        }
    }
    if (mDirtyCache && !mWritingHeader
        && (mDirtyCache > 0x400 || SystemMs() - mLastHdrWriteMs > 60000)) {
        WriteHdr();
    }
}

bool HDCache::ReadAsync(int arkfileNum, int blockNum, void *dst) {
    MILO_ASSERT(ReadDone(), 0x191);
    if (mBlockState[arkfileNum]) {
        MILO_ASSERT(blockNum < TheArchive->GetArkfileNumBlocks(arkfileNum), 0x196);
        if (mBlockState[arkfileNum][blockNum / 32] & 1 << (blockNum % 32)) {
            MILO_ASSERT(
                mReadArkFiles[arkfileNum]->Size() >= ((blockNum + 1) * kArkBlockSize), 0x19D
            );
            mReadFileIdx = arkfileNum;
            mReadArkFiles[arkfileNum]->Seek(blockNum * kArkBlockSize, 0);
            return mReadArkFiles[mReadFileIdx]->ReadAsync(dst, kArkBlockSize);
        }
    }
    return false;
}

bool HDCache::WriteAsync(int arkfileNum, int blockNum, const void *src) {
    MILO_ASSERT(WriteDone(), 0x1C1);
    if (mBlockState[arkfileNum]) {
        MILO_ASSERT(blockNum < TheArchive->GetArkfileNumBlocks(arkfileNum), 0x1C6);
        if (!(mBlockState[arkfileNum][blockNum / 32] & 1 << (blockNum % 32))
            && mWriteArkFiles[arkfileNum]->Size() >= (blockNum + 1) * kArkBlockSize
            && LockCache()) {
            mLastCacheWriteMs = SystemMs();
            mWriteFileIdx = arkfileNum;
            mWriteBlock = blockNum;
            mWriteArkFiles[arkfileNum]->Seek(blockNum * kArkBlockSize, 0);
            bool ret = WriteFile()->WriteAsync(src, kArkBlockSize);
            if (!ret) {
                WriteDone();
            }
            return ret;
        }
    }
    return false;
}

FileStream *HDCache::OpenHeader() {
    if (mHdrFmt.empty()) {
        return nullptr;
    }
    const char *str;
    int i;
    for (i = 0; i < 2; i++) {
        str = MakeString(mHdrFmt.c_str(), 0);
        if (FileExists(str, 0x10000, nullptr)) {
            break;
        }
    }
    if (i == 2) {
        return nullptr;
    }
    return new FileStream(str, FileStream::kReadNoArk, true);
}

void HDCache::WriteHdr() {
    if (!mHdr[mHdrIdx]->Fail() && LockCache()) {
        MILO_ASSERT(mHdr[mHdrIdx]->WriteDone(), 0x144);
        CSHA1 dataSignature;
        mHdrBuf->Seek(0, BinStream::kSeekBegin);
        mHdrBuf->EnableWriteEncryption();
        *mHdrBuf << 2;
        HxGuid guid;
        TheArchive->GetGuid(guid);
        *mHdrBuf << guid;
        int numArkfiles = TheArchive->NumArkFiles();
        *mHdrBuf << numArkfiles;
        for (int i = 0; i < numArkfiles; i++) {
            int blockSize = 0;
            if (mBlockState[i]) {
                int numWords = (TheArchive->GetArkfileNumBlocks(i) + 31) / 32;
                blockSize = numWords * sizeof(int);
            }
            *mHdrBuf << blockSize;
            if (blockSize > 0) {
                mHdrBuf->Write(mBlockState[i], blockSize);
                dataSignature.Update((const unsigned char *)mBlockState[i], blockSize);
            }
        }
        char dataSignatureString[256] = { 0 };
        dataSignature.Final().ReportHash(dataSignatureString, 0);
        mHdrBuf->Write(dataSignatureString, 256);
        mHdrBuf->DisableEncryption();
        mDirtyCache = 0;
        int finalSize = HdrSize();
        MILO_ASSERT(mHdrBuf->Size() <= finalSize, 0x176);
        char zeros[0x80];
        memset(zeros, 0, 0x80);
        while (mHdrBuf->Size() < finalSize) {
            int size = finalSize - mHdrBuf->Size();
            if (size > 0x80U)
                size = 0x80;
            mHdrBuf->Write(zeros, size);
        }
        MILO_ASSERT(mHdrBuf->Size() == finalSize, 0x183);
        int oldSize = mHdr[mHdrIdx]->Size();
        int newSize = mHdrBuf->Size();
        MILO_ASSERT(oldSize == newSize, 0x186);
        mWritingHeader = true;
        mHdr[mHdrIdx]->Seek(0, 0);
        mHdr[mHdrIdx]->WriteAsync(mHdrBuf->Buffer(), mHdrBuf->Size());
    }
}

void HDCache::OpenFiles(int numCachedArkfiles) {
    if (mFileFmt.empty())
        return;
    int numArkfiles = TheArchive->NumArkFiles();
    MILO_ASSERT(numCachedArkfiles <= numArkfiles, 0x22F);
    FileMkDir(FileGetPath(mFileFmt.c_str()));
    std::vector<int> pendingArkfiles;
    for (int i = 0; i < numArkfiles; i++) {
        const char *fileName = MakeString(mFileFmt.c_str(), i);
        bool exists = FileExists(fileName, 0x10000, nullptr);
        int prio = TheArchive->GetArkfileCachePriority(i);
        if (exists && i > numCachedArkfiles) {
            FileDelete(fileName);
        }
        if (prio >= 0) {
            pendingArkfiles.push_back(i);
        }
    }
    const char *hdrName = MakeString(mHdrFmt.c_str(), 0);
    mHdr[0] = NewFile(hdrName, 0x50101);
    bool hdrValid = mHdr[0] && !mHdr[0]->Fail();
    if (hdrValid) {
        int hdrSize = HdrSize();
        mHdr[0]->Truncate(hdrSize);
        RELEASE(mHdr[0]);
        mHdr[0] = NewFile(hdrName, 0x50001);
        hdrValid = (hdrSize - mHdr[0]->Size()) == 0;
    }
    if (!hdrValid) {
        RELEASE(mHdr[0]);
    } else {
        while (pendingArkfiles.size() != 0) {
            std::vector<int>::iterator max = pendingArkfiles.end();
            int maxPriority = -1;
            for (std::vector<int>::iterator it = pendingArkfiles.begin();
                 it != pendingArkfiles.end();
                 ++it) {
                int priority = TheArchive->GetArkfileCachePriority(*it);
                if (priority > maxPriority) {
                    maxPriority = priority;
                    max = it;
                }
            }
            MILO_ASSERT(max != pendingArkfiles.end(), 0x26F);
            int arkfileNum = *max;
            const char *fileName = MakeString(mFileFmt.c_str(), arkfileNum);
            File *file = NewFile(fileName, 0x50101);
            bool truncated = file
                && file->Truncate(
                    TheArchive->GetArkfileNumBlocks(arkfileNum) * kArkBlockSize
                );
            if (file) {
                delete file;
                if (!truncated) {
                    FileDelete(fileName);
                }
            }
            pendingArkfiles.erase(max);
        }
        for (int i = 0; i < numArkfiles; i++) {
            const char *fileName = MakeString(mFileFmt.c_str(), i);
            File *readFile = NewFile(fileName, 0x50002);
            File *writeFile = NewFile(fileName, 0x50001);
            if (!readFile || !writeFile || readFile->Fail() || writeFile->Fail()) {
                RELEASE(readFile);
                RELEASE(writeFile);
            }
            mReadArkFiles[i] = readFile;
            mWriteArkFiles[i] = writeFile;
        }
    }
}

// sets up the hard drive cache for the ark files
// however it seems that in this build HDCache is effectively always disabled
void HDCache::Init() {
    mLockCrit = new CriticalSection();
    if (TheArchive) {
		
        // this option actually does nothing as Flush does nothing in this build
        if (OptionBool("no_hdcache", true)) {
            Flush();
        }
		
        int numArkfiles = TheArchive->NumArkFiles();
        mReadArkFiles.resize(numArkfiles);
        mWriteArkFiles.resize(numArkfiles);
		
        // this will always be null due to how mHdrFmt/mFileFmt are never set to anything?
        FileStream *header = OpenHeader();
		
        // the header is only considered good if it's the expected size, it's v2,
        // and it was written for the current archive, so if the ark guid changes the hdcache is invalidated
        bool valid = header && header->Size() == HdrSize();
        if (valid) {
            header->EnableReadEncryption();
            int version;
            *header >> version;
            valid = version == 2;
        }
        if (valid) {
            HxGuid guidA, guidB;
            *header >> guidA;
            TheArchive->GetGuid(guidB);
            valid = guidA == guidB;
        }
        int numCachedArkfiles = 0;
        if (valid) {
            *header >> numCachedArkfiles;
            // can't have cached more arkfiles than there are
            if (numCachedArkfiles < 0 || numCachedArkfiles > numArkfiles) {
                numCachedArkfiles = 0;
                valid = false;
            }
        }
        // create  or open the cache file for each arkfile on the HDD
        OpenFiles(numCachedArkfiles);
		
        // one bitmap per ark and 1 bit per block, which becomes set if the block is cached
        mBlockState = new int *[numArkfiles];
		
        // hash of all the saved bitmaps which is checked against the one at the end of the header
        CSHA1 dataSignature;
        int cacheBuf[1024];
        for (int i = 0; i < numArkfiles; i++) {
            int cacheBytes = 0;
            if (i < numCachedArkfiles) {
                *header >> cacheBytes;
                if (cacheBytes > sizeof(cacheBuf) || cacheBytes % sizeof(int)) {
                    valid = false;
                } else {
                    header->Read(cacheBuf, cacheBytes);
                }
				
                // any read failure invalidates it all
                if (header->Fail() || !valid) {
                    cacheBytes = 0;
                    valid = false;
                    numCachedArkfiles = 0;
                }
                if (valid) {
                    dataSignature.Update((const unsigned char *)cacheBuf, cacheBytes);
                }
            }
			
            File *&readFile = mReadArkFiles[i];
            File *&writeFile = mWriteArkFiles[i];
            if (!readFile || readFile->Fail() || !writeFile || writeFile->Fail()) {
                RELEASE(readFile);
                RELEASE(writeFile);
            }
            if (readFile) {
                int numBlocks = TheArchive->GetArkfileNumBlocks(i);
                int numWords = (numBlocks + 31) / 32;
                int *state = new int[numWords];
                memcpy(state, cacheBuf, cacheBytes);
                memset(&state[cacheBytes], 0, numWords * sizeof(int) - cacheBytes);
                mBlockState[i] = state;
            } else {
                mBlockState[i] = nullptr;
            }
        }
        // make sure the hash of the bitmaps matches the one that was saved
        if (valid) {
            char dataSignatureStringA[256] = { 0 };
            char dataSignatureStringB[256] = { 0 };
            dataSignature.Final().ReportHash(dataSignatureStringA, 0);
            header->Read(dataSignatureStringB, 256);
            valid = !header->Fail()
                && memcmp(dataSignatureStringA, dataSignatureStringB, 256) == 0;
        }
		
        // does what it says on the tin
        if (OptionBool("skip_hdcache", false)) {
            valid = false;
        }
        if (valid) {
            mUsingPriorData = true;
            MILO_LOG("Using the archive cache\n");
        } else {
            for (int i = 0; i < numArkfiles; i++) {
                if (mBlockState[i]) {
                    int numWords = (TheArchive->GetArkfileNumBlocks(i) + 31) / 32;
                    memset(mBlockState[i], 0, numWords * sizeof(int));
                }
            }
        }
        delete header;
        mHdrFmt = "";
        mFileFmt = "";
        mHdrBuf = new MemStream(true);
    }
}
