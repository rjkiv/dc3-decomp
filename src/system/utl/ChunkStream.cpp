#include "utl/ChunkStream.h"
#include "Compress.h"
#include "obj/Object.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/SynchronizationEvent.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"

namespace {
    HANDLE mThreadHandle[1];
    bool gDecompressionThread = false;
    static Hmx::Object *gActiveChunkObject;
    std::list<DecompressTask> gDecompressionQueue;
    CriticalSection gDecompressionCritSec;
    SynchronizationEvent gDataProcessedEvt;
    SynchronizationEvent gDataReadyEvt;

    DWORD DecompressionThread(void *) {
        while (gDecompressionThread) {
            if (ChunkStream::PollDecompressionWorker()) {
                gDataProcessedEvt.Set();
            } else {
                gDataReadyEvt.Wait(-1);
            }
        }
        return false;
    }

    void StartDecompressionThread() {
        if (!gDecompressionThread) {
            gDecompressionThread = true;
            for (int i = 0; i < DIM(mThreadHandle); i++) {
                mThreadHandle[i] =
                    CreateThread(nullptr, 0, DecompressionThread, nullptr, 4, nullptr);
                MILO_ASSERT(mThreadHandle[i], 0x82);
                XSetThreadProcessor(mThreadHandle[i], 3);
                ResumeThread(mThreadHandle[i]);
            }
        } else {
            gDataReadyEvt.Set();
        }
    }
}

void ChunkStream::SetPlatform(Platform plat) {
    if (plat == kPlatformNone) {
        plat = ConsolePlatform();
    }
    mLittleEndian = PlatformLittleEndian(plat);
    mPlatform = plat;
}

void ChunkStream::WriteImpl(const void *data, int bytes) {
    if (mCurBufOffset + bytes > mBufSize) {
        while (mCurBufOffset + bytes > mBufSize)
            mBufSize += mBufSize;
        void *a = _MemAllocTemp(mBufSize, __FILE__, 0x1E4, "ChunkStreamBuf", 0);
        memcpy(a, mBuffers[0], mCurBufOffset);
        MemFree(mBuffers[0]);
        mBuffers[0] = (char *)a;
        MemFree(mBuffers[1]);
        mBuffers[1] =
            (char *)_MemAllocTemp(mBufSize, __FILE__, 0x1EA, "ChunkStreamBuf", 0);
    }
    memcpy(mBuffers[0] + mCurBufOffset, data, bytes);
    mCurBufOffset += bytes;
}

void ChunkStream::ReadChunkAsync() {
    int bufIdx = 1;
    int idx;
    for (; bufIdx < 4; bufIdx++) {
        idx = (mCurBufferIdx + bufIdx) % (int)DIM(mBuffers);
        if (mBuffersState[idx] == kInvalid)
            break;
    }
    if (mBuffersState[idx] == kInvalid) {
        int *thechunk = &mCurChunk[bufIdx];
        if (thechunk != mChunkEnd) {
            int thechunkval = *thechunk;
            int sizemask = thechunkval & kChunkSizeMask;
            if (mChunkInfo.mID != 0xCABEDEAF && !((thechunkval >> 24) & 1)) {
                mFile->ReadAsync(mBuffers[idx] + mBufSize - sizemask, sizemask);
            } else
                mFile->ReadAsync(mBuffers[idx], sizemask);
            mBuffersOffset[idx] = &mCurChunk[bufIdx];
            mBuffersState[idx] = kReading;
        }
    }
}

void SetActiveChunkObject(Hmx::Object *obj) { gActiveChunkObject = obj; }

BinStream &ReadChunks(BinStream &bs, void *data, int total_len, int max_chunk_size) {
    int curr_size = 0;
    while (curr_size != total_len) {
        int len_left = Min(total_len - curr_size, max_chunk_size);
        char *dataAsChars = (char *)data;
        bs.Read(&dataAsChars[curr_size], len_left);
        curr_size += len_left;
        while (bs.Eof() == TempEof)
            Timer::Sleep(0);
    }
    return bs;
}

