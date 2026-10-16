#pragma once
#include "os/Debug.h"

namespace Hmx {
    class CRC {
    public:
        CRC() : mCRC(0) {}
        CRC(const char *cstr) : mCRC(ComputeHash(cstr, strlen(cstr))) {
            MILO_ASSERT(ValidateCRC(mCRC, cstr), 0x20);
        }
        CRC(int crc) : mCRC(crc) {}

        operator int() const { return mCRC; }
        // bool Null() const;
        // bool operator==(int) const;
        // bool operator!=(int) const;
        // const char* Str() const;

        static bool ValidateCRC(int crc, const char *str); // just returns true
        static int ComputeHash(const char *data, unsigned int size);
        // static const char* LookupCRC(int);

    private:
        int mCRC; // 0x0
    };
}

#include "utl/BinStream.h"
inline BinStream &operator<<(BinStream &bs, const Hmx::CRC &crc) {
    bs << (int &)crc;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::CRC &crc) {
    int hash = 0;
    bs >> hash;
    (int &)crc = hash;
    return bs;
}
