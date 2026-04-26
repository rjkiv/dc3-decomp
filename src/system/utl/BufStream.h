#pragma once
#include "utl/BinStream.h"
#include "math/StreamChecksum.h"

class BufStream : public BinStream {
public:
    BufStream(void *buffer, int size, bool littleEndian);
    virtual ~BufStream();
    virtual void Flush() {}
    virtual int Tell() { return mTell; }
    virtual EofType Eof() {
        if (mTell == mSize) {
            return RealEof;
        } else {
            return NotEof;
        }
    }
    virtual bool Fail() { return mFail; }
    virtual const char *Name() const;
    virtual int Size();

    void DeleteChecksum();
    void StartChecksum(const char *);
    bool ValidateChecksum();
    void SetName(const char *);

private:
    char *mBuffer; // 0x10
    bool mFail; // 0x14
    int mTell; // 0x18
    int mSize; // 0x1c
    StreamChecksumValidator *mChecksum; // 0x20
    int mBytesChecksummed; // 0x24
    String mName; // 0x28

    virtual void ReadImpl(void *, int);
    virtual void WriteImpl(const void *, int);
    virtual void SeekImpl(int, SeekType);
};
