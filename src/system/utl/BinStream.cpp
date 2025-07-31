#include "BinStream.h"
#include "os/Debug.h"

#define BUF_SIZE 512

const char *BinStream::Name() const { return "<unnamed>"; }

BinStream::BinStream(bool b) : mLittleEndian(b), mCrypto(nullptr), mRevStack(nullptr) {}

void SwapData(const void *in, void *out, int size);

void BinStream::DisableEncryption(){
    MILO_ASSERT(mCrypto, 0xDC);
    RELEASE(mCrypto);
}

void BinStream::WriteEndian(const void *in, int size) {
    if (mLittleEndian) {
        u64 output;
        SwapData(in, &output, size);
        in = &output;
    }
    Write(in, size);
}

bool BinStream::AddSharedInlined(const class FilePath &) {
    MILO_FAIL("BinStream::AddSharedInlined is a PC only dev tool !!");
    return false;
}

BinStream &BinStream::operator<<(const char *str) {
    MILO_ASSERT(str, 0x60);
    int len = 0;
    const char *cc = str;
    while (*++cc)
        ;
    len = cc - str;
    len--;
    *this << len;
    Write(str, len);
    return *this;
}

BinStream &BinStream::operator<<(const Symbol &sym) {
    int len;
    MILO_ASSERT(len < BUF_SIZE, 0x6C);
    return *this;
}

BinStream &BinStream::operator<<(const class String &str) { return *this; }

void BinStream::ReadEndian(void *out, int size) {
    Read(out, size);
    if (mLittleEndian) {
        SwapData(out, out, size);
    }
}

// void BinStream::ReadString(char *, int) {}

BinStream &BinStream::operator>>(Symbol &sym) {
    char buf[BUF_SIZE];
    ReadString(buf, BUF_SIZE);
    sym = buf;
    return *this;
}
BinStream &BinStream::operator>>(String &str) {
    int siz;
    *this >> siz;
    str.resize(siz);
    Read((void*)str.Str(), siz);
    return *this;
}

void BinStream::EnableReadEncryption(void) {}

BinStream::~BinStream() { delete mCrypto; }