ChunkStream::ChunkStream(
    const char *file,
    FileType type,
    int chunkSize,
    bool compress,
    Platform plat,
    bool cached
)
    : BinStream(false), mFile(nullptr), mFilename(file), mFail(false), mType(type),
      mChunkInfo(compress), mIsCached(cached), mBufSize(-1), mCurReadBuffer(nullptr),
      mRecommendedChunkSize(chunkSize), mLastWriteMarker(0), mCurBufferIdx(-1),
      mCurBufOffset(0), mChunkInfoPending(false), mCurChunk(nullptr), mChunkEnd(nullptr),
      mTell(0) {
    SetPlatform(plat);
    for (int bufCnt = 0; bufCnt < 3; bufCnt++) {
        mBuffersState[bufCnt] = kInvalid;
        mBuffersOffset[bufCnt] = 0;
        mBuffers[bufCnt] = 0;
    }
    mFile = NewFile(file, type == kRead ? 2 : 0x301);
    mFail = !mFile;
    if (!mFail) {
        if (type == kWrite) {
            mFile->Write(&mChunkInfo, 0x810);
            mBufSize = mRecommendedChunkSize * 2;
            mBuffers[0] =
                (char *)_MemAllocTemp(mBufSize, __FILE__, 0x144, "ChunkStreamBuf", 0);
            mBuffers[1] =
                (char *)_MemAllocTemp(mBufSize, __FILE__, 0x145, "ChunkStreamBuf", 0);
            mCurBufferIdx = 0;
        } else {
            mChunkInfoPending = true;
            mFile->ReadAsync(&mChunkInfo, 0x810);
        }
    }
}

ChunkStream::~ChunkStream() {
    if (!mFail && mType == kWrite) {
        MaybeWriteChunk(true);
        if (mChunkInfo.mNumChunks == 512) {
            MILO_NOTIFY(
                "%s is %d compressed bytes too large", mFilename, mChunkInfo.mChunks[511]
            );
        }
        memset(
            &mChunkInfo.mChunks[mChunkInfo.mNumChunks],
            0,
            sizeof(mChunkInfo.mChunks) - mChunkInfo.mNumChunks * sizeof(int)
        );
        for (int i = 0; i < mChunkInfo.mNumChunks; i++) {
            EndianSwapEq(mChunkInfo.mChunks[i]);
        }
        EndianSwapEq(mChunkInfo.mID);
        EndianSwapEq(mChunkInfo.mChunkInfoSize);
        EndianSwapEq(mChunkInfo.mNumChunks);
        EndianSwapEq(mChunkInfo.mMaxChunkSize);
        mFile->Seek(0, 0);
        mFile->Write(&mChunkInfo, sizeof(ChunkInfo));
    }
    delete mFile;
    while (true) {
        bool b3;
        for (int i = 2; i >= 0; i--) {
            if (mBuffersState[i] == kDecompressing) {
                b3 = true;
                goto check;
            }
        }
        b3 = false;
    check:
        if (!b3) {
            for (int i = 0; i < 3; i++) {
                MILO_ASSERT(mBuffersState[i] != kDecompressing, 0x194);
                MemFree(mBuffers[i]);
            }
            return;
        }
        gDataProcessedEvt.Wait(-1);
    }
}

bool ChunkStream::Cached() const { return mIsCached; }
Platform ChunkStream::GetPlatform() const { return mPlatform; }

void ChunkStream::ReadImpl(void *data, int bytes) {
    MILO_ASSERT(mCurBufferIdx != -1, 0x1D3);
    MILO_ASSERT(mBuffersState[mCurBufferIdx] == kReady, 0x1D4);
    MILO_ASSERT(mBuffersOffset[mCurBufferIdx] == mCurChunk, 0x1D5);
    MILO_ASSERT(mCurBufOffset + bytes <= (*mCurChunk & kChunkSizeMask), 0x1D6);
    memcpy(data, (void *)(mCurReadBuffer + mCurBufOffset), bytes);
    mCurBufOffset += bytes;
    mTell += bytes;
}

void ChunkStream::SeekImpl(int, SeekType) { MILO_FAIL("Can't seek on chunkstream"); }

int ChunkStream::Tell() {
    if (mType == kRead) {
        return mTell;
    } else {
        MILO_FAIL("Can't tell on chunkstream");
        return 0;
    }
}

int ChunkStream::WriteChunk() {
    MILO_ASSERT(mCurBufOffset < kChunkSizeMask, 0x310);
    int size = mCurBufOffset;
    int flags = 0;
    int *curBuf = (int *)mBuffers[0];
    if (mChunkInfo.mID == 0xCDBEDEAF) {
        int l38 = mBufSize - 4;
        int *secondBuf = (int *)mBuffers[1];
        *secondBuf = size;
        EndianSwapEq(*secondBuf);
        CompressMem(mBuffers[0], size, secondBuf + 1, l38, nullptr);
        if (((float)mCurBufOffset / (float)l38) > 1.1f && mChunkInfo.mNumChunks != 0) {
            curBuf = secondBuf;
            size = l38 + 4;
        } else
            flags |= 0x1000000;
    }
    if (mFile->Write(curBuf, size) != size) {
        mFail = true;
    }
    MILO_ASSERT((size & ~kChunkSizeMask) == 0, 0x33A);
    MILO_ASSERT((flags & (kChunkSizeMask|kChunkUnusedMask)) == 0, 0x33C);
    int result = size | flags;
    MILO_ASSERT((result & kChunkUnusedMask) == 0, 0x341);
    return result;
}

