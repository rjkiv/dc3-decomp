#include "utl/Chunks.h"
#include "os/Debug.h"

void ChunkHeader::Read(BinStream &bs) {
    mID.Load(bs);
    bs >> mLength;
    if (mID == kListChunkID || mID == kRiffChunkID) {
        mID.Load(bs);
        mIsList = true;
        mLength -= 4;
        MILO_ASSERT(mLength == 0 || mLength >= kDataHeaderSize, 0x26);
    } else
        mIsList = false;
}

IListChunk::IListChunk(BinStream &bs, bool b)
    : mParent(0), mBaseBinStream(bs), mHeader(0), mStartMarker(-1), mEndMarker(-1),
      mLocked(0), mSubHeader(), mRecentlyReset(1) {
    if (b) {
        mHeader = new ChunkHeader(mBaseBinStream);
    } else {
        int tell1 = bs.Tell();
        bs.Seek(0, BinStream::kSeekEnd);
        int tell2 = bs.Tell();
        bs.Seek(tell1, BinStream::kSeekBegin);
        mHeader = new ChunkHeader(kListChunkID, tell2 - tell1, true);
    }
    mStartMarker = mBaseBinStream.Tell();
    Init();
}

IListChunk::IListChunk(IListChunk &chunk)
    : mParent(&chunk), mBaseBinStream(chunk.mBaseBinStream), mHeader(0), mStartMarker(-1),
      mEndMarker(-1), mLocked(0), mSubHeader(), mSubChunkValid(0), mRecentlyReset(1),
      mSubChunkMarker(-1) {
    mHeader = new ChunkHeader(*mParent->CurSubChunkHeader());
    mStartMarker = mBaseBinStream.Tell();
    Init();
}

IListChunk::~IListChunk() {
    if (mParent)
        mParent->UnLock();
    delete mHeader;
}

void IListChunk::Init() {
    MILO_ASSERT(mHeader->IsList(), 0x105);
    mEndMarker = mStartMarker + mHeader->Length();
    if (mParent)
        mParent->Lock();
    Reset();
}

void IListChunk::Reset() {
    MILO_ASSERT(!mLocked, 0x116);
    mBaseBinStream.Seek(mStartMarker, BinStream::kSeekBegin);
    mSubChunkMarker = mStartMarker;
    mSubChunkValid = false;
    mRecentlyReset = true;
}

const ChunkHeader *IListChunk::CurSubChunkHeader() const {
    MILO_ASSERT(mRecentlyReset == false, 0x126);
    if (!mSubChunkValid)
        return 0;
    else
        return &mSubHeader;
}

const ChunkHeader *IListChunk::Next() {
    MILO_ASSERT(!mLocked, 0x138);
    mRecentlyReset = false;
    if (mSubChunkMarker >= mEndMarker) {
        mSubChunkValid = false;
        return 0;
    } else {
        mSubChunkValid = true;
        mBaseBinStream.Seek(mSubChunkMarker, BinStream::kSeekBegin);
        mSubHeader.Read(mBaseBinStream);

        int newlen = mSubHeader.TotalLength();
        ChunkID theID = mSubHeader.ID();
        if (theID != kMidiTrackChunkID) {
            newlen += newlen % 2;
        }

        mSubChunkMarker += newlen;
        return &mSubHeader;
    }
}

const ChunkHeader *IListChunk::Next(ChunkID id) {
    MILO_ASSERT(!mLocked, 0x15A);
    while (Next()) {
        ChunkID theID = mSubHeader.ID();
        if (id == theID)
            return &mSubHeader;
    }
    return 0;
}

void IListChunk::Lock() {
    MILO_ASSERT(mLocked == false, 0x16A);
    mLocked = true;
}

void IListChunk::UnLock() {
    MILO_ASSERT(mLocked == true, 0x174);
    mLocked = false;
}

IDataChunk::IDataChunk(IListChunk &chunk)
    : BinStream(true), mParent(&chunk), mBaseBinStream(chunk.BaseStream()), mHeader(0),
      mFailed(0), mEof(0) {
    MILO_ASSERT(mParent->CurSubChunkHeader(), 0x47);
    mHeader = new ChunkHeader(*mParent->CurSubChunkHeader());
    MILO_ASSERT(!mHeader->IsList(), 0x49);
    mStartMarker = mBaseBinStream.Tell();
    mEndMarker = mStartMarker + mHeader->Length();
    mParent->Lock();
}

IDataChunk::~IDataChunk() {
    if (mParent)
        mParent->UnLock();
    delete mHeader;
}

int IDataChunk::Tell() {
    if (Fail())
        return -1;
    else
        return mBaseBinStream.Tell() - mStartMarker;
}

void IDataChunk::ReadImpl(void *data, int bytes) {
    int tell = mBaseBinStream.Tell();
    if (bytes < mEndMarker - tell) {
        mBaseBinStream.Read(data, bytes);
    } else {
        mBaseBinStream.Read(data, mEndMarker - tell);
        mEof = true;
    }
}

void IDataChunk::SeekImpl(int iOffset, SeekType t) {
    if (!Fail()) {
        switch (t) {
        case BinStream::kSeekBegin:
            MILO_ASSERT(iOffset >= 0, 0x79);
            if (iOffset > mHeader->Length())
                mFailed = true;
            mBaseBinStream.Seek(iOffset + mStartMarker, kSeekBegin);
            break;
        case BinStream::kSeekCur:
            mBaseBinStream.Seek(iOffset, kSeekCur);
            break;
        case BinStream::kSeekEnd:
            MILO_ASSERT(iOffset <= 0, 0x8A);
            if (iOffset < -mHeader->Length())
                mFailed = true;
            mBaseBinStream.Seek(iOffset + mEndMarker, kSeekBegin);
            break;
        default:
            break;
        }
        mEof = mBaseBinStream.Eof() != 0;
    }
}
