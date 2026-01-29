#include "trie.h"

void Trie::inc_dup_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    unsigned int dupCount = TRIE_GET_DUP_COUNT(countsPtr);
    check_index(n);
    unsigned char count = TRIE_GET_COUNT(node);
    *countsPtr = TRIE_INC_DUP_COUNT(dupCount, count);
}

void Trie::dec_dup_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    unsigned int dupCount = TRIE_GET_DUP_COUNT(countsPtr);
    check_index(n);
    unsigned char count = TRIE_GET_COUNT(node);
    *countsPtr = TRIE_DEC_DUP_COUNT(dupCount, count);
}

void Trie::inc_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned char count = TRIE_GET_COUNT(node);
    check_index(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    *countsPtr = TRIE_INC_COUNT(countsPtr, count);
}

void Trie::dec_count(unsigned int n) {
    check_index(n);
    char *node = TRIE_GET_NODE(n);
    unsigned char count = TRIE_GET_COUNT(node);
    check_index(n);
    unsigned int *countsPtr = TRIE_GET_COUNTS(node);
    *countsPtr = TRIE_DEC_COUNT(countsPtr, count);
}

unsigned int Trie::get_free_node() {
    unsigned int n = *TRIE_GET_FREE_HEAD;

    if (n != 0) {
        check_index(n);
        *TRIE_GET_FREE_HEAD = TRIE_GET_SIBLING(n);
    } else {
        int _nodeCount = *TRIE_GET_NODE_COUNT;
        MILO_ASSERT(_nodeCount < MAX_NODES, 0x82);
        n = _nodeCount;
        TRIE_INC_NODE_COUNT;
    }
    return n;
}

void Trie::delete_node(unsigned int n) {
    check_index(n);
    TRIE_CLEAR_NODE(n);
    check_index(n);
    TRIE_CLEAR_SIBLING(n);
    check_index(n);
    TRIE_CLEAR_PARENT(n);
    *TRIE_GET_COUNTS(n) = *(unsigned int *)0;
    check_index(n);
    unsigned int *freeHead = TRIE_GET_FREE_HEAD;
    TRIE_SET_CHAR(n, 0xFF);
    if (*freeHead != 0) {
        check_index(n);
        *TRIE_SET_SIBLING(n, *freeHead);
    }
    *freeHead = n;
}