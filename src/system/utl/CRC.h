#pragma once

namespace Hmx {
    class CRC {
    public:
        CRC() : mCRC(0) {}
        CRC(const char *);
        bool operator<(const CRC &c) const { return mCRC < c.mCRC; }
        void Reset() { mCRC = 0; }

        int mCRC; // 0x0
    };
}

#include "utl/BinStream.h"
inline BinStream &operator<<(BinStream &bs, const Hmx::CRC &crc) {
    bs << crc.mCRC;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::CRC &crc) {
    bs >> crc.mCRC;
    return bs;
}
