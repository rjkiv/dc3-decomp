#include "utl/BinStream.h"
#include "math/Rand.h"
#include "math/Rand2.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/Timer.h"
#include <vector>

#define BUF_SIZE 512

const char *BinStream::Name() const { return "<unnamed>"; }

BinStream::BinStream(bool b) : mLittleEndian(b), mCrypto(nullptr), mRevStack(nullptr) {}

void SwapData(const void *in, void *out, int size) {
    switch (size) {
    case 2: {
        unsigned short *s1 = (unsigned short *)in;
        unsigned short *s2 = (unsigned short *)out;
        *s2 = EndianSwap(*s1);
        break;
    }
    case 4: {
        unsigned int *i1 = (unsigned int *)in;
        unsigned int *i2 = (unsigned int *)out;
        *i2 = EndianSwap(*i1);
        break;
    }
    case 8: {
        unsigned long long *l1 = (unsigned long long *)in;
        unsigned long long *l2 = (unsigned long long *)out;
        *l2 = EndianSwap(*l1);
        break;
    }
    default:
        MILO_ASSERT(0, 0xAC);
        break;
    }
}

void BinStream::DisableEncryption() {
    MILO_ASSERT(mCrypto, 0xDC);
    RELEASE(mCrypto);
}

void BinStream::Write(const void *void_data, int bytes) {
    if (Fail()) {
        MILO_PRINT_ONCE("Stream error: Can't write to %s\n", Name());
    } else {
        const unsigned char *data = (u8 *)void_data;
        if (!mCrypto) {
            WriteImpl(void_data, bytes);
        } else {
            u8 crypt[512];
            while (bytes > 0) {
                int x = Min(512, bytes);
                for (int i = 0; i < x; i++) {
                    u8 bastard = mCrypto->Int();
                    crypt[i] = data[i] ^ bastard;
                }
                WriteImpl(crypt, x);
                bytes -= 512;
                data += 512;
            }
        }
    }
}

void BinStream::Seek(int offset, SeekType type) {
    MILO_ASSERT(!Fail(), 0x11F);
    MILO_ASSERT(!mCrypto, 0x122);
    SeekImpl(offset, type);
}

void BinStream::WriteEndian(const void *in, int size) {
    if (mLittleEndian) {
        u64 output[2]; // 128 bits of buffer to swap
        SwapData(in, output, size);
        Write(output, size);
    } else
        Write(in, size);
}

bool BinStream::AddSharedInlined(const class FilePath &) {
    MILO_FAIL("BinStream::AddSharedInlined is a PC dev tool only !!");
    return false;
}

BinStream &BinStream::operator<<(const char *str) {
    MILO_ASSERT(str, 0x60);
    int size = strlen(str);
    *this << size;
    Write(str, size);
    return *this;
}

BinStream &BinStream::operator<<(const Symbol &sym) {
    const char *str = sym.Str();
    unsigned int len = strlen(str);
    MILO_ASSERT(len < BUF_SIZE, 0x6C);
    *this << len;
    Write(str, len);
    return *this;
}

BinStream &BinStream::operator<<(const class String &str) {
    int size = str.length();
    *this << size;
    Write(str.c_str(), size);
    return *this;
}

void BinStream::EnableWriteEncryption() {
    MILO_ASSERT(!mCrypto, 0xC8);
    int i = RandomInt();
    *this << i;
    mCrypto = new Rand2(i);
}

int BinStream::PopRev(Hmx::Object *o) {
    MILO_ASSERT(mRevStack, 0x34);
    ObjVersion *back = &mRevStack->back();
    while (back->obj == nullptr) {
        MILO_NOTIFY("hey object got deleted!");
        mRevStack->pop_back();
        back = &mRevStack->back();
    }
    int revs = back->revs;
    if (o != back->obj) {
        MILO_LOG("rev stack $this mismatch (%08x != %08x\n", o, back->obj);
        MILO_LOG("curr obj: %s %s\n", o->ClassName(), PathName(o));
        MILO_LOG("stack obj: %s %s\n", back->obj->ClassName(), PathName(back->obj));
        MILO_FAIL(
            "rev stack (%08x %s %s != %08x %s %s)\n",
            o,
            o->ClassName(),
            PathName(o),
            back->obj,
            back->obj->ClassName(),
            PathName(back->obj)
        );
    }
    mRevStack->pop_back();
    return revs;
}

void BinStream::Read(void *data, int bytes) {
    if (Fail()) {
        MILO_NOTIFY_ONCE("Stream error: Can't read from %s", Name());
        memset(data, 0, bytes);
    } else {
        AutoGlitchReport report(50.0f, "BinStream::Read");
        unsigned char *ptr = (unsigned char *)data;
        unsigned char *end;
        ReadImpl(data, bytes);
        if (mCrypto) {
            end = ptr + bytes;
            while (ptr < end) {
                *ptr++ ^= mCrypto->Int();
            }
        }
    }
}

int BinStream::ReadAsync(void *v, int i) {
    Read(v, i);
    return Fail() & i; // fix this
}

void BinStream::ReadEndian(void *out, int size) {
    Read(out, size);
    if (mLittleEndian) {
        SwapData(out, out, size);
    }
}

void BinStream::ReadString(char *c, int i) {
    unsigned int a;
    *this >> a;
    if (a >= i)
        MILO_FAIL("String chars %d > %d", a + 1, i);
    Read(c, a);
    c[a] = 0;
}

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
    Read((void *)str.c_str(), siz);
    return *this;
}

void BinStream::EnableReadEncryption() {
    MILO_ASSERT(!mCrypto, 0xC0);
    int i;
    *this >> i;
    mCrypto = new Rand2(i);
}

BinStream::~BinStream() {
    delete mCrypto;
    delete mRevStack;
}

void BinStream::PushRev(int revs, Hmx::Object *obj) {
    if (!mRevStack) {
        mRevStack = new std::vector<ObjVersion>();
    }
    mRevStack->push_back(ObjVersion(revs, obj));
}
