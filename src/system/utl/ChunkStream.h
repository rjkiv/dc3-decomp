#pragma once
#include "os/File.h"
#include "os/Platform.h"
#include "os/Timer.h"
#include "utl/BinStream.h"

#define kChunkSizeMask 0x00ffffff
#define kChunkUnusedMask 0xfe000000
#define CHUNKSTREAM_Z_ID 0xCBBEDEAF
#define CHUNKSTREAM_Z_ID2 0xCCBEDEAF
#define CHUNKSTREAM_Z_ID3 0xCDBEDEAF
#define kChunkIDMask 0xC0BEDEAF


enum BufferState {
    kInvalid,
    kReading,
    kDecompressing,
    kReady,
};

struct DecompressTask {
    int *mChunk; // 0x0
    void *mBuffer; // 0x4
    BufferState *mState; // 0x8
    int unkc; // 0xc i think this is the expected decompressed size
    int mID; // 0x10
    char *mTempBuf; // 0x14
};

class ChunkStream : public BinStream {
public:
    class ChunkInfo {
    public:
        ChunkInfo(bool compressed)
            : mID(compressed ? 0xCDBEDEAF : 0xCABEDEAF), mChunkInfoSize(0x810),
              mNumChunks(0), mMaxChunkSize(0) {
            mChunks[0] = 0;
        }
        int mID; // offset 0x0, size 0x4
        int mChunkInfoSize; // offset 0x4, size 0x4
        int mNumChunks; // offset 0x8, size 0x4
        int mMaxChunkSize; // offset 0xC, size 0x4
        int mChunks[512]; // offset 0x10, size 0x800
    };

    enum FileType {
        kRead = 0,
        kWrite = 1,
    };

    ChunkStream(const char *, FileType, int, bool, Platform, bool);
    virtual ~ChunkStream();
    virtual void Flush() {}
    virtual int Tell();
    virtual EofType Eof();
    virtual bool Fail();
    virtual const char *Name() const;
    virtual bool Cached() const;
    virtual Platform GetPlatform() const;

    void PotentiallyWriteChunk(bool b) { MaybeWriteChunk(b); } // so dumb
    static bool PollDecompressionWorker();

private:
    virtual void ReadImpl(void *, int);
    virtual void WriteImpl(const void *, int);
    virtual void SeekImpl(int, SeekType);

    static void DecompressChunk(DecompressTask &);
    void DecompressChunkAsync();
    int WriteChunk();

    void SetPlatform(Platform);
    void ReadChunkAsync();
    void MaybeWriteChunk(bool);

    File *mFile; // 0x10
    String mFilename; // 0x14
    bool mFail; // 0x1c
    FileType mType; // 0x20
    ChunkInfo mChunkInfo; // 0x24
    bool mIsCached; // 0x834
    Platform mPlatform; // 0x838
    int mBufSize; // 0x83c
    char *mBuffers[3]; // 0x840, 0x844, 0x848
    char *mCurReadBuffer; // 0x84c
    Timer mStartTime; // 0x850
    int mRecommendedChunkSize; // 0x880
    int mLastWriteMarker; // 0x884
    int mCurBufferIdx; // 0x888
    BufferState mBuffersState[3]; // 0x88c, 0x890, 0x894
    int *mBuffersOffset[3]; // 0x898, 0x89c, 0x8a0
    int mCurBufOffset; // 0x8a4
    bool mChunkInfoPending; // 0x8a8
    int *mCurChunk; // 0x8ac
    int *mChunkEnd; // 0x8b0
    int mTell; // 0x8b4
};

BinStream &MarkChunk(BinStream &);
void SetActiveChunkObject(Hmx::Object *obj);
BinStream &WriteChunks(BinStream &, const void *, int, int);
void DecompressMemHelper(const void *, int, void *, int &, const char *);