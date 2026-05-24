#pragma once
#include "os/Debug.h"

namespace Hmx {
    class CRC {
    public:
        CRC() : mCRC(0) {}
        CRC(const char *cstr) : mCRC(ComputeHash(cstr, strlen(cstr))) {
            MILO_ASSERT(ValidateCRC(mCRC, cstr), 0x20);
        }
        // CRC(int);
        // CRC(const FixedString&);

        operator int() const { return mCRC; }
        // bool IsValid() const;
        // bool Null() const;
        // bool operator==(int) const;
        // bool operator!=(int) const;
        // int GetHash();
        // const char* Str() const;

        // a bunch of AddData funcs, see RBVR if you need to use them

        static bool ValidateCRC(int, const char *); // just returns true
        static int ComputeHash(const char *, unsigned int);
        // static const char* LookupCRC(int);
        // static const int kInvalidCRC;

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
