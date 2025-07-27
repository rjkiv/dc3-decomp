#pragma once

#include <types.h>

class BinStream {
public:
    void Read(void *, int);
    void ReadEndian(void *, int);

#define BASIC_READ(typename)                                                             \
    inline BinStream &operator>>(typename &rhs) {                                        \
        typename tmp;                                                                    \
        ReadEndian(&tmp, sizeof(typename));                                              \
        rhs = tmp;                                                                       \
        return *this;                                                                    \
    }
    BASIC_READ(int)
    BASIC_READ(uint)
    BASIC_READ(u16)
    BASIC_READ(u32)
    BASIC_READ(u64)

#undef BASIC_READ
};
