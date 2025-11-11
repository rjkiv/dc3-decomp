#pragma once
#include "File.h"
#include "os/File.h"
#include "os/Debug.h"

class BufFile : public File {
public:
    BufFile(const void *buf, int size)
        : mBuf((const unsigned char *)buf), mSize(size), mPos(mBuf), mBytesRead(0) {
        MILO_ASSERT(buf, 0x20);
        MILO_ASSERT(size > 0, 0x21);
    }
    virtual class String Filename() const { return MakeString("--memory (BufFile)--"); }
    virtual int Read(void *data, int bytes) {
        int size = mSize + mBuf - mPos;
        if (size >= bytes) {
            size = bytes;
        }
        memcpy(data, mPos, size);
        mBytesRead = size;
        mPos += size;
        return size;
    }
    virtual bool ReadAsync(void *data, int bytes) {
        Read(data, bytes);
        return true;
    }
    virtual int Write(const void *, int) {
        MILO_FAIL("not implemented");
        return 0;
    }
    virtual int Seek(int iPos, int iType) {
        MILO_ASSERT(iType == FILE_SEEK_SET || iType == FILE_SEEK_CUR, 0x3E);
        if (iType == FILE_SEEK_SET) {
            mPos = mBuf + iPos;
        } else if (iType == FILE_SEEK_CUR) {
            mPos += iPos;
        }
        MILO_ASSERT(mPos >= mBuf && mPos < mPos + mSize, 0x45);
        return mPos - mBuf;
    }
    virtual int Tell() { return mPos - mBuf; }
    virtual void Flush() {}
    virtual bool Eof();
    virtual bool Fail() { return false; }
    virtual int Size() { return mSize; }
    virtual int UncompressedSize() { return 0; }
    virtual bool ReadDone(int &bytes) {
        bytes = mBytesRead;
        return true;
    }
    virtual bool GetFileHandle(void *&) { return false; }
    virtual bool Truncate(int) { return false; }

private:
    const unsigned char *mBuf; // 0x4
    int mSize; // 0x8
    const unsigned char *mPos; // 0xc
    int mBytesRead; // 0x10
};
