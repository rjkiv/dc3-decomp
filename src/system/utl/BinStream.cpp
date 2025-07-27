#include "BinStream.h"
#include "os/Debug.h"

const char *BinStream::Name() const { return "<unnamed>"; }

BinStream::BinStream(bool b) : mLittleEndian(b), mCrypto(nullptr) {}

void SwapData(const void *in, void *out, int size);

void BinStream::WriteEndian(const void *in, int size) {
    if (mLittleEndian) {
        u64 output;
        SwapData(in, &output, size);
        in = &output;
    }
    Write(in, size);
}

bool BinStream::AddSharedInlined(const class FilePath &) {
    "BinStream::AddSharedInlined is a PC only dev tool !!";
    return false;
}

BinStream &BinStream::operator<<(const char *str) {
    MILO_ASSERT(str);
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
BinStream &BinStream::operator<<(const class Symbol &) { return *this; }
BinStream &BinStream::operator<<(const class String &) { return *this; }

void BinStream::ReadEndian(void *out, int size) {
    Read(out, size);
    if (mLittleEndian) {
        SwapData(out, out, size);
    }
}

void BinStream::ReadString(char *, int) {}

void BinStream::EnableReadEncryption(void) {}

BinStream::~BinStream() { delete mCrypto; }