BinStream &MarkChunk(BinStream &bs) {
    ChunkStream *cs = dynamic_cast<ChunkStream *>(&bs);
    if (cs)
        cs->PotentiallyWriteChunk();
    return bs;
}

void DecompressMemHelper(
    const void *compressedMem, int size, void *dst, int &dstLen, const char *filename
) {
    unsigned int rawSize = *(unsigned int *)compressedMem;
    DecompressMem((const char *)compressedMem + 4, size - 4, dst, dstLen, filename);
    int expectedDstLen = EndianSwap(rawSize);
    MILO_ASSERT(dstLen == expectedDstLen, 0x3bb);
}

void ChunkStream::DecompressChunk(DecompressTask &task) {
    MILO_ASSERT(*task.mState == kDecompressing, 0x3c1);
    int data = *task.mChunkSize;
    int compressedSize = data & kChunkSizeMask;
    MILO_ASSERT((data & ~kChunkSizeMask) == 0, 0x3c5);
    int uncompressedSize = task.mUncompressedSize;
    if (task.mID == CHUNKSTREAM_Z_ID3) {
        DecompressMemHelper(
            (char *)task.mUncompressedData + (uncompressedSize - compressedSize),
            compressedSize,
            task.mUncompressedData,
            uncompressedSize,
            task.mFilename
        );
    } else if (task.mID == CHUNKSTREAM_Z_ID2) {
        DecompressMem(
            (char *)task.mUncompressedData + (uncompressedSize - compressedSize) + 10,
            compressedSize - 18,
            task.mUncompressedData,
            uncompressedSize,
            task.mFilename
        );
    } else {
        MILO_ASSERT(task.mID == CHUNKSTREAM_Z_ID, 0x3d7);
        DecompressMem(
            (char *)task.mUncompressedData + (task.mUncompressedSize - compressedSize),
            compressedSize,
            task.mUncompressedData,
            uncompressedSize,
            task.mFilename
        );
    }
    *task.mChunkSize = uncompressedSize;
    *task.mState = kReady;
}

void ChunkStream::DecompressChunkAsync() {
    int idx = 1;
    BufferState *cur;
    for (; idx < 4; idx++) {
        cur = &mBuffersState[(mCurBufferIdx + idx) % 3];
        if (*cur == kReading) {
            break;
        }
    }
    if (*cur == kReading) {
        int val = mCurChunk[idx];
        if (mChunkInfo.mID == 0xCABEDEAF || (val >> 24) & 1) {
            *cur = kReady;
        } else {
            *cur = kDecompressing;
            {
                DecompressTask task(
                    &mCurChunk[idx],
                    mBuffers[idx],
                    cur,
                    mBufSize,
                    mChunkInfo.mID,
                    mFilename.c_str()
                );
                CritSecTracker t(&gDecompressionCritSec);
                gDecompressionQueue.push_back(task);
            }
            StartDecompressionThread();
        }
    }
}

bool ChunkStream::PollDecompressionWorker() {
    CritSecTracker t(&gDecompressionCritSec);
    if (gDecompressionQueue.size() != 0) {
        DecompressTask task = gDecompressionQueue.front();
        gDecompressionQueue.pop_front();
        gDecompressionCritSec.Exit(); // wat do here
        DecompressChunk(task);
        return true;
    } else {
        return false;
    }
}

BinStream &WriteChunks(BinStream &bs, const void *v, int i1, int i2) {
    for (int i = 0; i != i1;) {
        int temp = i1 - i;
        if (i2 < temp) {
            temp = i2;
        }
        bs.Write((void *)(i + (int)v), temp);
        i += temp;
        if (bs.GetPlatform() == kPlatformWii) {
            MarkChunk(bs);
        }
    }
    return bs;
}

