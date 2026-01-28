#include "trie.h"

void Trie::inc_dup_count(unsigned int n) {
    check_index(n);
    char *node = GetNode(n);
    unsigned int *countsPtr = (unsigned int *)(node + 0xC);
    unsigned int dupCount = *countsPtr >> 8;
    check_index(n);
    unsigned char count = *(node + 0xF);
    *countsPtr = (dupCount + 1) << 8 | count;
}

void Trie::dec_dup_count(unsigned int n) {
    check_index(n);
    unsigned int *countsPtr = GetCounts(n);
    unsigned int dupCount = GetDupCount(n);
    check_index(n);
    unsigned char count = GetCount(n);
    *countsPtr = (dupCount - 1) << 8 | count;
}

void Trie::inc_count(unsigned int n) {
    check_index(n);
    unsigned char count = GetCount(n);
    check_index(n);
    unsigned int *countsPtr = GetCounts(n);
    *countsPtr = (*countsPtr & 0xFFFFFF00) | (count + 1);
}

void Trie::dec_count(unsigned int n) {
    check_index(n);
    unsigned char count = GetCount(n);
    check_index(n);
    unsigned int *countsPtr = GetCounts(n);
    *countsPtr = (*countsPtr & 0xFFFFFF00) | (count - 1);
}

unsigned int Trie::get_free_node() {
    unsigned int n = GetFreeHead();

    if (n != 0) {
        check_index(n);
        SetFreeHead(GetSibling(n));
    } else {
        int _nodeCount = *GetNodeCount();
        MILO_ASSERT(_nodeCount < MAX_NODES, 0x82);
        n = _nodeCount;
        IncNodeCount();
    }
    return n;
}

void Trie::delete_node(unsigned int n) {
    check_index(n);
    int nodeOffset = n * NODE_SIZE;
    *GetNode(nodeOffset) = 0;

    check_index(n);
    ClearSibling(nodeOffset);
}