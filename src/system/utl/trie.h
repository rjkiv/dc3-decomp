#pragma once
#include "MemTrack.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

#define MAX_NODES 0x20000
#define NODE_SIZE 0x11
#define TRIE_GET_NODE(idx) ((char *)this + idx * NODE_SIZE)
#define TRIE_GET_COUNTS(node) (unsigned int *)(node + 0xC)
#define TRIE_GET_DUP_COUNT(countsPtr) *countsPtr >> 8
#define TRIE_INC_DUP_COUNT(dupCount, count) (dupCount + 1) << 8 | count
#define TRIE_DEC_DUP_COUNT(dupCount, count) (dupCount - 1) << 8 | count
#define TRIE_GET_COUNT(node) *(node + 0xF)
#define TRIE_INC_COUNT(countsPtr, count) (*countsPtr & 0xFFFFFF00) | (count + 1)
#define TRIE_DEC_COUNT(countsPtr, count) (*countsPtr & 0xFFFFFF00) | (count - 1)
#define TRIE_GET_NODE_COUNT (int *)(unsigned int *)((char *)this + 0x220000)
#define TRIE_GET_FREE_HEAD (unsigned int *)((char *)this + 0x220004)
#define TRIE_GET_SIBLING(node) *(unsigned int *)(TRIE_GET_NODE(node) + 0x4)
#define TRIE_SET_SIBLING(node, freehead)                                                 \
    *(unsigned int *)(TRIE_GET_NODE(node) + 0x4) = freehead
#define TRIE_INC_NODE_COUNT *TRIE_GET_NODE_COUNT = *TRIE_GET_NODE_COUNT + 1
#define TRIE_CLEAR_NODE(node) *(unsigned int *)(char *)(this + node * NODE_SIZE) = 0
#define TRIE_CLEAR_SIBLING(node)                                                         \
    *(unsigned int *)((char *)this + node * NODE_SIZE + 0x4) = 0
#define TRIE_GET_PARENT(node) *(unsigned int *)(TRIE_GET_NODE(node) + 0x8)
#define TRIE_CLEAR_PARENT(node)                                                          \
    *(unsigned int *)((char *)this + node * NODE_SIZE + 0x8) = 0
#define TRIE_GET_CHAR(node) *((char *)this + node * NODE_SIZE + 0x10)
#define TRIE_SET_CHAR(node, chr) *((char *)this + node * NODE_SIZE + 0x10) = chr
#define TRIE_GET_CHILD(node) *(unsigned int *)(TRIE_GET_NODE(node))
#define TRIE_SET_CHILD(node, set) *(unsigned int *)(TRIE_GET_NODE(node)) = set
#define TRIE_SET_PARENT(node, set) *(unsigned int *)(TRIE_GET_NODE(node) + 0x8) = set
#define TRIE_SET_COUNTS(node, set) *(unsigned int *)(TRIE_GET_NODE(node) + 0xC) = set
#define TRIE_CLEAR_COUNTS(node) *(unsigned int *)(TRIE_GET_NODE(node) + 0xC) = 0

// oh yeah this class is awful
#pragma pack(push, 1)
class Trie {
public:
    Trie() {
        memset(this, 0, sizeof(Trie));
        _nodeCount = 1;
    }

    int store(const char *);
    void remove(unsigned int);

    char *get(int n, char *buf, int bufSize);
    void check_index(unsigned int n) { MILO_ASSERT(0<= n && n < MAX_NODES, 0x36); }
    void inc_count(unsigned int);
    void dec_count(unsigned int);
    void inc_dup_count(unsigned int);
    void dec_dup_count(unsigned int);
    unsigned int get_free_node();
    void delete_node(unsigned int);

    void set_unk0(unsigned int idx, char *value) {
        check_index(idx);
        mNodes[idx].unk0 = value;
    }

    unsigned int get_sibling(unsigned int idx) {
        check_index(idx);
        return mNodes[idx].sibling;
    }
    void set_sibling(unsigned int idx, unsigned int sibling) {
        check_index(idx);
        mNodes[idx].sibling = sibling;
    }

    void clear_parent(unsigned int idx) {
        check_index(idx);
        mNodes[idx].unk8 = 0;
        mNodes[idx].mCounts = 0;
    }

    void set_char(unsigned int idx, char c) {
        check_index(idx);
        mNodes[idx].mChar = c;
    }

    MEM_OVERLOAD(Trie, 0x28);

private:
    // size 0x11
    struct Node {
        char *unk0;
        unsigned int sibling; // 0x4
        unsigned int unk8; // 0x8 - parent?
        union {
            struct {
                unsigned int mDupCount : 24; // 0xc
                unsigned int mCount : 8; // 0xf
            };
            unsigned int mCounts; // 0xc
        };
        char mChar; // 0x10
    };

    Node mNodes[MAX_NODES]; // 0x0
    int _nodeCount; // 0x220000
    unsigned int mFreeHead; // 0x220004
    // Counts is a 4 byte int thats used to store Duplicate count and total count
    // DupCount = upper 24 bits of the int(16mil max) & Count = low 8 bits(255 max)
};
#pragma pack(pop)