void ChunkStream::MaybeWriteChunk(bool b) {
    if (mChunkInfo.mNumChunks < 2 && 0x2000 <= mCurBufOffset) {
        b = true;
    }
    if (mCurBufOffset >= mRecommendedChunkSize || b != false) {
        bool nearlyFull = mChunkInfo.mNumChunks == 511;
        if (!b && nearlyFull) {
            return;
        }
        if (mCurBufOffset >= mRecommendedChunkSize + 0x2000 && 0x2000 <= mLastWriteMarker
            && !nearlyFull) {
            int size = mCurBufOffset - mLastWriteMarker;
            void *dst = _MemAllocTemp(size, __FILE__, 0x2e6, "ChunkStreamBuf", 0);
            memcpy(dst, mBuffers[0] + mLastWriteMarker, size);
            int writeMarker = mLastWriteMarker;
            mLastWriteMarker = 0;
            mCurBufOffset = writeMarker;
            MaybeWriteChunk(true);
            mCurBufOffset = size;
            memcpy(mBuffers[0], dst, size);
            MemFree(dst);
            if (b == false) {
                return;
            }
        }
        MILO_ASSERT_FMT(
            mChunkInfo.mNumChunks < 512,
            "%s has %d chunks, max is %d",
            mFilename,
            mChunkInfo.mNumChunks,
            512
        );
        int chunkWrite = WriteChunk();
        mChunkInfo.mChunks[mChunkInfo.mNumChunks] = chunkWrite;
        mChunkInfo.mNumChunks++;
        mChunkInfo.mMaxChunkSize =
            Max(chunkWrite & kChunkSizeMask, mCurBufOffset, mChunkInfo.mMaxChunkSize);
        mCurBufOffset = 0;
    }
    mLastWriteMarker = mCurBufOffset;
}

EofType ChunkStream::Eof() {
    MILO_ASSERT(!mFail && mType == kRead, 0x22C);
    if (mChunkInfoPending) {
        int lol;
        if (!mFile->ReadDone(lol)) {
            return TempEof;
        } else {
            mChunkInfoPending = false;
            EndianSwapEq(mChunkInfo.mID);
            EndianSwapEq(mChunkInfo.mChunkInfoSize);
            EndianSwapEq(mChunkInfo.mNumChunks);
            EndianSwapEq(mChunkInfo.mMaxChunkSize);
            for (int i = 0; i < mChunkInfo.mNumChunks; i++) {
                EndianSwapEq(mChunkInfo.mChunks[i]);
            }
            if ((mChunkInfo.mID & 0xf0ffffff) != kChunkIDMask) {
                mChunkInfo.mChunkInfoSize = 0;
                mChunkInfo.mNumChunks = 1;
                mChunkInfo.mID = 0xCABEDEAF;
                mChunkInfo.mMaxChunkSize = mFile->Size();
                MILO_ASSERT((mChunkInfo.mMaxChunkSize & ~kChunkSizeMask) == 0, 0x24B);
                mChunkInfo.mChunks[0] = mChunkInfo.mMaxChunkSize;
            }
            if (strstr(mFilename.c_str(), ".milo_")) {
                mIsCached = true;
                if (strstr(mFilename.c_str(), ".milo_xbox")) {
                    SetPlatform(kPlatformXBox);
                } else if (strstr(mFilename.c_str(), ".milo_ps3")) {
                    SetPlatform(kPlatformPS3);
                } else if (strstr(mFilename.c_str(), ".milo_wii")) {
                    SetPlatform(kPlatformWii);
                } else if (strstr(mFilename.c_str(), ".milo_3ds")) {
                    SetPlatform(kPlatform3DS);
                } else {
                    SetPlatform(kPlatformPC);
                }
            } else {
                mIsCached = false;
                SetPlatform(kPlatformPC);
            }
            mBufSize = mChunkInfo.mMaxChunkSize;
            if (mChunkInfo.mID != 0xCABEDEAF) {
                mBufSize = mChunkInfo.mMaxChunkSize + 0x800;
            }
            int num = Min(3, mChunkInfo.mNumChunks);
            for (int i = 0; i < num; i++) {
                mBuffers[i] =
                    (char *)_MemAllocTemp(mBufSize, __FILE__, 0x26F, "ChunkStreamBuf", 0);
            }
            int *chunks = mChunkInfo.mChunks;
            mCurChunk = chunks - 1;
            mChunkEnd = chunks + mChunkInfo.mNumChunks;
            mCurBufOffset = *mCurChunk & kChunkSizeMask;
            mCurBufferIdx = 2;
            mFile->Seek(mChunkInfo.mChunkInfoSize, 0);
            ReadChunkAsync();
        }
    }

    int lmao;
    if (mFile->ReadDone(lmao)) {
        DecompressChunkAsync();
        ReadChunkAsync();
    }
    if ((*mCurChunk & kChunkSizeMask) <= mCurBufOffset) {
        MILO_ASSERT(mCurBufOffset == (*mCurChunk & kChunkSizeMask), 0x291);
        if (mBuffersOffset[mCurBufferIdx] == mCurChunk) {
            mBuffersState[mCurBufferIdx] = kInvalid;
        }
        if (mCurChunk + 1 == mChunkEnd) {
            return RealEof;
        } else {
            int idx = (mCurBufferIdx + 1) % 3;
            if (mBuffersState[idx] == kReady) {
                mCurBufferIdx = idx;
                mCurChunk++;
                mCurBufOffset = 0;
                mCurReadBuffer = mBuffers[idx];
                return NotEof;
            } else {
                return TempEof;
            }
        }
    }
    return NotEof;
}
