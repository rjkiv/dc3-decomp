#pragma once
#include "obj/Data.h"
#include <cstdlib>

class LocaleChunkSort {
public:
    struct OrderedLocaleChunk {
        DataNode sym; // 0x0
        DataNode pos; // 0x8
        DataNode str; // 0x10

        MEM_ARRAY_OVERLOAD(OrderedLocaleChunk, 0x1d)
    };

    // FIXME: this needs to not be static
    template <int N>
    static int FastSort(const void *node0, const void *node1);

    void Sort(OrderedLocaleChunk *data, int size) {
        qsort(data, size, sizeof(OrderedLocaleChunk), FastSort<3>);
    }
};
