#pragma once
#include "utl/BinStream.h"
#include <vector>

class MemStream : public BinStream {
public:
    MemStream(bool = false);
    virtual ~MemStream() {}
    virtual void Flush();
    virtual int Tell() { return mTell; }
    virtual EofType Eof() {
        if (mTell == mBuffer.size()) {
            return RealEof;
        } else {
            return NotEof;
        }
    }
    virtual bool Fail();

    void WriteStream(BinStream &, int);
    void Compact();
    int Size() const { return mBuffer.size(); }
    void Resize(int size) { mBuffer.resize(size); }
    void Reserve(int size) { mBuffer.reserve(size); }
    const char *Buffer() const { return mBuffer.begin(); }

private:
    virtual void ReadImpl(void *, int);
    virtual void WriteImpl(const void *, int);
    virtual void SeekImpl(int, SeekType);

    bool mFail; // 0x10
    int mTell; // 0x14
    std::vector<char> mBuffer; // 0x18
};
