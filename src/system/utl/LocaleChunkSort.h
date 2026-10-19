#pragma once
#include "obj/Data.h"
#include <cstdlib>

namespace LocaleChunkSort {
    struct OrderedLocaleChunk {
        DataNode sym; // 0x0
        DataNode pos; // 0x8
        DataNode str; // 0x10

        MEM_OVERLOAD(OrderedLocaleChunk, 0x1d)
    };

    template <int N>
    int FastSort(const void *node0, const void *node1) {
        const DataNode *anode = (const DataNode *)node0;
        const DataNode *bnode = (const DataNode *)node1;
        for (int i = 0; i < N; i++, bnode++) {
            if (anode[i].IntValue() < bnode[i].IntValue()) {
                return -1;
            }
            if (anode[i].IntValue() > bnode[i].IntValue()) {
                return 1;
            }
        }
        return 0;
    }
};
