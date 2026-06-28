#pragma once
#include "os/Debug.h"

template <class T>
void EndianSwapBlock(T *block, int count) {
    MILO_ASSERT(block != NULL, 0x53);
    MILO_ASSERT(count >= 0, 0x54);
    for (T *it = block; it != block + count; it++) {
        *it = EndianSwap(*it);
    }
}

// template <>
inline void EndianSwapEq(unsigned int &i) {
    i = i >> 0x18 | i << 0x18 | i >> 8 & 0xFF00 | (i & 0xFF00) << 8;
}

// template <>
// inline void EndianSwapEq(unsigned short &s) {
//     s = (s << 8 | s >> 8);
// }

// template <>
// inline void EndianSwapEq(short &s) {
//     s = (s << 8 | s >> 8);
// }

inline unsigned short EndianSwap(unsigned short s) {
    unsigned short us = s;
    return us << 8 | us >> 8;
}

inline unsigned int EndianSwap(unsigned int i) {
    unsigned int ui = i;
    return ui >> 0x18 | ui << 0x18 | ui >> 8 & 0xFF00 | (ui & 0xFF00) << 8;
}

inline unsigned short SwapBytes(unsigned short bytes) { return EndianSwap(bytes); }

// the asm for this is inlined, it's in BinStream::ReadEndian and WriteEndian
// could also find the standalone function asm in RB3 retail

// example input:   0x12345678DEADBEEF
// should yield:    0xEFBEADDE78563412

// oh my god ghidra pseudocode managed to be spot on
// what are the odds
inline unsigned long long EndianSwap(unsigned long long ull) {
    return (((ull & 0xff000000000000 | ull >> 0x10) >> 0x10 | ull & 0xff0000000000)
                >> 0x10
            | ull & 0xff00000000)
        >> 8
        | (((ull << 0x10 | ull & 0xff00) << 0x10 | ull & 0xff0000) << 0x10
           | ull & 0xff000000)
        << 8;

    // unsigned int hi = (ull >> 32) & 0xFFFFFFFF;
    // unsigned int lo = ull & 0xFFFFFFFF;
    // unsigned int hi_swapped = EndianSwap(hi);
    // unsigned long long lo_swapped = EndianSwap(lo);
    // return (lo_swapped << 32) | hi_swapped;

    // unsigned int hi = (ull >> 56) | (ull >> 48 | 0xFF00) | (ull >> 40 | 0xFF0000)
    //     | (ull >> 32 | 0xFF000000);
    // unsigned long long lo = (ull >> 24 | 0xFF00000000) | (ull >> 16 | 0xFF0000000000)
    //     | (ull >> 8 | 0xFF000000000000) | (ull | 0xFF00000000000000);
    // return hi | lo;
}
